#include "environment_dock.h"
#include <octoon/environment_light_component.h>
#include <octoon/mesh_renderer_component.h>
#include <octoon/image/image.h>
#include <octoon/PMREM_loader.h>

#include <qpainter.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qevent.h>
#include <qapplication.h>
#include <qdrag.h>
#include <qmimedata.h>
#include <qprogressdialog.h>

namespace flower
{
	class DoubleSpinBox final : public QDoubleSpinBox
	{
	public:
		void focusInEvent(QFocusEvent* event) override
		{
			this->grabKeyboard();
			QDoubleSpinBox::focusInEvent(event);
		}

		void focusOutEvent(QFocusEvent* event) override
		{
			this->releaseKeyboard();
			QDoubleSpinBox::focusOutEvent(event);
		}
	};

	EnvironmentListDialog::EnvironmentListDialog(QWidget* parent, const octoon::GameObjectPtr& behaviour, const std::shared_ptr<FlowerProfile>& profile)
		: QDialog(parent)
		, behaviour_(behaviour)
		, profile_(profile)
		, clickedItem_(nullptr)
	{
		this->setObjectName("EnvironmentDialog");
		this->setWindowTitle(tr("Environment Resource"));
		this->setFixedSize(900, 600);
		this->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);

		okButton_ = new QToolButton;
		okButton_->setText(tr("Ok"));
		okButton_->setFixedSize(50, 30);

		closeButton_ = new QToolButton;
		closeButton_->setText(tr("Close"));
		closeButton_->setFixedSize(50, 30);

		importButton_ = new QToolButton;
		importButton_->setText(tr("Import"));
		importButton_->setFixedSize(60, 30);

		auto topLayout_ = new QHBoxLayout();
		topLayout_->addWidget(importButton_, 0, Qt::AlignLeft);
		topLayout_->addStretch();
		topLayout_->setContentsMargins(15, 0, 0, 0);

		auto bottomLayout_ = new QHBoxLayout();
		bottomLayout_->addStretch();
		bottomLayout_->addWidget(okButton_, 0, Qt::AlignRight);
		bottomLayout_->addWidget(closeButton_, 0, Qt::AlignRight);
		bottomLayout_->setSpacing(2);
		bottomLayout_->setContentsMargins(0, 5, 15, 0);

		mainWidget_ = new QListWidget;
		mainWidget_->setResizeMode(QListView::Fixed);
		mainWidget_->setViewMode(QListView::IconMode);
		mainWidget_->setMovement(QListView::Static);
		mainWidget_->setDefaultDropAction(Qt::DropAction::MoveAction);
		mainWidget_->setStyleSheet("background:transparent;");
		mainWidget_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		mainWidget_->setSpacing(10);

		mainLayout_ = new QVBoxLayout(this);
		mainLayout_->addLayout(topLayout_);
		mainLayout_->addWidget(mainWidget_);
		mainLayout_->addStretch();
		mainLayout_->addLayout(bottomLayout_);
		mainLayout_->setContentsMargins(5, 10, 5, 10);

		connect(okButton_, SIGNAL(clicked()), this, SLOT(okClickEvent()));
		connect(closeButton_, SIGNAL(clicked()), this, SLOT(closeClickEvent()));
		connect(importButton_, SIGNAL(clicked()), this, SLOT(importClickEvent()));
		connect(mainWidget_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClicked(QListWidgetItem*)));
		connect(mainWidget_, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));
	}

	EnvironmentListDialog::~EnvironmentListDialog() noexcept
	{
	}

	void 
	EnvironmentListDialog::addItem(std::string_view uuid) noexcept
	{
		auto hdrComponent = behaviour_->getComponent<FlowerBehaviour>()->getComponent<HDRiComponent>();
		if (!hdrComponent)
			return;

		auto package = hdrComponent->getPackage(uuid);
		if (!package.is_null())
		{
			QLabel* imageLabel = new QLabel;
			imageLabel->setFixedSize(QSize(200, 100));

			QLabel* nameLabel = new QLabel();
			nameLabel->setFixedHeight(30);

			QVBoxLayout* widgetLayout = new QVBoxLayout;
			widgetLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
			widgetLayout->addWidget(nameLabel, 0, Qt::AlignCenter);
			widgetLayout->setSpacing(0);
			widgetLayout->setContentsMargins(0, 0, 0, 0);

			QWidget* widget = new QWidget;
			widget->setLayout(widgetLayout);

			QListWidgetItem* item = new QListWidgetItem;
			item->setData(Qt::UserRole, QString::fromStdString(std::string(uuid)));
			item->setSizeHint(QSize(imageLabel->width(), imageLabel->height() + nameLabel->height()) + QSize(10, 10));

			mainWidget_->addItem(item);
			mainWidget_->setItemWidget(item, widget);

			if (package.find("preview") != package.end())
			{
				auto filepath = package["preview"].get<nlohmann::json::string_t>();
				imageLabel->setPixmap(QPixmap(QString::fromStdString(filepath)).scaled(imageLabel->size()));
			}

			if (package.find("name") != package.end())
			{
				QFontMetrics metrics(nameLabel->font());

				auto name = QString::fromStdString(package["name"].get<nlohmann::json::string_t>());
				nameLabel->setText(metrics.elidedText(name, Qt::ElideRight, imageLabel->width()));
				imageLabel->setToolTip(name);
			}
		}
	}

	void
	EnvironmentListDialog::importClickEvent()
	{
		QStringList filepaths = QFileDialog::getOpenFileNames(this, u8"导入图像", "", tr("HDRi Files (*.hdr)"));
		if (!filepaths.isEmpty())
		{
			auto hdrComponent = behaviour_->getComponent<FlowerBehaviour>()->getComponent<HDRiComponent>();

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

					auto package = hdrComponent->importHDRi(filepaths[i].toStdString());
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
	EnvironmentListDialog::itemClicked(QListWidgetItem* item)
	{
		clickedItem_ = item;
	}

	void
	EnvironmentListDialog::itemDoubleClicked(QListWidgetItem* item)
	{
		this->close();

		if (item)
		{
			emit itemSelected(item);
		}
	}

	void
	EnvironmentListDialog::okClickEvent()
	{
		this->close();

		if (clickedItem_)
		{
			emit itemSelected(clickedItem_);
		}
	}

	void
	EnvironmentListDialog::closeClickEvent()
	{
		this->close();
	}

	void
	EnvironmentListDialog::resizeEvent(QResizeEvent* e) noexcept
	{
		QMargins margins = mainLayout_->contentsMargins();
		mainWidget_->resize(
			this->width(),
			this->height() - (margins.top() + margins.bottom()) * 2 - okButton_->height() - importButton_->height());
	}

	void
	EnvironmentListDialog::showEvent(QShowEvent* event) noexcept
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
		{
			mainWidget_->clear();

			auto hdriComponent = behaviour->getComponent<HDRiComponent>();
			for (auto& uuid : hdriComponent->getIndexList())
				this->addItem(uuid.get<nlohmann::json::string_t>());
		}
	}

	EnvironmentDock::EnvironmentDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<FlowerProfile>& profile)
		: behaviour_(behaviour)
		, profile_(profile)
		, environmentListDialog_(nullptr)
	{
		this->setWindowTitle(tr("Environment Light"));
		this->setObjectName("EnvironmentDock");

		this->previewButton_ = new QToolButton();
		this->previewButton_->setFixedSize(QSize(260, 130));
		this->previewButton_->setIconSize(this->previewButton_->size());
		this->previewButton_->setToolTip(tr("Click the select a Preview button to locate each HDRi on your computer"));

		this->previewName_ = new QLabel;
		this->previewName_->setText(tr("Untitled"));
		this->previewName_->setAlignment(Qt::AlignCenter);
		this->previewName_->setMinimumWidth(260);

		this->colorButton = new QToolButton;
		this->colorButton->setIconSize(QSize(50, 30));
		
		this->thumbnail = new QToolButton;
		this->thumbnail->setIcon(QIcon::fromTheme(":res/icons/append2.png"));
		this->thumbnail->setIconSize(QSize(48, 48));
		this->thumbnail->setToolTip(tr("Open"));

		this->thumbnailToggle = new QCheckBox;
		this->thumbnailToggle->setText(tr("Thumbnail"));

		this->backgroundToggle = new QCheckBox;
		this->backgroundToggle->setText(tr("Toggle Background"));
		this->backgroundToggle->setChecked(true);
		
		this->thumbnailPath = new QLabel;
		this->thumbnailPath->setMinimumSize(QSize(160, 20));

		this->intensityLabel_ = new QLabel;
		this->intensityLabel_->setText(tr("Intensity"));

		this->intensitySlider = new QSlider(Qt::Horizontal);
		this->intensitySlider->setObjectName("Value");
		this->intensitySlider->setMinimum(0);
		this->intensitySlider->setMaximum(100);
		this->intensitySlider->setValue(0);
		this->intensitySlider->setMinimumWidth(270);

		this->intensitySpinBox = new DoubleSpinBox;
		this->intensitySpinBox->setFixedWidth(50);
		this->intensitySpinBox->setMaximum(10.0f);
		this->intensitySpinBox->setSingleStep(0.1f);
		this->intensitySpinBox->setAlignment(Qt::AlignRight);
		this->intensitySpinBox->setValue(0.0f);
		this->intensitySpinBox->setDecimals(1);

		this->horizontalRotationLabel_ = new QLabel;
		this->horizontalRotationLabel_->setText(tr("Horizontal Rotation"));

		this->horizontalRotationSlider = new QSlider(Qt::Horizontal);
		this->horizontalRotationSlider->setObjectName("Value");
		this->horizontalRotationSlider->setMinimum(-100);
		this->horizontalRotationSlider->setMaximum(100);
		this->horizontalRotationSlider->setValue(0);
		this->horizontalRotationSlider->setMinimumWidth(270);

		this->horizontalRotationSpinBox = new DoubleSpinBox;
		this->horizontalRotationSpinBox->setFixedWidth(50);
		this->horizontalRotationSpinBox->setMinimum(-1.0f);
		this->horizontalRotationSpinBox->setMaximum(1.0f);
		this->horizontalRotationSpinBox->setSingleStep(0.03f);
		this->horizontalRotationSpinBox->setAlignment(Qt::AlignRight);
		this->horizontalRotationSpinBox->setValue(0.0f);

		this->verticalRotationLabel_ = new QLabel;
		this->verticalRotationLabel_->setText(tr("Vertical Rotation"));

		this->verticalRotationSlider = new QSlider(Qt::Horizontal);
		this->verticalRotationSlider->setObjectName("Value");
		this->verticalRotationSlider->setMinimum(-100);
		this->verticalRotationSlider->setMaximum(100);
		this->verticalRotationSlider->setValue(0);
		this->verticalRotationSlider->setMinimumWidth(270);

		this->verticalRotationSpinBox = new DoubleSpinBox;
		this->verticalRotationSpinBox->setFixedWidth(50);
		this->verticalRotationSpinBox->setMinimum(-1.0f);
		this->verticalRotationSpinBox->setMaximum(1.0f);
		this->verticalRotationSpinBox->setSingleStep(0.03f);
		this->verticalRotationSpinBox->setAlignment(Qt::AlignRight);
		this->verticalRotationSpinBox->setValue(0.0f);

		this->resetButton_ = new QToolButton();
		this->resetButton_->setText(tr("Reset"));
	
		auto thumbnailTitleLayout = new QHBoxLayout();
		thumbnailTitleLayout->addWidget(thumbnailToggle, 0, Qt::AlignLeft);
		thumbnailTitleLayout->addSpacing(4);
		thumbnailTitleLayout->addWidget(backgroundToggle, 0, Qt::AlignLeft);
		thumbnailTitleLayout->addStretch();
		thumbnailTitleLayout->setSpacing(0);
		thumbnailTitleLayout->setContentsMargins(0, 2, 0, 0);

		auto thumbnailTextLayout = new QHBoxLayout;
		thumbnailTextLayout->setSpacing(2);
		thumbnailTextLayout->setContentsMargins(0, 2, 0, 0);
		thumbnailTextLayout->addWidget(this->thumbnailPath, 0, Qt::AlignLeft | Qt::AlignCenter);
		thumbnailTextLayout->addStretch();
		thumbnailTextLayout->addWidget(this->colorButton, 0, Qt::AlignRight);

		auto thumbnailRightLayout = new QVBoxLayout;
		thumbnailRightLayout->setSpacing(0);
		thumbnailRightLayout->setContentsMargins(0, 0, 0, 0);
		thumbnailRightLayout->addLayout(thumbnailTitleLayout);
		thumbnailRightLayout->addLayout(thumbnailTextLayout);
		thumbnailRightLayout->addStretch();

		auto thumbnailLayout = new QHBoxLayout;
		thumbnailLayout->addWidget(thumbnail);
		thumbnailLayout->addLayout(thumbnailRightLayout);

		auto intensityLayout = new QHBoxLayout();
		intensityLayout->addWidget(this->intensityLabel_, 0, Qt::AlignLeft);
		intensityLayout->addWidget(this->intensitySpinBox, 0, Qt::AlignRight);

		auto horizontalRotationLayout = new QHBoxLayout();
		horizontalRotationLayout->addWidget(this->horizontalRotationLabel_, 0, Qt::AlignLeft);
		horizontalRotationLayout->addWidget(this->horizontalRotationSpinBox, 0, Qt::AlignRight);

		auto verticalRotationLayout = new QHBoxLayout();
		verticalRotationLayout->addWidget(this->verticalRotationLabel_, 0, Qt::AlignLeft);
		verticalRotationLayout->addWidget(this->verticalRotationSpinBox, 0, Qt::AlignRight);

		auto spoilerLayout = new QVBoxLayout();
		spoilerLayout->addLayout(thumbnailLayout);
		spoilerLayout->addLayout(intensityLayout);
		spoilerLayout->addWidget(this->intensitySlider);
		spoilerLayout->addLayout(horizontalRotationLayout);
		spoilerLayout->addWidget(this->horizontalRotationSlider);
		spoilerLayout->addLayout(verticalRotationLayout);
		spoilerLayout->addWidget(this->verticalRotationSlider);
		spoilerLayout->setContentsMargins(20, 0, 20, 0);

		this->spoiler = new Spoiler(tr("Attribute"));
		this->spoiler->setContentLayout(*spoilerLayout);
		this->spoiler->toggleButton.click();

		auto imageLayout = new QHBoxLayout();
		imageLayout->addStretch();
		imageLayout->addWidget(previewButton_, 0, Qt::AlignCenter);
		imageLayout->addStretch();

		auto mainLayout = new QVBoxLayout();
		mainLayout->addLayout(imageLayout);
		mainLayout->addWidget(previewName_, 0, Qt::AlignCenter);
		mainLayout->addWidget(spoiler);
		mainLayout->addStretch();
		mainLayout->addWidget(resetButton_, 0, Qt::AlignBottom | Qt::AlignRight);
		mainLayout->setContentsMargins(10, 10, 10, 10);

		auto mainWidget = new QWidget();
		mainWidget->setLayout(mainLayout);

		this->setWidget(mainWidget);

		connect(previewButton_, SIGNAL(clicked(bool)), this, SLOT(previewClickEvent(bool)));
		connect(thumbnail, SIGNAL(clicked()), this, SLOT(thumbnailClickEvent()));
		connect(thumbnailToggle, SIGNAL(stateChanged(int)), this, SLOT(thumbnailToggleEvent(int)));
		connect(backgroundToggle, SIGNAL(stateChanged(int)), this, SLOT(backgroundMapCheckEvent(int)));
		connect(colorButton, SIGNAL(clicked()), this, SLOT(colorClickEvent()));
		connect(intensitySpinBox, SIGNAL(valueChanged(double)), this, SLOT(intensityEditEvent(double)));
		connect(intensitySlider, SIGNAL(valueChanged(int)), this, SLOT(intensitySliderEvent(int)));
		connect(horizontalRotationSpinBox, SIGNAL(valueChanged(double)), this, SLOT(horizontalRotationEditEvent(double)));
		connect(horizontalRotationSlider, SIGNAL(valueChanged(int)), this, SLOT(horizontalRotationSliderEvent(int)));
		connect(verticalRotationSpinBox, SIGNAL(valueChanged(double)), this, SLOT(verticalRotationEditEvent(double)));
		connect(verticalRotationSlider, SIGNAL(valueChanged(int)), this, SLOT(verticalRotationSliderEvent(int)));
		connect(&colorSelector_, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(colorChangeEvent(const QColor&)));
		connect(resetButton_, SIGNAL(clicked()), this, SLOT(resetEvent()));
	}

	EnvironmentDock::~EnvironmentDock()
	{
		this->previewImage_.reset();

		if (environmentListDialog_)
			delete environmentListDialog_;
	}

	void
	EnvironmentDock::setColor(const QColor& c, int w, int h)
	{
		this->profile_->environmentModule->color = octoon::math::float3(c.redF(), c.greenF(), c.blueF());

		if (profile_->entitiesModule->enviromentLight)
		{
			auto environmentLight = profile_->entitiesModule->enviromentLight->getComponent<octoon::EnvironmentLightComponent>();
			if (environmentLight)
				environmentLight->setColor(octoon::math::srgb2linear(profile_->environmentModule->color));

			auto meshRenderer = profile_->entitiesModule->enviromentLight->getComponent<octoon::MeshRendererComponent>();
			if (meshRenderer)
				meshRenderer->getMaterial()->set("diffuse", octoon::math::srgb2linear(profile_->environmentModule->color) * profile_->environmentModule->intensity);
		}

		QPixmap pixmap(w, h);
		QPainter painter(&pixmap);
		painter.setPen(Qt::NoPen);
		painter.fillRect(QRect(0, 0, w, h), c);
		this->colorButton->setIcon(QIcon(pixmap));
	}

	void
	EnvironmentDock::setPreviewImage(QString name, std::shared_ptr<QImage> image)
	{
		QFontMetrics previewMetrics(this->previewName_->font());

		this->previewImage_ = image;
		this->previewName_->setText(previewMetrics.elidedText(name, Qt::ElideRight, this->previewName_->width()));
	}

	void
	EnvironmentDock::setThumbnailImage(QString name, const QImage& image)
	{
		QFontMetrics thumbnailMetrics(this->thumbnailPath->font());

		this->thumbnailPath->setToolTip(name);
		this->thumbnailPath->setText(thumbnailMetrics.elidedText(this->thumbnailPath->toolTip(), Qt::ElideRight, this->thumbnailPath->width()));
		this->thumbnail->setIcon(QIcon(QPixmap::fromImage(image.scaled(QSize(48, 30)))));
		this->thumbnailToggle->setChecked(false);
		this->thumbnailToggle->setChecked(true);
	}

	void
	EnvironmentDock::updatePreviewImage()
	{
		auto w = this->previewButton_->width();
		auto h = this->previewButton_->height();
		auto c = QColor::fromRgbF(profile_->environmentModule->color.x, profile_->environmentModule->color.y, profile_->environmentModule->color.z);

		if (this->previewImage_ && this->thumbnailToggle->isChecked())
		{
			auto srcWidth = this->previewImage_->width();
			auto srcHeight = this->previewImage_->height();
			auto pixels = std::make_unique<std::uint8_t[]>(w * h * 3);
			auto offset = this->profile_->environmentModule->offset;

			for (std::size_t y = 0; y < h; y++)
			{
				for (std::size_t x = 0; x < w; x++)
				{
					auto u = x / float(w) - offset.x;
					auto v = y / float(h) - offset.y;
					u -= std::floor(u);
					v -= std::floor(v);
					auto ui = int(u * srcWidth);
					auto vi = int(v * srcHeight);

					auto dst = (y * w + x) * 3;
					auto color = this->previewImage_->pixelColor(ui, vi);

					pixels[dst] = std::clamp<float>(color.redF() * c.red(), 0, 255);
					pixels[dst + 1] = std::clamp<float>(color.greenF() * c.green(), 0, 255);
					pixels[dst + 2] = std::clamp<float>(color.blueF() * c.blue(), 0, 255);
				}
			}

			previewButton_->setIcon(QPixmap::fromImage(QImage(pixels.get(), w, h, QImage::Format::Format_RGB888)));
		}
		else
		{
			QPixmap pixmap(w, h);
			QPainter painter(&pixmap);
			painter.setPen(Qt::NoPen);
			painter.fillRect(QRect(0, 0, w, h), c);
			previewButton_->setIcon(pixmap);
		}
	}

	void
	EnvironmentDock::previewClickEvent(bool checked)
	{
		if (!environmentListDialog_)
		{
			environmentListDialog_ = new EnvironmentListDialog(this, behaviour_, profile_);
			connect(environmentListDialog_, SIGNAL(itemSelected(QListWidgetItem*)), this, SLOT(itemSelected(QListWidgetItem*)));
		}

		if (environmentListDialog_->isHidden())
			environmentListDialog_->show();
		else
			environmentListDialog_->close();
	}

	void
	EnvironmentDock::itemSelected(QListWidgetItem* item)
	{
		try
		{
			auto hdrComponent = behaviour_->getComponent<FlowerBehaviour>()->getComponent<HDRiComponent>();
			if (hdrComponent)
			{
				auto uuid = item->data(Qt::UserRole).toString();
				auto package = hdrComponent->getPackage(uuid.toStdString());
				if (!package.is_null())
				{
					auto name = package["name"].get<nlohmann::json::string_t>();
					auto hdrPath = package["path"].get<nlohmann::json::string_t>();
					auto previewPath = package["preview"].get<nlohmann::json::string_t>();

					octoon::Image image;
					if (!image.load(hdrPath))
						throw std::runtime_error("Cannot load the image");

					auto previewImage = std::make_shared<QImage>();
					if (!previewImage->load(QString::fromStdString(previewPath)))
						throw std::runtime_error("Cannot generate image for preview");

					this->texture_ = octoon::TextureLoader::load(image, hdrPath);
					this->irradianceTexture_ = octoon::PMREMLoader::load(this->texture_);

					this->setColor(QColor::fromRgbF(1, 1, 1));
					this->setPreviewImage(QString::fromStdString(name), previewImage);
					this->setThumbnailImage(QString::fromStdString(hdrPath), *previewImage);
				}
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox msg(this);
			msg.setWindowTitle(tr("Error"));
			msg.setText(e.what());
			msg.setIcon(QMessageBox::Information);
			msg.setStandardButtons(QMessageBox::Ok);

			msg.exec();
		}
	}

	void
	EnvironmentDock::thumbnailClickEvent()
	{
		try
		{
			QString filepath = QFileDialog::getOpenFileName(this, u8"导入图像", "", tr("HDRi Files (*.hdr)"));
			if (!filepath.isEmpty())
			{
				auto texel = octoon::TextureLoader::load(filepath.toStdString());
				if (texel)
				{
					auto width = texel->getTextureDesc().getWidth();
					auto height = texel->getTextureDesc().getHeight();
					float* data_ = nullptr;

					if (texel->map(0, 0, width, height, 0, (void**)&data_))
					{
						auto size = width * height * 3;
						auto pixels = std::make_unique<std::uint8_t[]>(size);

						for (std::size_t i = 0; i < size; i += 3)
						{
							pixels[i] = std::clamp<float>(std::pow(data_[i], 1 / 2.2) * 255.0f, 0, 255);
							pixels[i + 1] = std::clamp<float>(std::pow(data_[i + 1], 1 / 2.2) * 255.0f, 0, 255);
							pixels[i + 2] = std::clamp<float>(std::pow(data_[i + 2], 1 / 2.2) * 255.0f, 0, 255);
						}

						texel->unmap();

						QImage qimage(pixels.get(), width, height, QImage::Format::Format_RGB888);

						this->texture_ = texel;
						this->irradianceTexture_ = octoon::PMREMLoader::load(this->texture_);

						this->setColor(QColor::fromRgbF(1, 1, 1));
						this->setPreviewImage(QFileInfo(filepath).fileName(), std::make_shared<QImage>(qimage.scaled(previewButton_->size())));
						this->setThumbnailImage(filepath, qimage);
					}
				}
			}
		}
		catch (const std::exception & e)
		{
			QCoreApplication::processEvents();

			QMessageBox msg(this);
			msg.setWindowTitle(tr("Error"));
			msg.setText(e.what());
			msg.setIcon(QMessageBox::Information);
			msg.setStandardButtons(QMessageBox::Ok);

			msg.exec();
		}
	}

	void
	EnvironmentDock::thumbnailToggleEvent(int state)
	{
		auto environmentLight = this->profile_->entitiesModule->enviromentLight;
		if (environmentLight)
		{
			auto envLight = environmentLight->getComponent<octoon::EnvironmentLightComponent>();
			if (envLight)
			{
				envLight->setBackgroundMap(state == Qt::Checked ? this->texture_ : nullptr);
				envLight->setEnvironmentMap(state == Qt::Checked ? this->irradianceTexture_ : nullptr);
			}

			auto material = environmentLight->getComponent<octoon::MeshRendererComponent>()->getMaterial()->downcast<octoon::MeshBasicMaterial>();
			material->setColorMap(state == Qt::Checked && backgroundToggle->isChecked() ? this->texture_ : nullptr);
		}

		this->updatePreviewImage();
	}

	void
	EnvironmentDock::backgroundMapCheckEvent(int state)
	{
		auto environmentLight = this->profile_->entitiesModule->enviromentLight;
		if (environmentLight)
		{
			auto envLight = environmentLight->getComponent<octoon::EnvironmentLightComponent>();
			if (envLight)
				envLight->setShowBackground(state == Qt::Checked);

			environmentLight->getComponent<octoon::MeshRendererComponent>()->setActive(state == Qt::Checked);
		}
	}

	void
	EnvironmentDock::colorClickEvent()
	{
		colorSelector_.setCurrentColor(QColor::fromRgbF(this->profile_->environmentModule->color.x, this->profile_->environmentModule->color.y, this->profile_->environmentModule->color.z));
		colorSelector_.show();
	}

	void 
	EnvironmentDock::colorChangeEvent(const QColor& color)
	{
		this->setColor(color);
		this->updatePreviewImage();
	}

	void
	EnvironmentDock::intensitySliderEvent(int value)
	{
		this->intensitySpinBox->setValue(value / 10.0f);
	}

	void
	EnvironmentDock::intensityEditEvent(double value)
	{
		if (profile_->entitiesModule->enviromentLight)
		{
			auto environmentLight = profile_->entitiesModule->enviromentLight->getComponent<octoon::EnvironmentLightComponent>();
			if (environmentLight)
				environmentLight->setIntensity(value);

			auto meshRenderer = profile_->entitiesModule->enviromentLight->getComponent<octoon::MeshRendererComponent>();
			if (meshRenderer)
				meshRenderer->getMaterial()->set("diffuse", octoon::math::srgb2linear(profile_->environmentModule->color) * value);
		}

		this->profile_->environmentModule->intensity = value;
		this->intensitySlider->setValue(value * 10.0f);
	}

	void
	EnvironmentDock::horizontalRotationSliderEvent(int value)
	{
		this->horizontalRotationSpinBox->setValue(value / 100.0f);
	}
	
	void
	EnvironmentDock::horizontalRotationEditEvent(double value)
	{
		this->profile_->environmentModule->offset = octoon::math::float2(value, this->profile_->environmentModule->offset.y);

		if (profile_->entitiesModule->enviromentLight)
		{
			auto meshRenderer = profile_->entitiesModule->enviromentLight->getComponent<octoon::MeshRendererComponent>();
			if (meshRenderer)
			{
				auto material = meshRenderer->getMaterial();
				if (material->isInstanceOf<octoon::MeshBasicMaterial>())
				{
					auto basicMaterial = material->downcast<octoon::MeshBasicMaterial>();
					basicMaterial->setOffset(this->profile_->environmentModule->offset);
				}
			}

			auto environmentLight = profile_->entitiesModule->enviromentLight->getComponent<octoon::EnvironmentLightComponent>();
			if (environmentLight)
				environmentLight->setOffset(this->profile_->environmentModule->offset);
		}

		this->horizontalRotationSlider->setValue(value * 100.0f);
		this->updatePreviewImage();
	}

	void
	EnvironmentDock::verticalRotationSliderEvent(int value)
	{
		this->verticalRotationSpinBox->setValue(value / 100.0f);
	}
	
	void
	EnvironmentDock::verticalRotationEditEvent(double value)
	{
		this->profile_->environmentModule->offset = octoon::math::float2(this->profile_->environmentModule->offset.x, value);

		if (profile_->entitiesModule->enviromentLight)
		{
			auto meshRenderer = profile_->entitiesModule->enviromentLight->getComponent<octoon::MeshRendererComponent>();
			if (meshRenderer)
			{
				auto material = meshRenderer->getMaterial();
				if (material->isInstanceOf<octoon::MeshBasicMaterial>())
				{
					auto basicMaterial = material->downcast<octoon::MeshBasicMaterial>();
					basicMaterial->setOffset(this->profile_->environmentModule->offset);
				}
			}

			auto environmentLight = profile_->entitiesModule->enviromentLight->getComponent<octoon::EnvironmentLightComponent>();
			if (environmentLight)
				environmentLight->setOffset(this->profile_->environmentModule->offset);
		}

		this->verticalRotationSlider->setValue(value * 100.0f);
		this->updatePreviewImage();
	}

	void
	EnvironmentDock::resetEvent()
	{
		this->intensitySpinBox->setValue(1.0f);
		this->thumbnailToggle->setChecked(false);
		this->horizontalRotationSpinBox->setValue(0.0f);
		this->verticalRotationSpinBox->setValue(0.0f);
		this->setColor(QColor::fromRgb(229, 229, 235));
		this->updatePreviewImage();
	}

	void
	EnvironmentDock::showEvent(QShowEvent* event)
	{
		this->intensitySpinBox->setValue(profile_->environmentModule->intensity);
		this->horizontalRotationSpinBox->setValue(profile_->environmentModule->offset.x);
		this->verticalRotationSpinBox->setValue(profile_->environmentModule->offset.y);
		this->setColor(QColor::fromRgbF(profile_->environmentModule->color.x, profile_->environmentModule->color.y, profile_->environmentModule->color.z));

		if (this->profile_->entitiesModule->enviromentLight)
		{
			auto envLight = this->profile_->entitiesModule->enviromentLight->getComponent<octoon::EnvironmentLightComponent>();
			if (envLight)
			{
				this->texture_ = envLight->getBackgroundMap();
				this->irradianceTexture_ = envLight->getEnvironmentMap();

				thumbnailToggle->setChecked(envLight->getBackgroundMap() ? true : false);
				backgroundToggle->setChecked(envLight->getShowBackground());
			}
		}

		this->updatePreviewImage();
	}

	void
	EnvironmentDock::closeEvent(QCloseEvent* event)
	{
		if (profile_->playerModule->isPlaying)
			event->ignore();
		else
			event->accept();
	}
}