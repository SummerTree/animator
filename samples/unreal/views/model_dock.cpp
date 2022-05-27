#include "model_dock.h"
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
	ModelDock::ModelDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile)
		: behaviour_(behaviour)
		, profile_(profile)
		, clickedItem_(nullptr)
	{
		this->setWindowTitle(tr("Model Library"));
		this->setObjectName("ModelDock");

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
		mainWidget_->setObjectName("ModelWidget");
		mainWidget_->setLayout(mainLayout_);

		this->setWidget(mainWidget_);

		this->profile_->resourceModule->modelIndexList_ += [this](const nlohmann::json& json)
		{
			if (this->isVisible())
			{
				listWidget_->clear();

				for (auto& uuid : this->profile_->resourceModule->modelIndexList_.getValue())
					this->addItem(uuid.get<nlohmann::json::string_t>());
			}
		};

		connect(importButton_, SIGNAL(clicked()), this, SLOT(importClickEvent()));
		connect(listWidget_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClicked(QListWidgetItem*)));
		connect(listWidget_, SIGNAL(itemSelected(QListWidgetItem*)), this, SLOT(itemSelected(QListWidgetItem*)));
	}

	ModelDock::~ModelDock() noexcept
	{
	}

	void 
	ModelDock::addItem(std::string_view uuid) noexcept
	{
		auto hdrComponent = behaviour_->getComponent<UnrealBehaviour>()->getComponent<ModelComponent>();
		if (!hdrComponent)
			return;

		auto package = hdrComponent->getPackage(uuid);
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
				imageLabel->setPixmap(QPixmap(":res/icons/model.png").scaled(100, 100));
			}

			if (package.find("name") != package.end())
			{
				QFontMetrics metrics(nameLabel->font());

				auto name = QString::fromStdString(package["name"].get<nlohmann::json::string_t>());
				imageLabel->setToolTip(name);
				name.truncate(name.lastIndexOf('.'));
				nameLabel->setText(metrics.elidedText(name, Qt::ElideRight, imageLabel->width()));
			}
		}
	}

	void
	ModelDock::keyPressEvent(QKeyEvent * event) noexcept
	{
		if (event->key() == Qt::Key_Delete)
		{
			if (QMessageBox::question(this, tr("Info"), tr("Are you sure you want to delete this model?")) == QMessageBox::Yes)
			{
				if (clickedItem_)
				{
					auto modelComponent = behaviour_->getComponent<UnrealBehaviour>()->getComponent<ModelComponent>();
					if (modelComponent)
					{
						auto uuid = clickedItem_->data(Qt::UserRole).toString();
						if (modelComponent->removePackage(uuid.toStdString()))
						{
							listWidget_->takeItem(listWidget_->row(clickedItem_));
							delete clickedItem_;
							clickedItem_ = listWidget_->currentItem();
							modelComponent->save();
						}
					}
				}
			}
		}
	}

	void
	ModelDock::importClickEvent()
	{
		QStringList filepaths = QFileDialog::getOpenFileNames(this, tr("Import Resource"), "", tr("PMX Files (*.pmx)"));
		if (!filepaths.isEmpty())
		{
			auto hdrComponent = behaviour_->getComponent<UnrealBehaviour>()->getComponent<ModelComponent>();
			if (!hdrComponent)
				return;

			try
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

					auto package = hdrComponent->importModel(filepaths[i].toStdString());
					if (!package.is_null())
						this->addItem(package["uuid"].get<nlohmann::json::string_t>());
				}

				hdrComponent->save();
			}
			catch (...)
			{
				hdrComponent->save();
			}
		}
	}

	void
	ModelDock::itemClicked(QListWidgetItem* item)
	{
		clickedItem_ = item;
	}

	void
	ModelDock::itemSelected(QListWidgetItem* item)
	{
		if (!item)
			return;
		
		auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
		if (behaviour)
		{
			auto modelComponent = behaviour->getComponent<ModelComponent>();
			if (modelComponent)
			{
				auto uuid = item->data(Qt::UserRole).toString().toStdString();
				auto package = modelComponent->getPackage(uuid);

				QProgressDialog dialog(tr("Loading..."), tr("Cancel"), 0, 1, this);
				dialog.setWindowTitle(tr("Loading..."));
				dialog.setWindowModality(Qt::WindowModal);
				dialog.setValue(0);
				dialog.show();

				if (package["name"].is_string())
					dialog.setLabelText(QString::fromStdString(package["name"].get<nlohmann::json::string_t>()));

				QCoreApplication::processEvents();

				if (package["path"].is_string())
					behaviour->load(package["path"].get<nlohmann::json::string_t>());

				dialog.setValue(1);
			}
		}
	}

	void
	ModelDock::resizeEvent(QResizeEvent* e) noexcept
	{
		QMargins margins = mainLayout_->contentsMargins() + topLayout_->contentsMargins() + bottomLayout_->contentsMargins();
		listWidget_->resize(
			this->width(),
			mainWidget_->height() - margins.top() - margins.bottom() - importButton_->height());
	}

	void
	ModelDock::showEvent(QShowEvent* event) noexcept
	{
		QMargins margins = mainLayout_->contentsMargins() + topLayout_->contentsMargins() + bottomLayout_->contentsMargins();
		listWidget_->resize(this->width(), mainWidget_->height() - margins.top() - margins.bottom() - importButton_->height());
		listWidget_->clear();

		for (auto& uuid : this->profile_->resourceModule->modelIndexList_.getValue())
			this->addItem(uuid.get<nlohmann::json::string_t>());
	}
}