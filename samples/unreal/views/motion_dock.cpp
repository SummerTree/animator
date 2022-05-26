#include "motion_dock.h"
#include "draggable_list_widget.h"

#include <qpainter.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qevent.h>
#include <qapplication.h>
#include <qdrag.h>
#include <qmimedata.h>
#include <qprogressdialog.h>

namespace unreal
{
	MotionDock::MotionDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile)
		: behaviour_(behaviour)
		, profile_(profile)
		, clickedItem_(nullptr)
	{
		this->setWindowTitle(tr("Motion Library"));
		this->setObjectName("MotionDock");

		importButton_ = new QToolButton;
		importButton_->setObjectName("Import");
		importButton_->setText(tr("Import"));

		topLayout_ = new QHBoxLayout();
		topLayout_->addWidget(importButton_, 0, Qt::AlignLeft);
		topLayout_->addStretch();
		topLayout_->setContentsMargins(5, 0, 0, 0);

		bottomLayout_ = new QHBoxLayout();
		bottomLayout_->addStretch();
		bottomLayout_->setSpacing(2);
		bottomLayout_->setContentsMargins(0, 5, 15, 0);

		listWidget_ = new DraggableListWindow;
		listWidget_->setStyleSheet("background:transparent;");
		listWidget_->setSpacing(0);

		mainLayout_ = new QVBoxLayout();
		mainLayout_->addLayout(topLayout_);
		mainLayout_->addWidget(listWidget_);
		mainLayout_->addStretch();
		mainLayout_->addLayout(bottomLayout_);
		mainLayout_->setContentsMargins(0, 10, 0, 10);

		mainWidget_ = new QWidget;
		mainWidget_->setObjectName("MotionWidget");
		mainWidget_->setLayout(mainLayout_);

		this->setWidget(mainWidget_);

		connect(importButton_, SIGNAL(clicked()), this, SLOT(importClickEvent()));
		connect(listWidget_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClicked(QListWidgetItem*)));
		connect(listWidget_, SIGNAL(itemSelected(QListWidgetItem*)), this, SLOT(itemSelected(QListWidgetItem*)));
	}

	MotionDock::~MotionDock() noexcept
	{
	}

	void 
	MotionDock::addItem(std::string_view uuid) noexcept
	{
		auto motionComponent = behaviour_->getComponent<UnrealBehaviour>()->getComponent<MotionComponent>();
		if (!motionComponent)
			return;

		auto package = motionComponent->getPackage(uuid);
		if (!package.is_null())
		{
			QLabel* imageLabel = new QLabel;
			imageLabel->setObjectName("preview");
			imageLabel->setFixedSize(QSize(100, 100));

			QLabel* nameLabel = new QLabel();
			nameLabel->setObjectName("name");
			nameLabel->setFixedHeight(30);

			QVBoxLayout* widgetLayout = new QVBoxLayout;
			widgetLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
			widgetLayout->addWidget(nameLabel, 0, Qt::AlignCenter);
			widgetLayout->setSpacing(0);
			widgetLayout->setContentsMargins(0, 0, 0, 0);

			QWidget* widget = new QWidget;
			widget->setLayout(widgetLayout);

			QListWidgetItem* item = new QListWidgetItem;
			item->setData(Qt::UserRole, QString::fromStdString(package["uuid"].get<nlohmann::json::string_t>()));
			item->setSizeHint(QSize(imageLabel->width(), imageLabel->height() + nameLabel->height()) + QSize(10, 10));

			listWidget_->addItem(item);
			listWidget_->setItemWidget(item, widget);

			if (package.find("preview") != package.end())
			{
				auto filepath = package["preview"].get<nlohmann::json::string_t>();
				imageLabel->setPixmap(QPixmap(QString::fromStdString(filepath)).scaled(imageLabel->size()));
			}
			else
			{
				imageLabel->setPixmap(QPixmap(":res/icons/dance2.png").scaled(imageLabel->size()));
			}

			if (package.find("name") != package.end())
			{
				QFontMetrics metrics(nameLabel->font());

				auto name = QString::fromStdString(package["name"].get<nlohmann::json::string_t>());
				imageLabel->setToolTip(name);
				nameLabel->setText(metrics.elidedText(name, Qt::ElideRight, imageLabel->width()));
				nameLabel->setToolTip(name);
			}
		}
	}

	void
	MotionDock::importClickEvent()
	{
		QStringList filepaths = QFileDialog::getOpenFileNames(this, tr("Import Resource"), "", tr("VMD Files (*.vmd)"));
		if (!filepaths.isEmpty())
		{
			auto motionComponent = behaviour_->getComponent<UnrealBehaviour>()->getComponent<MotionComponent>();
			if (!motionComponent)
				return;

			try
			{
				if (filepaths.size() > 1)
				{
					QProgressDialog dialog(tr("Loading..."), tr("Cancel"), 0, filepaths.size(), this);
					dialog.setWindowTitle(tr("Loading..."));
					dialog.setWindowModality(Qt::WindowModal);
					dialog.show();

					for (qsizetype i = 0; i < filepaths.size(); i++)
					{
						dialog.setValue(i);
						dialog.setLabelText(QFileInfo(filepaths[i]).fileName());

						QCoreApplication::processEvents();
						if (dialog.wasCanceled())
							break;

						auto package = motionComponent->importMotion(filepaths[i].toStdString());
						if (!package.is_null())
							this->addItem(package["uuid"].get<nlohmann::json::string_t>());
					}
				}
				else
				{
					auto package = motionComponent->importMotion(filepaths[0].toStdString());
					if (!package.is_null())
						this->addItem(package["uuid"].get<nlohmann::json::string_t>());
				}

				motionComponent->save();
			}
			catch (...)
			{
				motionComponent->save();
			}
		}
	}

	void
	MotionDock::itemClicked(QListWidgetItem* item)
	{
		clickedItem_ = item;
	}

	void
	MotionDock::itemSelected(QListWidgetItem* item)
	{
		if (item)
		{
			auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
			if (!behaviour)
				return;

			auto selectedItem = behaviour->getProfile()->selectorModule->selectedItemHover_;
			if (!selectedItem.has_value())
				return;
			
			QProgressDialog dialog(tr("Loading..."), tr("Cancel"), 0, 2, this);
			dialog.setWindowTitle(tr("Loading..."));
			dialog.setWindowModality(Qt::WindowModal);
			dialog.setValue(0);
			dialog.show();

			QCoreApplication::processEvents();

			auto motionComponent = behaviour->getComponent<MotionComponent>();
			if (motionComponent)
			{
				auto uuid = item->data(Qt::UserRole).toString().toStdString();
				auto package = motionComponent->getPackage(uuid);

				if (package["path"].is_string())
				{
					auto filepath = package["path"].get<nlohmann::json::string_t>();

					octoon::io::ifstream stream;

					if (stream.open(filepath))
					{
						octoon::VMDLoader loader;
						auto animation = loader.loadMotion(stream);

						dialog.setValue(1);
						QCoreApplication::processEvents();

						if (!animation.clips.empty())
						{
							auto model = selectedItem->object.lock();
							auto animator = model->getComponent<octoon::AnimatorComponent>();
							auto smr = model->getComponent<octoon::SkinnedMeshRendererComponent>();

							if (animator)
								animator->setAnimation(std::move(animation));
							else
							{
								if (smr)
									animator = model->addComponent<octoon::AnimatorComponent>(std::move(animation), smr->getTransforms());
								else
									animator = model->addComponent<octoon::AnimatorComponent>(std::move(animation));
							}

							animator->sample();

							if (smr)
							{
								for (auto& transform : smr->getTransforms())
								{
									auto solver = transform->getComponent<octoon::CCDSolverComponent>();
									if (solver)
										solver->solve();
								}
							}
						}

						behaviour->getComponent<PlayerComponent>()->updateTimeLength();

						dialog.setValue(2);
					}
				}
			}
		}
	}

	void
	MotionDock::resizeEvent(QResizeEvent* e) noexcept
	{
		QMargins margins = mainLayout_->contentsMargins() + topLayout_->contentsMargins() + bottomLayout_->contentsMargins();
		listWidget_->resize(
			this->width(),
			mainWidget_->height() - margins.top() - margins.bottom() - importButton_->height());
	}

	void
	MotionDock::showEvent(QShowEvent* event) noexcept
	{
		QMargins margins = mainLayout_->contentsMargins() + topLayout_->contentsMargins() + bottomLayout_->contentsMargins();
		listWidget_->resize(
			this->width(),
			mainWidget_->height() - margins.top() - margins.bottom() - importButton_->height());

		auto behaviour = behaviour_->getComponent<unreal::UnrealBehaviour>();
		if (behaviour)
		{
			listWidget_->clear();

			auto motionComponent = behaviour->getComponent<MotionComponent>();
			for (auto& uuid : motionComponent->getIndexList())
				this->addItem(uuid.get<nlohmann::json::string_t>());
		}
	}
}