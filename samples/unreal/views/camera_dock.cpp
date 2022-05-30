#include "camera_dock.h"
#include <octoon/io/fstream.h>
#include <octoon/vmd_loader.h>
#include <qapplication.h>
#include <qevent.h>
#include <qfiledialog.h>
#include <qmessagebox.h>

namespace unreal
{
	class SpinBox final : public QSpinBox
	{
	public:
		void
		focusInEvent(QFocusEvent* event) override
		{
			this->grabKeyboard();
			QSpinBox::focusInEvent(event);
		}

		void
		focusOutEvent(QFocusEvent* event) override
		{
			this->releaseKeyboard();
			QSpinBox::focusOutEvent(event);
		}
	};

	class DoubleSpinBox final : public QDoubleSpinBox
	{
	public:
		void
		focusInEvent(QFocusEvent* event) override
		{
			this->grabKeyboard();
			QDoubleSpinBox::focusInEvent(event);
		}

		void
		focusOutEvent(QFocusEvent* event) override
		{
			this->releaseKeyboard();
			QDoubleSpinBox::focusOutEvent(event);
		}
	};

	CameraDock::CameraDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile) noexcept
		: behaviour_(behaviour)
		, profile_(profile)
	{
		this->setObjectName("CameraDock");
		this->setWindowTitle(tr("Camera"));

		fovLabel_ = new QLabel();
		fovLabel_->setText(tr("fov:"));
		fovLabel_->setStyleSheet("color: rgb(200,200,200);");

		fovSpinbox_ = new DoubleSpinBox();
		fovSpinbox_->setMinimum(1.0f);
		fovSpinbox_->setMaximum(120.0f);
		fovSpinbox_->setValue(60.0f);
		fovSpinbox_->setSingleStep(1.0f);
		fovSpinbox_->setAlignment(Qt::AlignRight);
		fovSpinbox_->setFixedWidth(100);
		fovSpinbox_->setSuffix(u8"бу");
		fovSpinbox_->setDecimals(1);
		fovSpinbox_->installEventFilter(this);

		dofInfoLabel_ = new QLabel();
		dofInfoLabel_->setText(tr("* The following parameters take effect on rendering"));
		dofInfoLabel_->setStyleSheet("color: rgb(100,100,100);");

		dofLabel_ = new QLabel();
		dofLabel_->setText(tr("Depth Of Filed:"));

		dofButton_ = new QCheckBox();
		dofButton_->setCheckState(Qt::CheckState::Unchecked);
		dofButton_->installEventFilter(this);

		auto dofLayout_ = new QHBoxLayout();
		dofLayout_->addWidget(dofLabel_, 0, Qt::AlignLeft);
		dofLayout_->addWidget(dofButton_, 0, Qt::AlignLeft);
		dofLayout_->setSpacing(0);
		dofLayout_->setContentsMargins(0, 0, 0, 0);

		apertureLabel_ = new QLabel();
		apertureLabel_->setText(tr("Aperture:"));
		apertureLabel_->setStyleSheet("color: rgb(200,200,200);");

		apertureSpinbox_ = new DoubleSpinBox();
		apertureSpinbox_->setMinimum(1.0f);
		apertureSpinbox_->setMaximum(64.0f);
		apertureSpinbox_->setValue(64.0f);
		apertureSpinbox_->setSingleStep(0.1f);
		apertureSpinbox_->setAlignment(Qt::AlignRight);
		apertureSpinbox_->setFixedWidth(100);
		apertureSpinbox_->setPrefix(tr("f/"));
		apertureSpinbox_->setDecimals(1);
		apertureSpinbox_->installEventFilter(this);

		focalLengthLabel_ = new QLabel();
		focalLengthLabel_->setText(tr("Focal Length (35mmfilm):"));
		focalLengthLabel_->setStyleSheet("color: rgb(200,200,200);");

		focusDistanceLabel_ = new QLabel();
		focusDistanceLabel_->setText(tr("Focus Distance:"));
		focusDistanceLabel_->setStyleSheet("color: rgb(200,200,200);");

		focalLengthSpinbox_ = new DoubleSpinBox();
		focalLengthSpinbox_->setMinimum(1.0f);
		focalLengthSpinbox_->setMaximum(1200.0f);
		focalLengthSpinbox_->setValue(31.18f);
		focalLengthSpinbox_->setSingleStep(1.0f);
		focalLengthSpinbox_->setAlignment(Qt::AlignRight);
		focalLengthSpinbox_->setSuffix(tr("mm"));
		focalLengthSpinbox_->setFixedWidth(100);
		focalLengthSpinbox_->installEventFilter(this);

		focusDistanceSpinbox_ = new DoubleSpinBox();
		focusDistanceSpinbox_->setMinimum(0);
		focusDistanceSpinbox_->setMaximum(std::numeric_limits<float>::infinity());
		focusDistanceSpinbox_->setValue(0);
		focusDistanceSpinbox_->setSingleStep(0.1f);
		focusDistanceSpinbox_->setAlignment(Qt::AlignRight);
		focusDistanceSpinbox_->setSuffix(tr("m"));
		focusDistanceSpinbox_->setFixedWidth(100);
		focusDistanceSpinbox_->installEventFilter(this);

		focusDistanceName_ = new QLabel();
		focusDistanceName_->setText(tr("Target: Empty"));
		focusDistanceName_->setStyleSheet("color: rgb(200,200,200);");

		focusTargetButton_ = new DraggableButton();
		focusTargetButton_->setIcon(QIcon(":res/icons/target.png"));
		focusTargetButton_->setIconSize(QSize(48, 48));
		focusTargetButton_->setToolTip(tr("Drag and drop this icon onto the target model to enable autofocus"));
		focusTargetButton_->installEventFilter(this);

		loadButton_ = new QToolButton();
		loadButton_->setObjectName("Load");
		loadButton_->setText(tr("Load Animation"));
		loadButton_->setContentsMargins(0, 0, 0, 0);
		loadButton_->installEventFilter(this);

		unloadButton_ = new QToolButton();
		unloadButton_->setObjectName("Unload");
		unloadButton_->setText(tr("Uninstall"));
		unloadButton_->setContentsMargins(0, 0, 0, 0);
		unloadButton_->installEventFilter(this);

		auto fovLayout = new QHBoxLayout;
		fovLayout->addWidget(fovLabel_);
		fovLayout->addWidget(fovSpinbox_);

		auto apertureLayout = new QHBoxLayout;
		apertureLayout->addWidget(apertureLabel_);
		apertureLayout->addWidget(apertureSpinbox_);

		auto focusLengthLayout = new QHBoxLayout;
		focusLengthLayout->addWidget(focalLengthLabel_);
		focusLengthLayout->addWidget(focalLengthSpinbox_);

		auto focusDistanceLayout = new QHBoxLayout;
		focusDistanceLayout->addWidget(focusDistanceLabel_);
		focusDistanceLayout->addWidget(focusDistanceSpinbox_);

		auto focusTargetLayout = new QHBoxLayout;
		focusTargetLayout->addWidget(focusTargetButton_, 0, Qt::AlignLeft);
		focusTargetLayout->addWidget(focusDistanceName_);
		focusTargetLayout->addStretch();

		auto animtionLayout = new QHBoxLayout;
		animtionLayout->addStretch();
		animtionLayout->addWidget(loadButton_, 0, Qt::AlignRight);
		animtionLayout->addWidget(unloadButton_, 0, Qt::AlignLeft);
		animtionLayout->addStretch();
		animtionLayout->setContentsMargins(0, 0, 0, 0);

		mainLayout_ = new QVBoxLayout;
		mainLayout_->addLayout(fovLayout);
		mainLayout_->addWidget(dofInfoLabel_, 0, Qt::AlignLeft);
		mainLayout_->addLayout(dofLayout_);
		mainLayout_->addLayout(apertureLayout);
		mainLayout_->addLayout(focusLengthLayout);
		mainLayout_->addLayout(focusDistanceLayout);
		mainLayout_->addLayout(focusTargetLayout);
		mainLayout_->addLayout(animtionLayout);
		mainLayout_->addStretch();
		mainLayout_->setContentsMargins(30, 5, 20, 0);

		mainWidget_ = new QWidget;
		mainWidget_->setLayout(mainLayout_);

		this->setWidget(mainWidget_);

		profile_->entitiesModule->camera += [this](const octoon::GameObjectPtr& camera) {
			if (camera)
			{
				if (camera->getComponent<octoon::AnimatorComponent>())
					unloadButton_->setEnabled(true);
				else
					unloadButton_->setEnabled(false);
			}
		};

		profile_->cameraModule->fov += [this](float fov) {
			this->fovSpinbox_->blockSignals(true);
			this->fovSpinbox_->setValue(fov);
			this->fovSpinbox_->blockSignals(false);
		};

		profile_->cameraModule->useDepthOfFiled += [this](bool value) {
			this->dofButton_->blockSignals(true);
			this->dofButton_->setChecked(value);
			this->dofButton_->blockSignals(false);
		};

		profile_->cameraModule->aperture += [this](float fov) {
			this->apertureSpinbox_->blockSignals(true);
			this->apertureSpinbox_->setValue(fov);
			this->apertureSpinbox_->blockSignals(false);
		};

		profile_->cameraModule->focalLength += [this](float fov) {
			this->focalLengthSpinbox_->blockSignals(true);
			this->focalLengthSpinbox_->setValue(fov);
			this->focalLengthSpinbox_->blockSignals(false);
		};

		connect(focusTargetButton_, SIGNAL(mouseMoveSignal()), this, SLOT(onUpdateTarget()));
		connect(fovSpinbox_, SIGNAL(valueChanged(double)), this, SLOT(onFovChanged(double)));
		connect(apertureSpinbox_, SIGNAL(valueChanged(double)), this, SLOT(onApertureChanged(double)));
		connect(dofButton_, SIGNAL(stateChanged(int)), this, SLOT(dofEvent(int)));
		connect(focalLengthSpinbox_, SIGNAL(valueChanged(double)), this, SLOT(onFocalLengthChanged(double)));
		connect(focusDistanceSpinbox_, SIGNAL(valueChanged(double)), this, SLOT(onFocusDistanceChanged(double)));
		connect(loadButton_, SIGNAL(clicked()), this, SLOT(onLoadAnimation()));
		connect(unloadButton_, SIGNAL(clicked()), this, SLOT(onUnloadAnimation()));
	}

	CameraDock::~CameraDock() noexcept
	{
	}

	void
	CameraDock::onUpdateTarget()
	{
		if (!profile_->playerModule->isPlaying)
		{
			auto hit = profile_->selectorModule->selectedItemHover_;
			if (hit)
			{
				profile_->playerModule->dofTarget = hit;

				auto object = hit->object.lock();
				auto renderer = object->getComponent<octoon::MeshRendererComponent>();
				auto material = renderer->getMaterial(profile_->playerModule->dofTarget->mesh);

				focusDistanceName_->setText(tr("Target: %1").arg(material->getName().c_str()));
				focusDistanceSpinbox_->setValue(0);
				focusDistanceSpinbox_->setSpecialValueText(tr("Auto-measuring"));
			}
			else
			{
				profile_->playerModule->dofTarget = hit;

				focusDistanceName_->setText(tr("Target: Empty"));
				focusDistanceSpinbox_->setValue(10);
				focusDistanceSpinbox_->setSpecialValueText(QString());
			}
		}
	}

	void
	CameraDock::onFovChanged(double value)
	{
		profile_->cameraModule->fov = value;
	}

	void
	CameraDock::onFocalLengthChanged(double value)
	{
		profile_->cameraModule->focalLength = value;
	}

	void
	CameraDock::onApertureChanged(double value)
	{
		profile_->cameraModule->aperture = value;
	}

	void
	CameraDock::dofEvent(int checked)
	{
		if (checked == Qt::CheckState::Checked)
			profile_->cameraModule->useDepthOfFiled = true;
		else
			profile_->cameraModule->useDepthOfFiled = false;
	}

	void
	CameraDock::onFocusDistanceChanged(double value)
	{
		auto& camera = profile_->entitiesModule->camera.getValue();
		if (camera)
		{
			if (!focusDistanceSpinbox_->specialValueText().isEmpty())
			{
				profile_->playerModule->dofTarget = std::nullopt;

				focusDistanceName_->setText(tr("Target: Empty"));
				focusDistanceSpinbox_->setSpecialValueText(QString());
				focusDistanceSpinbox_->setValue(camera->getComponent<octoon::FilmCameraComponent>()->getFocalDistance());
			}
			else
			{
				profile_->cameraModule->focusDistance = value;
			}
		}
	}

	void
	CameraDock::onLoadAnimation()
	{
		try
		{
			QString filepath = QFileDialog::getOpenFileName(this, tr("Load Animation"), "", tr("VMD Files (*.vmd)"));
			if (!filepath.isEmpty())
			{
				this->profile_->cameraModule->animation = filepath.toUtf8().toStdString();
				unloadButton_->setEnabled(true);
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(this, tr("Error"), e.what());
		}
	}

	void
	CameraDock::onUnloadAnimation()
	{
		auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
		if (behaviour)
		{
			behaviour->getComponent<CameraComponent>()->removeAnimation();
			behaviour->getComponent<PlayerComponent>()->updateTimeLength();

			unloadButton_->setEnabled(false);
		}
	}

	void
	CameraDock::showEvent(QShowEvent* event)
	{
		auto mainCamera = profile_->entitiesModule->camera.getValue();

		fovSpinbox_->setValue(profile_->cameraModule->fov);
		focalLengthSpinbox_->setValue(profile_->cameraModule->focalLength);
		apertureSpinbox_->setValue(profile_->cameraModule->aperture);
		dofButton_->setChecked(profile_->cameraModule->useDepthOfFiled);
		unloadButton_->setEnabled(mainCamera->getComponent<octoon::AnimatorComponent>() ? true : false);

		if (profile_->playerModule->dofTarget)
		{
			auto object = profile_->playerModule->dofTarget->object.lock();
			auto renderer = object->getComponent<octoon::MeshRendererComponent>();
			auto material = renderer->getMaterial(profile_->playerModule->dofTarget->mesh);

			focusDistanceSpinbox_->setValue(0);
			focusDistanceSpinbox_->setSpecialValueText(tr("Auto-measuring"));
			focusDistanceName_->setText(tr("Target: %1").arg(material->getName().c_str()));
		}
		else
		{
			focusDistanceName_->setText(tr("Target: Empty"));
			focusDistanceSpinbox_->setValue(mainCamera->getComponent<octoon::FilmCameraComponent>()->getFocalDistance());
		}
	}

	void
	CameraDock::closeEvent(QCloseEvent* event)
	{
		if (profile_->playerModule->isPlaying)
			event->ignore();
		else
			event->accept();
	}

	bool
	CameraDock::eventFilter(QObject* watched, QEvent* event)
	{
		if (event->type() != QEvent::Paint)
		{
			if (profile_->playerModule->isPlaying)
			{
				return true;
			}
		}

		return QWidget::eventFilter(watched, event);
	}

	void
	CameraDock::retranslate()
	{
		this->setWindowTitle(tr("Camera"));
		fovLabel_->setText(tr("fov:"));
		dofInfoLabel_->setText(tr("* The following parameters take effect on rendering"));
		dofLabel_->setText(tr("Depth Of Filed:"));
		apertureLabel_->setText(tr("Aperture:"));
		focalLengthLabel_->setText(tr("Focal Length (35mmfilm):"));
		focusDistanceLabel_->setText(tr("Focus Distance:"));
		focusTargetButton_->setToolTip(tr("Drag and drop this icon onto the target model to enable autofocus"));
		focusDistanceName_->setText(tr("Target: Empty"));
		loadButton_->setText(tr("Load Animation"));
		unloadButton_->setText(tr("Uninstall"));
	}
}