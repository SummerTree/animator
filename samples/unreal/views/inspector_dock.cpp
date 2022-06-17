#include "inspector_dock.h"
#include <octoon/material_importer.h>
#include "../widgets/draggable_list_widget.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <qdrag.h>
#include <qmimedata.h>
#include <qapplication.h>
#include <codecvt>
#include <qpainter.h>
#include <qcolordialog.h>
#include <qtreewidget.h>
#include <qprogressdialog.h>
#include <octoon/asset_database.h>
#include <octoon/asset_bundle.h>

namespace unreal
{
	MaterialListPanel::MaterialListPanel(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile)
		: behaviour_(behaviour)
		, profile_(profile)
	{
		mainWidget_ = new DraggableListWindow;
		mainWidget_->setIconSize(QSize(74, 74));
		mainWidget_->setStyleSheet("background:transparent;");
		mainWidget_->setSpacing(4);

		mainLayout_ = new QVBoxLayout(this);
		mainLayout_->addWidget(mainWidget_);
		mainLayout_->setContentsMargins(0, 0, 0, 0);

		connect(mainWidget_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClicked(QListWidgetItem*)));
		connect(mainWidget_, SIGNAL(itemSelected(QListWidgetItem*)), this, SLOT(itemSelected(QListWidgetItem*)));

		profile->selectorModule->selectedItem_ += [this](const std::optional<octoon::RaycastHit>& data_) {
			if (this->isVisible() && data_.has_value() && !data_.value().object.expired())
			{
				auto hit = data_.value();
				auto hitObject = hit.object.lock();

				if (selectedObject_.expired() || selectedObject_.lock() != hitObject)
				{
					octoon::MaterialImporter::instance()->clear();

					auto renderComponent = hitObject->getComponent<octoon::MeshRendererComponent>();
					if (renderComponent)
					{
						for (auto& mat : renderComponent->getMaterials())
							octoon::MaterialImporter::instance()->addMaterial(mat);
					}
					else
					{
						auto smr = hitObject->getComponent<octoon::SkinnedMeshRendererComponent>();
						if (smr)
						{
							for (auto& mat : smr->getMaterials())
								octoon::MaterialImporter::instance()->addMaterial(mat);
						}
					}

					this->updateItemList();

					selectedObject_ = hitObject;
				}

				auto& sceneList = octoon::MaterialImporter::instance()->getSceneList();
				if (hit.mesh < sceneList.size())
				{
					auto uuid = sceneList[hit.mesh];
					if (!uuid.empty())
					{
						auto count = this->mainWidget_->count();
						for (int i = 0; i < count; i++)
						{
							auto item = this->mainWidget_->item(hit.mesh);
							if (item->data(Qt::UserRole).toString().toStdString() == uuid)
							{
								this->mainWidget_->setCurrentItem(item);
								break;
							}
						}
					}
				}
			}
			else
			{
				selectedObject_.reset();
				octoon::MaterialImporter::instance()->clear();
				this->updateItemList();
			}
		};
	}

	MaterialListPanel::~MaterialListPanel() noexcept
	{
	}

	void
	MaterialListPanel::resizeEvent(QResizeEvent* e) noexcept
	{
		QMargins margins = mainLayout_->contentsMargins();
		mainWidget_->resize(this->width(), this->height() - margins.top() - margins.bottom());
	}

	void
	MaterialListPanel::itemClicked(QListWidgetItem* item)
	{
	}

	void
	MaterialListPanel::itemSelected(QListWidgetItem* item)
	{
		if (item)
		{
			auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
			if (!behaviour)
				return;

			auto selectedItem = behaviour->getProfile()->selectorModule->selectedItemHover_.getValue();
			if (selectedItem)
			{
				auto uuid = item->data(Qt::UserRole).toString().toStdString();
				auto material = octoon::MaterialImporter::instance()->getMaterial(std::string_view(uuid));
				if (material)
				{
					auto hit = selectedItem.value();
					auto meshRenderer = hit.object.lock()->getComponent<octoon::MeshRendererComponent>();
					if (meshRenderer)
						meshRenderer->setMaterial(material, hit.mesh);
				}
			}
		}
	}

	void
	MaterialListPanel::addItem(const nlohmann::json& package) noexcept(false)
	{
		if (package.is_object())
		{
			auto item = std::make_unique<QListWidgetItem>();
			item->setData(Qt::UserRole, QString::fromStdString(package["uuid"].get<nlohmann::json::string_t>()));
			item->setSizeHint(mainWidget_->iconSize() + QSize(8, 35));

			auto material = octoon::MaterialImporter::instance()->getMaterial(std::string_view(package["uuid"].get<nlohmann::json::string_t>()));
			if (material)
			{
				QFontMetrics metrics(item->font());
				item->setToolTip(QString::fromStdString(material->getName()));
				item->setText(metrics.elidedText(item->toolTip(), Qt::ElideRight, mainWidget_->iconSize().width()));

				auto colorTexture = octoon::AssetDatabase::instance()->createMaterialPreview(material);
				if (colorTexture)
				{
					auto width = colorTexture->getTextureDesc().getWidth();
					auto height = colorTexture->getTextureDesc().getHeight();

					std::uint8_t* pixels;
					if (colorTexture->map(0, 0, width, height, 0, (void**)&pixels))
					{
						QImage image(pixels, width, height, QImage::Format_RGBA8888);
						item->setIcon(QPixmap::fromImage(image));

						colorTexture->unmap();
					}
				}
			}
			else
			{
				if (package.contains("preview"))
				{
					auto filepath = QString::fromStdString(package["preview"].get<nlohmann::json::string_t>());
					item->setIcon(QIcon(QPixmap(filepath)));
				}

				if (package.contains("name"))
				{
					QFontMetrics metrics(mainWidget_->font());
					auto name = QString::fromUtf8(package["name"].get<nlohmann::json::string_t>());
					item->setText(metrics.elidedText(name, Qt::ElideRight, mainWidget_->iconSize().width()));
					item->setToolTip(name);
				}
			}

			mainWidget_->addItem(item.release());
		}
	}

	void 
	MaterialListPanel::addItem(std::string_view uuid) noexcept
	{
		auto package = octoon::MaterialImporter::instance()->getPackage(uuid);
		if (package.is_object())
			this->addItem(package);
	}

	void
	MaterialListPanel::updateItemList()
	{
		mainWidget_->clear();

		for (auto& it : octoon::MaterialImporter::instance()->getSceneList())
		{
			try
			{
				this->addItem(std::string_view(it.get<nlohmann::json::string_t>()));
			}
			catch (...)
			{
			}
		}
	}

	InspectorDock::InspectorDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile) noexcept(false)
		: behaviour_(behaviour)
		, profile_(profile)
	{
		this->setObjectName("InspectorDock");
		this->setWindowTitle(tr("Inspector"));
		this->setMouseTracking(true);
		this->setFeatures(QDockWidget::NoDockWidgetFeatures);

		auto oldTitleBar = this->titleBarWidget();
		this->setTitleBarWidget(new QWidget());
		delete oldTitleBar;

		title_ = new QLabel;
		title_->setObjectName("title");
		title_->setText(tr("Inspector"));

		headerLine_ = new QFrame;
		headerLine_->setObjectName("HLine");
		headerLine_->setFixedHeight(1);
		headerLine_->setFrameShape(QFrame::NoFrame);
		headerLine_->setFrameShadow(QFrame::Plain);
		headerLine_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

		titleLayout_ = new QVBoxLayout();
		titleLayout_->addWidget(title_, 0, Qt::AlignLeft);
		titleLayout_->addSpacing(8);
		titleLayout_->addWidget(headerLine_);
		titleLayout_->setSpacing(0);
		titleLayout_->setContentsMargins(4, 0, 4, 0);
		
		materialList_ = new MaterialListPanel(behaviour, profile);

		modifyWidget_ = new MaterialEditWindow(behaviour);
		modifyWidget_->hide();
		
		mainLayout_ = new QVBoxLayout;
		mainLayout_->addLayout(titleLayout_);
		mainLayout_->addWidget(materialList_);
		mainLayout_->addWidget(modifyWidget_);
		mainLayout_->setContentsMargins(0, 12, 0, 8);

		mainWidget_ = new QWidget;
		mainWidget_->setLayout(mainLayout_);

		this->setWidget(mainWidget_);

		connect(modifyWidget_->backButton_, SIGNAL(clicked()), this, SLOT(backEvent()));
		connect(materialList_->mainWidget_, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));

		profile->selectorModule->selectedItem_ += [this](const std::optional<octoon::RaycastHit>& data_) {
			if (!data_.has_value() || data_.value().object.expired())
				this->backEvent();
		};
	}

	InspectorDock::~InspectorDock() noexcept
	{
	}

	void
	InspectorDock::resizeEvent(QResizeEvent* e) noexcept
	{
		QMargins margins = mainLayout_->contentsMargins() + titleLayout_->contentsMargins();

		modifyWidget_->resize(mainWidget_->width(), mainWidget_->height() - margins.top() - margins.bottom() - title_->height());
		materialList_->resize(mainWidget_->width(), mainWidget_->height() - margins.top() - margins.bottom() - title_->height());
	}

	void
	InspectorDock::showEvent(QShowEvent* event) noexcept
	{
		QMargins margins = mainLayout_->contentsMargins() + titleLayout_->contentsMargins();

		modifyWidget_->hide();
		modifyWidget_->resize(mainWidget_->width(), mainWidget_->height() - margins.top() - margins.bottom() - title_->height());

		materialList_->resize(mainWidget_->width(), mainWidget_->height() - margins.top() - margins.bottom() - title_->height());
		materialList_->show();
		materialList_->updateItemList();

		profile_->selectorModule->selectedItem_.submit();
	}

	void
	InspectorDock::backEvent()
	{
		modifyWidget_->hide();
		materialList_->show();
		materialList_->updateItemList();
		title_->setText(tr("Inspector"));
	}

	void
	InspectorDock::itemDoubleClicked(QListWidgetItem* item)
	{
		if (behaviour_)
		{
			auto behaviour = behaviour_->getComponent<unreal::UnrealBehaviour>();
			if (behaviour)
			{
				auto uuid = item->data(Qt::UserRole).toString().toStdString();
				auto material = octoon::MaterialImporter::instance()->getMaterial(std::string_view(uuid));
				if (material)
				{
					title_->setText(tr("Material Properties"));

					octoon::AssetDatabase::instance()->addUpdateList(uuid);

					selectedItem_ = item;
					materialList_->hide();

					modifyWidget_->setMaterial(material);
					modifyWidget_->show();
				}
			}
		}
	}
}