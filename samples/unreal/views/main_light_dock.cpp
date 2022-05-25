#include "main_light_dock.h"
#include <qscrollbar.h>
#include <octoon/transform_component.h>
#include <octoon/directional_light_component.h>

namespace unreal
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

	MainLightDock::MainLightDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<unreal::UnrealProfile>& profile)
		: profile_(profile)
	{
		this->setWindowTitle(tr("Main Light"));
		this->setObjectName("MainLightDock");

		auto color = profile->mainLightModule->color.getValue();

		colorDialog_ = new ColorDialog();
		colorDialog_->setCurrentColor(QColor::fromRgbF(color.x, color.y, color.z));

		resetButton_ = new QToolButton();
		resetButton_->setText(tr("Reset"));
		resetButton_->setContentsMargins(0, 0, 10, 0);

		labelIntensity_ = new QLabel();
		labelIntensity_->setText(tr("Intensity"));

		editIntensity_ = new DoubleSpinBox();
		editIntensity_->setFixedWidth(50);
		editIntensity_->setMaximum(10.0f);
		editIntensity_->setSingleStep(0.1f);
		editIntensity_->setAlignment(Qt::AlignRight);
		editIntensity_->setValue(profile->mainLightModule->intensity);
		editIntensity_->setDecimals(1);
		editIntensity_->setSuffix(u8"cd");

		sliderIntensity_ = new QSlider();
		sliderIntensity_->setObjectName("Intensity");
		sliderIntensity_->setOrientation(Qt::Horizontal);
		sliderIntensity_->setMinimum(0);
		sliderIntensity_->setMaximum(100);
		sliderIntensity_->setValue(profile->mainLightModule->intensity * 10.0f);
		sliderIntensity_->setFixedWidth(250);

		layoutIntensity_ = new QHBoxLayout();
		layoutIntensity_->addWidget(labelIntensity_, 0, Qt::AlignLeft);
		layoutIntensity_->addWidget(editIntensity_, 0, Qt::AlignRight);
		layoutIntensity_->setContentsMargins(40, 5, 35, 0);

		labelSize_ = new QLabel();
		labelSize_->setText(tr("Size"));

		editSize_ = new DoubleSpinBox();
		editSize_->setFixedWidth(50);
		editSize_->setMaximum(1.0f);
		editSize_->setSingleStep(0.05f);
		editSize_->setAlignment(Qt::AlignRight);
		editSize_->setValue(profile->mainLightModule->size);
		editSize_->setDecimals(1);

		sliderSize_ = new QSlider();
		sliderSize_->setObjectName("Size");
		sliderSize_->setOrientation(Qt::Horizontal);
		sliderSize_->setMinimum(0);
		sliderSize_->setMaximum(100);
		sliderSize_->setValue(profile->mainLightModule->size * 100.0f);
		sliderSize_->setFixedWidth(250);

		layoutSize_ = new QHBoxLayout();
		layoutSize_->addWidget(labelSize_, 0, Qt::AlignLeft);
		layoutSize_->addWidget(editSize_, 0, Qt::AlignRight);
		layoutSize_->setContentsMargins(40, 5, 35, 0);

		labelRotationX_ = new QLabel();
		labelRotationX_->setText(tr("Rotation X"));

		editRotationX_ = new DoubleSpinBox();
		editRotationX_->setFixedWidth(50);
		editRotationX_->setMaximum(360.0f);
		editRotationX_->setSingleStep(1.0f);
		editRotationX_->setAlignment(Qt::AlignRight);
		editRotationX_->setValue(profile->mainLightModule->rotation.getValue().x);
		editRotationX_->setDecimals(1);
		editRotationX_->setSuffix(u8"°");

		sliderRotationX_ = new QSlider();
		sliderRotationX_->setObjectName("RotationX");
		sliderRotationX_->setOrientation(Qt::Horizontal);
		sliderRotationX_->setMinimum(0);
		sliderRotationX_->setMaximum(360);
		sliderRotationX_->setValue(profile->mainLightModule->rotation.getValue().x);
		sliderRotationX_->setFixedWidth(250);

		layoutRotationX_ = new QHBoxLayout();
		layoutRotationX_->addWidget(labelRotationX_, 0, Qt::AlignLeft);
		layoutRotationX_->addWidget(editRotationX_, 0, Qt::AlignRight);
		layoutRotationX_->setContentsMargins(40, 5, 35, 0);

		labelRotationY_ = new QLabel();
		labelRotationY_->setText(tr("Rotation Y"));

		editRotationY_ = new DoubleSpinBox();
		editRotationY_->setFixedWidth(50);
		editRotationY_->setMaximum(360.0f);
		editRotationY_->setSingleStep(1.0f);
		editRotationY_->setAlignment(Qt::AlignRight);
		editRotationY_->setValue(profile->mainLightModule->rotation.getValue().y);
		editRotationY_->setDecimals(1);
		editRotationY_->setSuffix(u8"°");

		sliderRotationY_ = new QSlider();
		sliderRotationY_->setObjectName("RotationY");
		sliderRotationY_->setOrientation(Qt::Horizontal);
		sliderRotationY_->setMinimum(0);
		sliderRotationY_->setMaximum(360);
		sliderRotationY_->setValue(profile->mainLightModule->rotation.getValue().y);
		sliderRotationY_->setFixedWidth(250);

		layoutRotationY_ = new QHBoxLayout();
		layoutRotationY_->addWidget(labelRotationY_, 0, Qt::AlignLeft);
		layoutRotationY_->addWidget(editRotationY_, 0, Qt::AlignRight);
		layoutRotationY_->setContentsMargins(40, 5, 35, 0);

		labelRotationZ_ = new QLabel();
		labelRotationZ_->setText(tr("Rotation Z"));

		editRotationZ_ = new DoubleSpinBox();
		editRotationZ_->setFixedWidth(50);
		editRotationZ_->setMaximum(360.0f);
		editRotationZ_->setSingleStep(1.0f);
		editRotationZ_->setAlignment(Qt::AlignRight);
		editRotationZ_->setValue(profile->mainLightModule->rotation.getValue().z);
		editRotationZ_->setDecimals(1);
		editRotationZ_->setSuffix(u8"°");

		sliderRotationZ_ = new QSlider();
		sliderRotationZ_->setObjectName("RotationZ");
		sliderRotationZ_->setOrientation(Qt::Horizontal);
		sliderRotationZ_->setMinimum(0);
		sliderRotationZ_->setMaximum(360);
		sliderRotationZ_->setValue(profile->mainLightModule->rotation.getValue().z);
		sliderRotationZ_->setFixedWidth(250);

		layoutRotationZ_ = new QHBoxLayout();
		layoutRotationZ_->addWidget(labelRotationZ_, 0, Qt::AlignLeft);
		layoutRotationZ_->addWidget(editRotationZ_, 0, Qt::AlignRight);
		layoutRotationZ_->setContentsMargins(40, 5, 35, 0);

		scrollLayout_ = new QVBoxLayout();
		scrollLayout_->addWidget(colorDialog_, 0, Qt::AlignHCenter | Qt::AlignTop);
		scrollLayout_->addLayout(layoutIntensity_, 0);
		scrollLayout_->addWidget(sliderIntensity_, 0, Qt::AlignHCenter);
		scrollLayout_->addLayout(layoutSize_, 0);
		scrollLayout_->addWidget(sliderSize_, 0, Qt::AlignHCenter);
		scrollLayout_->addLayout(layoutRotationX_, 0);
		scrollLayout_->addWidget(sliderRotationX_, 0, Qt::AlignHCenter);
		scrollLayout_->addLayout(layoutRotationY_, 0);
		scrollLayout_->addWidget(sliderRotationY_, 0, Qt::AlignHCenter);
		scrollLayout_->addLayout(layoutRotationZ_, 0);
		scrollLayout_->addWidget(sliderRotationZ_, 0, Qt::AlignHCenter);
		scrollLayout_->addStretch();

		scrollWidget_ = new QWidget();
		scrollWidget_->setLayout(scrollLayout_);

		scrollArea_ = new QScrollArea();
		scrollArea_->setWidget(scrollWidget_);
		scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea_->setWidgetResizable(true);

		mainLayout_ = new QVBoxLayout();
		mainLayout_->addWidget(scrollArea_);
		mainLayout_->addStretch();
		mainLayout_->addWidget(resetButton_, 0, Qt::AlignBottom | Qt::AlignRight);
		mainLayout_->setContentsMargins(0, 0, 0, 10);

		mainWidget_ = new QWidget();
		mainWidget_->setLayout(mainLayout_);

		this->setWidget(mainWidget_);

		connect(resetButton_, SIGNAL(clicked()), this, SLOT(resetEvent()));
		connect(editIntensity_, SIGNAL(valueChanged(double)), this, SLOT(intensityEditEvent(double)));
		connect(sliderIntensity_, SIGNAL(valueChanged(int)), this, SLOT(intensitySliderEvent(int)));
		connect(editSize_, SIGNAL(valueChanged(double)), this, SLOT(sizeEditEvent(double)));
		connect(sliderSize_, SIGNAL(valueChanged(int)), this, SLOT(sizeSliderEvent(int)));
		connect(editRotationX_, SIGNAL(valueChanged(double)), this, SLOT(editRotationXEvent(double)));
		connect(sliderRotationX_, SIGNAL(valueChanged(int)), this, SLOT(sliderRotationXEvent(int)));
		connect(editRotationY_, SIGNAL(valueChanged(double)), this, SLOT(editRotationYEvent(double)));
		connect(sliderRotationY_, SIGNAL(valueChanged(int)), this, SLOT(sliderRotationYEvent(int)));
		connect(editRotationZ_, SIGNAL(valueChanged(double)), this, SLOT(editRotationZEvent(double)));
		connect(sliderRotationZ_, SIGNAL(valueChanged(int)), this, SLOT(sliderRotationZEvent(int)));
		connect(colorDialog_, SIGNAL(currentColorChanged(QColor)), this, SLOT(currentColorChanged(QColor)));
	}

	MainLightDock::~MainLightDock()
	{
	}

	void
	MainLightDock::resizeEvent(QResizeEvent* event)
	{
	}

	void
	MainLightDock::paintEvent(QPaintEvent* e) noexcept
	{
		int left, top, bottom, right;
		mainLayout_->getContentsMargins(&left, &top, &right, &bottom);
		scrollArea_->resize(scrollArea_->size().width(), mainWidget_->size().height() - resetButton_->height() - (top + bottom) * 2);

		QDockWidget::paintEvent(e);
	}

	void 
	MainLightDock::currentColorChanged(QColor color)
	{
		if (color.isValid())
			profile_->mainLightModule->color = octoon::math::float3(color.redF(), color.greenF(), color.blueF());
	}

	void
	MainLightDock::resetEvent()
	{
		profile_->mainLightModule->size = 0.1f;
		profile_->mainLightModule->intensity = 1.0f;
		profile_->mainLightModule->color = octoon::math::float3::One;
		profile_->mainLightModule->rotation = octoon::math::float3::Zero;
	}

	void
	MainLightDock::intensitySliderEvent(int value)
	{
		editIntensity_->setValue(value / 10.0f);
	}

	void
	MainLightDock::intensityEditEvent(double value)
	{
		profile_->mainLightModule->intensity = value;
		sliderIntensity_->setValue(value * 10.0f);
	}

	void
	MainLightDock::sizeSliderEvent(int value)
	{
		editSize_->setValue(value / 100.0f);
	}

	void
	MainLightDock::sizeEditEvent(double value)
	{
		profile_->mainLightModule->size = value;
		sliderSize_->setValue(value * 100.0f);
	}

	void
	MainLightDock::sliderRotationXEvent(int value)
	{
		editRotationX_->setValue(value);
	}

	void
	MainLightDock::editRotationXEvent(double value)
	{
		profile_->mainLightModule->rotation = octoon::math::float3(value - 180.0f, profile_->mainLightModule->rotation.getValue().y, profile_->mainLightModule->rotation.getValue().z);
		sliderRotationX_->setValue(value);
	}

	void
	MainLightDock::sliderRotationYEvent(int value)
	{
		editRotationY_->setValue(value);
	}

	void
	MainLightDock::editRotationYEvent(double value)
	{
		profile_->mainLightModule->rotation = octoon::math::float3(profile_->mainLightModule->rotation.getValue().x, value - 180.0f, profile_->mainLightModule->rotation.getValue().z);
		sliderRotationY_->setValue(value);
	}

	void
	MainLightDock::sliderRotationZEvent(int value)
	{
		editRotationZ_->setValue(value);
	}

	void
	MainLightDock::editRotationZEvent(double value)
	{
		profile_->mainLightModule->rotation = octoon::math::float3(profile_->mainLightModule->rotation.getValue().x, profile_->mainLightModule->rotation.getValue().y, value);
		sliderRotationZ_->setValue(value);
	}

	void
	MainLightDock::showEvent(QShowEvent* event)
	{
		auto color = profile_->mainLightModule->color.getValue();
		auto rotation = profile_->mainLightModule->rotation.getValue();

		editSize_->setValue(profile_->mainLightModule->size);
		editIntensity_->setValue(profile_->mainLightModule->intensity);
		editRotationX_->setValue(rotation.x + 180.0f);
		editRotationY_->setValue(rotation.y + 180.0f);
		editRotationZ_->setValue(rotation.z);
		colorDialog_->setCurrentColor(QColor::fromRgbF(color.x, color.y, color.z));
	}

	void
	MainLightDock::closeEvent(QCloseEvent* event)
	{
		if (profile_->playerModule->isPlaying)
			event->ignore();
		else
			event->accept();
	}
}