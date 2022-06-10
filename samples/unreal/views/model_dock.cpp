#include "model_dock.h"
#include "../widgets/upushbutton.h"
#include "../widgets/draggable_list_widget.h"

#include <qpainter.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qevent.h>
#include <qapplication.h>
#include <qdrag.h>
#include <qmimedata.h>
#include <qprogressdialog.h>
#include <QToolButton>

#include <octoon/asset_bundle.h>

namespace unreal
{
	ModelDock::ModelDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile)
		: behaviour_(behaviour)
		, profile_(profile)
		, clickedItem_(nullptr)
	{
		this->setWindowTitle(tr("Model Library"));
		this->setObjectName("ModelDock");

		importButton_ = new UPushButton;
		importButton_->setObjectName("Import");
		importButton_->setText(tr("Import"));
		importButton_->setFixedSize(190, 35);
		importButton_->installEventFilter(this);

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

		/*AssetBundle::instance()->getIndexList() += [this](const nlohmann::json& json)
		{
			if (this->isVisible())
			{
				listWidget_->clear();

				for (auto& uuid : AssetBundle::instance()->getIndexList().getValue())
					this->addItem(uuid.get<nlohmann::json::string_t>());
			}
		};*/

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
		auto package = octoon::AssetBundle::instance()->getPackage(uuid);
		if (!package.is_null())
		{
			QLabel* imageLabel = new QLabel;
			imageLabel->setObjectName("preview");
			imageLabel->setFixedSize(QSize(150, 150));
			imageLabel->installEventFilter(this);

			QLabel* nameLabel = new QLabel();
			nameLabel->setObjectName("name");
			nameLabel->setFixedHeight(30);
			nameLabel->installEventFilter(this);

			QVBoxLayout* widgetLayout = new QVBoxLayout;
			widgetLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
			widgetLayout->addWidget(nameLabel, 0, Qt::AlignCenter);
			widgetLayout->setSpacing(0);
			widgetLayout->setContentsMargins(0, 0, 0, 0);

			QWidget* widget = new QWidget;
			widget->setLayout(widgetLayout);

			QListWidgetItem* item = new QListWidgetItem;
			item->setData(Qt::UserRole, QString::fromStdString(package["uuid"].get<nlohmann::json::string_t>()));
			item->setSizeHint(QSize(imageLabel->width(), imageLabel->height() + nameLabel->height()) + QSize(15, 15));

			listWidget_->addItem(item);
			listWidget_->setItemWidget(item, widget);

			if (package.find("preview") != package.end())
			{
				auto filepath = QString::fromStdString(package["preview"].get<nlohmann::json::string_t>());
				imageLabel->setPixmap(QPixmap(filepath).scaled(imageLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
			}
			else
			{
				imageLabel->setPixmap(QPixmap(":res/icons/model.png").scaled(imageLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
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
		try
		{
			if (event->key() == Qt::Key_Delete)
			{
				if (QMessageBox::question(this, tr("Info"), tr("Are you sure you want to delete this model?")) == QMessageBox::Yes)
				{
					if (clickedItem_)
					{
						auto uuid = clickedItem_->data(Qt::UserRole).toString();
						octoon::AssetBundle::instance()->removePackage(uuid.toStdString());
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
	ModelDock::importClickEvent()
	{
		QStringList filepaths = QFileDialog::getOpenFileNames(this, tr("Import Resource"), "", tr("PMX Files (*.pmx)"));
		if (!filepaths.isEmpty())
		{
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

					auto package = octoon::AssetBundle::instance()->importPackage(filepaths[i].toUtf8().toStdString());
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
			auto uuid = item->data(Qt::UserRole).toString().toStdString();
			auto package = octoon::AssetBundle::instance()->getPackage(uuid);
			if (package.is_object())
			{
				QProgressDialog dialog(tr("Loading..."), tr("Cancel"), 0, 1, this);
				dialog.setWindowTitle(tr("Loading..."));
				dialog.setWindowModality(Qt::WindowModal);
				dialog.setLabelText(package["name"].is_string() ? QString::fromStdString(package["name"].get<nlohmann::json::string_t>()) : tr("Unknown-name"));
				dialog.setValue(0);
				dialog.show();

				QCoreApplication::processEvents();

				auto model = octoon::AssetBundle::instance()->loadAssetAtPath<octoon::GameObject>(uuid);
				if (model)
					this->profile_->entitiesModule->objects.getValue().push_back(model);

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

		for (auto& uuid : octoon::AssetBundle::instance()->getModelList())
			this->addItem(uuid.get<nlohmann::json::string_t>());
	}

	bool
	ModelDock::eventFilter(QObject* watched, QEvent* event)
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