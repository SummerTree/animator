#include "motion_dock.h"
#include "../widgets/draggable_list_widget.h"
#include <octoon/asset_database.h>
#include <octoon/asset_bundle.h>
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
		this->setFixedWidth(360);
		this->setFeatures(QDockWidget::NoDockWidgetFeatures);

		auto oldTitleBar = this->titleBarWidget();
		this->setTitleBarWidget(new QWidget());
		delete oldTitleBar;

		title_ = new QLabel;
		title_->setObjectName("title");
		title_->setText(tr("Motion Library"));
		title_->setContentsMargins(0, 10, 0, 10);

		auto headerLine = new QFrame;
		headerLine->setObjectName("Separator");
		headerLine->setFrameShape(QFrame::HLine);
		headerLine->setFrameShadow(QFrame::Sunken);
		headerLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		headerLine->setContentsMargins(0, 10, 0, 10);

		topLayout_ = new QVBoxLayout();
		topLayout_->addWidget(title_, 0, Qt::AlignLeft);
		topLayout_->addWidget(headerLine);
		topLayout_->setContentsMargins(5, 0, 5, 0);

		bottomLayout_ = new QHBoxLayout();
		bottomLayout_->addStretch();
		bottomLayout_->setSpacing(2);
		bottomLayout_->setContentsMargins(0, 5, 15, 0);

		listWidget_ = new DraggableListWindow;
		listWidget_->setFixedWidth(this->width());
		listWidget_->setSelectionMode(QListWidget::NoSelection);
		listWidget_->setIconSize(QSize(100, 100));
		listWidget_->setStyleSheet("background:transparent;");
		listWidget_->setSpacing(5);
		listWidget_->installEventFilter(this);
		
		mainLayout_ = new QVBoxLayout();
		mainLayout_->addLayout(topLayout_);
		mainLayout_->addWidget(listWidget_, 0, Qt::AlignCenter);
		mainLayout_->addStretch();
		mainLayout_->addLayout(bottomLayout_);
		mainLayout_->setContentsMargins(0, 10, 0, 10);

		mainWidget_ = new QWidget;
		mainWidget_->setObjectName("MotionWidget");
		mainWidget_->setLayout(mainLayout_);
		mainWidget_->installEventFilter(this);

		this->setWidget(mainWidget_);

		/*MotionImporter::instance()->getIndexList() += [this](const nlohmann::json& json)
		{
			if (this->isVisible())
			{
				listWidget_->clear();

				for (auto& uuid : MotionImporter::instance()->getIndexList())
					this->addItem(uuid.get<nlohmann::json::string_t>());
			}
		};*/

		connect(listWidget_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClicked(QListWidgetItem*)));
		connect(listWidget_, SIGNAL(itemSelected(QListWidgetItem*)), this, SLOT(itemSelected(QListWidgetItem*)));
	}

	MotionDock::~MotionDock() noexcept
	{
	}

	void 
	MotionDock::addItem(std::string_view uuid) noexcept
	{
		auto package = octoon::AssetBundle::instance()->getPackage(uuid);
		if (package.is_object())
		{
			auto item = std::make_unique<QListWidgetItem>();
			item->setData(Qt::UserRole, QString::fromStdString(package["uuid"].get<nlohmann::json::string_t>()));
			item->setSizeHint(listWidget_->iconSize() + QSize(11, 50));

			if (package.contains("preview"))
			{
				auto filepath = QString::fromStdString(package["preview"].get<nlohmann::json::string_t>());
				item->setIcon(QIcon(QPixmap(filepath).scaled(listWidget_->iconSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
			}
			else
			{
				item->setIcon(QIcon(QPixmap(":res/icons/dance2.png").scaled(listWidget_->iconSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
			}

			if (package.contains("name"))
			{
				QFontMetrics metrics(mainWidget_->font());
				auto name = QString::fromUtf8(package["name"].get<nlohmann::json::string_t>());
				item->setText(metrics.elidedText(name, Qt::ElideRight, listWidget_->iconSize().width()));
				item->setToolTip(name);
			}

			listWidget_->addItem(item.release());
		}
	}

	void
	MotionDock::keyPressEvent(QKeyEvent * event) noexcept
	{
		try
		{
			if (event->key() == Qt::Key_Delete)
			{
				if (QMessageBox::question(this, tr("Info"), tr("Are you sure you want to delete this motion data?")) == QMessageBox::Yes)
				{
					if (clickedItem_)
					{
						auto uuid = clickedItem_->data(Qt::UserRole).toString();
						octoon::AssetBundle::instance()->removeAsset(uuid.toStdString());
						listWidget_->takeItem(listWidget_->row(clickedItem_));
						delete clickedItem_;
						clickedItem_ = listWidget_->currentItem();
						octoon::AssetBundle::instance()->saveAssets();
					}
				}
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(this, tr("Error"), e.what());
		}
	}

	void
	MotionDock::importClickEvent()
	{
		QStringList filepaths = QFileDialog::getOpenFileNames(this, tr("Import Resource"), "", tr("VMD Files (*.vmd)"));
		if (!filepaths.isEmpty())
		{
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

						auto package = octoon::AssetBundle::instance()->importAsset(filepaths[i].toStdWString());
						if (!package.is_null())
							this->addItem(package["uuid"].get<nlohmann::json::string_t>());
					}
				}
				else
				{
					auto package = octoon::AssetBundle::instance()->importAsset(filepaths[0].toStdWString());
					if (!package.is_null())
						this->addItem(package["uuid"].get<nlohmann::json::string_t>());
				}

				octoon::AssetBundle::instance()->saveAssets();
			}
			catch (...)
			{
				octoon::AssetBundle::instance()->saveAssets();
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

			QProgressDialog dialog(tr("Loading..."), tr("Cancel"), 0, 2, this);
			dialog.setWindowTitle(tr("Loading..."));
			dialog.setWindowModality(Qt::WindowModal);
			dialog.setValue(0);
			dialog.show();

			QCoreApplication::processEvents();

			auto uuid = item->data(Qt::UserRole).toString().toStdString();
			auto package = octoon::AssetBundle::instance()->getPackage(uuid);
			if (package.is_object())
			{
				auto animation = octoon::AssetBundle::instance()->loadAsset<octoon::Animation>(uuid);
				if (animation)
				{
					dialog.setValue(1);
					QCoreApplication::processEvents();

					if (animation->hasClip("Camera"))
						profile_->cameraModule->animation = animation;
					else
					{
						auto selectedItem = behaviour->getProfile()->selectorModule->selectedItemHover_.getValue();
						if (selectedItem.has_value())
						{
							if (animation && !animation->clips.empty())
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

								animator->setName(package["path"].get<nlohmann::json::string_t>());
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
						}
					}

					behaviour->getComponent<PlayerComponent>()->updateTimeLength();
				}

				dialog.setValue(2);
			}
		}
	}

	void
	MotionDock::resizeEvent(QResizeEvent* e) noexcept
	{
		QMargins margins = mainLayout_->contentsMargins() + topLayout_->contentsMargins() + bottomLayout_->contentsMargins();
		listWidget_->resize(
			this->width(),
			mainWidget_->height() - margins.top() - margins.bottom());
	}

	void
	MotionDock::showEvent(QShowEvent* event) noexcept
	{
		QMargins margins = mainLayout_->contentsMargins() + topLayout_->contentsMargins() + bottomLayout_->contentsMargins();
		listWidget_->resize(
			this->width(),
			mainWidget_->height() - margins.top() - margins.bottom());

		listWidget_->clear();

		for (auto& uuid : octoon::AssetBundle::instance()->getMotionList())
			this->addItem(uuid.get<nlohmann::json::string_t>());
	}

	bool
	MotionDock::eventFilter(QObject* watched, QEvent* event)
	{
		if (event->type() != QEvent::Paint &&
			event->type() != QEvent::Resize)
		{
			if (profile_->playerModule->isPlaying)
			{
				return true;
			}
		}

		return QWidget::eventFilter(watched, event);
	}
}