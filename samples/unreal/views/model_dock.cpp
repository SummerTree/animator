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
#include <qgraphicseffect.h>
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
		this->setFixedWidth(290);
		this->setFeatures(QDockWidget::NoDockWidgetFeatures);
		
		auto oldTitleBar = this->titleBarWidget();
		this->setTitleBarWidget(new QWidget());
		delete oldTitleBar;

		title_ = new QLabel;
		title_->setObjectName("title");
		title_->setText(tr("Model Library"));
		title_->setContentsMargins(0, 8, 0, 8);
		
		auto headerLine = new QFrame;
		headerLine->setObjectName("HSeparator");
		headerLine->setFrameShape(QFrame::HLine);
		headerLine->setFrameShadow(QFrame::Sunken);
		headerLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		headerLine->setContentsMargins(0, 0, 0, 0);

		topLayout_ = new QVBoxLayout();
		topLayout_->addWidget(title_, 0, Qt::AlignLeft);
		topLayout_->addWidget(headerLine);
		topLayout_->setContentsMargins(4, 0, 4, 0);

		bottomLayout_ = new QHBoxLayout();
		bottomLayout_->addStretch();
		bottomLayout_->setSpacing(2);
		bottomLayout_->setContentsMargins(0, 4, 12, 0);

		listWidget_ = new DraggableListWindow;
		listWidget_->setIconSize(QSize(120, 120));
		listWidget_->setFixedWidth(this->width());
		listWidget_->setStyleSheet("background:transparent;");
		listWidget_->setSpacing(4);
	
		mainLayout_ = new QVBoxLayout();
		mainLayout_->addLayout(topLayout_);
		mainLayout_->addWidget(listWidget_, 0, Qt::AlignCenter);
		mainLayout_->addStretch();
		mainLayout_->addLayout(bottomLayout_);
		mainLayout_->setContentsMargins(0, 8, 0, 8);

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
		if (package.is_object())
		{
			auto item = std::make_unique<QListWidgetItem>();
			item->setData(Qt::UserRole, QString::fromStdString(package["uuid"].get<nlohmann::json::string_t>()));
			item->setSizeHint(listWidget_->iconSize() + QSize(15, 40));

			if (package.contains("preview"))
			{
				auto filepath = QString::fromStdString(package["preview"].get<nlohmann::json::string_t>());
				item->setIcon(QIcon(QPixmap(filepath)));
			}
			else
			{
				item->setIcon(QIcon(QPixmap(":res/icons/model.png")));
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

					auto package = octoon::AssetBundle::instance()->importAsset(filepaths[i].toStdWString());
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
		
		auto uuid = item->data(Qt::UserRole).toString().toStdString();
		auto package = octoon::AssetBundle::instance()->getPackage(uuid);
		if (package.is_object())
		{
			QProgressDialog dialog(tr("Loading..."), tr("Cancel"), 0, 1, this);
			dialog.setWindowTitle(tr("Loading..."));
			dialog.setWindowModality(Qt::WindowModal);
			dialog.setLabelText(package["name"].is_string() ? QString::fromUtf8(package["name"].get<nlohmann::json::string_t>()) : tr("Unknown-name"));
			dialog.setValue(0);
			dialog.show();

			QCoreApplication::processEvents();

			auto model = octoon::AssetBundle::instance()->loadAsset<octoon::GameObject>(uuid);
			if (model)
				this->profile_->entitiesModule->objects.getValue().push_back(model);

			dialog.setValue(1);
		}
	}

	void
	ModelDock::resizeEvent(QResizeEvent* e) noexcept
	{
		QMargins margins = mainLayout_->contentsMargins() + topLayout_->contentsMargins() + bottomLayout_->contentsMargins();
		listWidget_->resize(this->width(), mainWidget_->height() - margins.top() - margins.bottom());
	}

	void
	ModelDock::showEvent(QShowEvent* event) noexcept
	{
		QMargins margins = mainLayout_->contentsMargins() + topLayout_->contentsMargins() + bottomLayout_->contentsMargins();
		listWidget_->resize(this->width(), mainWidget_->height() - margins.top() - margins.bottom());
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