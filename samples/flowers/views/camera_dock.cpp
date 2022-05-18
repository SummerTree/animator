#include "camera_dock.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <qdrag.h>
#include <qmimedata.h>
#include <qapplication.h>

namespace flower
{
	class SpinBox final : public QSpinBox
	{
	public:
		void focusInEvent(QFocusEvent* event) override
		{
			this->grabKeyboard();
			QSpinBox::focusInEvent(event);
		}

		void focusOutEvent(QFocusEvent* event) override
		{
			this->releaseKeyboard();
			QSpinBox::focusOutEvent(event);
		}
	};

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

	FocalTargetWindow::FocalTargetWindow() noexcept
	{
	}

	FocalTargetWindow::~FocalTargetWindow() noexcept
	{
	}

	void
	FocalTargetWindow::mouseMoveEvent(QMouseEvent *event)
	{
		if (event->buttons() & Qt::LeftButton)
		{
			QPoint length = event->pos() - startPos;
			if (length.manhattanLength() > QApplication::startDragDistance())
			{
				auto mimeData = new QMimeData;
				mimeData->setData("object/dof", "Automatic");

				auto drag = new QDrag(this);
				drag->setMimeData(mimeData);
				drag->setPixmap(QPixmap(":res/icons/target.png"));
				drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2));
				drag->exec(Qt::MoveAction);

				emit mouseMoveSignal();
			}
		}

		QToolButton::mouseMoveEvent(event);
	}

	void
	FocalTargetWindow::mousePressEvent(QMouseEvent *event)
	{
		if (event->button() == Qt::LeftButton)
			startPos = event->pos();

		QToolButton::mousePressEvent(event);
	}

	CameraDock::CameraDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<FlowerProfile>& profile) noexcept
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
		fovSpinbox_->setValue(32.0f);
		fovSpinbox_->setSingleStep(1.0f);
		fovSpinbox_->setAlignment(Qt::AlignRight);
		fovSpinbox_->setFixedWidth(100);
		fovSpinbox_->setSuffix(u8"°");
		fovSpinbox_->setDecimals(1);

		dofInfoLabel_ = new QLabel();
		dofInfoLabel_->setText(u8"* 以下参数开启渲染时生效");
		dofInfoLabel_->setStyleSheet("color: rgb(100,100,100);");

		apertureLabel_ = new QLabel();
		apertureLabel_->setText(tr("aperture:"));
		apertureLabel_->setStyleSheet("color: rgb(200,200,200);");

		apertureSpinbox_ = new DoubleSpinBox();
		apertureSpinbox_->setMinimum(1.0f);
		apertureSpinbox_->setMaximum(64.0f);
		apertureSpinbox_->setValue(64.0f);
		apertureSpinbox_->setSingleStep(0.1f);
		apertureSpinbox_->setAlignment(Qt::AlignRight);
		apertureSpinbox_->setFixedWidth(100);
		apertureSpinbox_->setPrefix(u8"f/");
		apertureSpinbox_->setDecimals(1);

		focusDistanceLabel_ = new QLabel();
		focusDistanceLabel_->setText(tr("Focus Distance:"));
		focusDistanceLabel_->setStyleSheet("color: rgb(200,200,200);");

		focusDistanceSpinbox_ = new DoubleSpinBox();
		focusDistanceSpinbox_->setMinimum(0);
		focusDistanceSpinbox_->setMaximum(std::numeric_limits<float>::infinity());
		focusDistanceSpinbox_->setValue(0);
		focusDistanceSpinbox_->setSingleStep(0.1f);
		focusDistanceSpinbox_->setAlignment(Qt::AlignRight);
		focusDistanceSpinbox_->setSuffix(u8"m");
		focusDistanceSpinbox_->setFixedWidth(100);

		focusDistanceName_ = new QLabel();
		focusDistanceName_->setText(tr("Target: Empty"));
		focusDistanceName_->setStyleSheet("color: rgb(200,200,200);");

		focusTargetButton_ = new FocalTargetWindow();
		focusTargetButton_->setIcon(QIcon(":res/icons/target.png"));
		focusTargetButton_->setIconSize(QSize(48, 48));
		focusTargetButton_->setToolTip(u8"拖拽该图标到目标模型可开启自动追焦");

		auto fovLayout = new QHBoxLayout;
		fovLayout->addWidget(fovLabel_);
		fovLayout->addWidget(fovSpinbox_);

		auto apertureLayout = new QHBoxLayout;
		apertureLayout->addWidget(apertureLabel_);
		apertureLayout->addWidget(apertureSpinbox_);

		auto focusDistanceLayout = new QHBoxLayout;
		focusDistanceLayout->addWidget(focusDistanceLabel_);
		focusDistanceLayout->addWidget(focusDistanceSpinbox_);

		auto focusTargetLayout = new QHBoxLayout;
		focusTargetLayout->addWidget(focusTargetButton_, 0, Qt::AlignLeft);
		focusTargetLayout->addWidget(focusDistanceName_);
		focusTargetLayout->addStretch();

		mainLayout_ = new QVBoxLayout;
		mainLayout_->addLayout(fovLayout);
		mainLayout_->addWidget(dofInfoLabel_, 0, Qt::AlignLeft);
		mainLayout_->addLayout(apertureLayout);
		mainLayout_->addLayout(focusDistanceLayout);
		mainLayout_->addLayout(focusTargetLayout);
		mainLayout_->addStretch();
		mainLayout_->setContentsMargins(30, 5, 20, 0);

		mainWidget_ = new QWidget;
		mainWidget_->setLayout(mainLayout_);

		this->setWidget(mainWidget_);
		
		connect(focusTargetButton_, SIGNAL(mouseMoveSignal()), this, SLOT(onUpdateTarget()));
		connect(fovSpinbox_, SIGNAL(valueChanged(double)), this, SLOT(onFovChanged(double)));
		connect(apertureSpinbox_, SIGNAL(valueChanged(double)), this, SLOT(onApertureChanged(double)));
		connect(focusDistanceSpinbox_, SIGNAL(valueChanged(double)), this, SLOT(onFocusDistanceChanged(double)));
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
		if (!profile_->playerModule->isPlaying && profile_->entitiesModule->camera)
			profile_->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->setFov(value);
		else
			this->updateDefaultSetting();
	}

	void
	CameraDock::onApertureChanged(double value)
	{
		if (!profile_->playerModule->isPlaying && profile_->entitiesModule->camera)
			profile_->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->setAperture(value == 64.f ? 0.0f : 1.0f / value);
		else
			this->updateDefaultSetting();
	}

	void
	CameraDock::onFocusDistanceChanged(double value)
	{
		if (!profile_->playerModule->isPlaying && profile_->entitiesModule->camera)
		{
			if (!focusDistanceSpinbox_->specialValueText().isEmpty())
			{
				profile_->playerModule->dofTarget = std::nullopt;

				focusDistanceName_->setText(tr("Target: Empty"));
				focusDistanceSpinbox_->setSpecialValueText(QString());
				focusDistanceSpinbox_->setValue(profile_->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->getFocalDistance());
			}
			else
			{
				profile_->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->setFocusDistance(value);
			}
		}
		else
		{
			this->updateDefaultSetting();
		}		
	}

	void
	CameraDock::updateDefaultSetting()
	{
		if (profile_->entitiesModule->camera)
		{
			auto camera = profile_->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>();

			float fov = camera->getFov();
			float aperture = camera->getAperture();

			fovSpinbox_->setValue(fov);
			apertureSpinbox_->setValue(aperture == 0.0f ? 64.0f : 1.0f / aperture);

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
				focusDistanceSpinbox_->setValue(profile_->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->getFocalDistance());
			}
		}
	}

	void
	CameraDock::showEvent(QShowEvent* event)
	{
		this->updateDefaultSetting();
	}

	void
	CameraDock::closeEvent(QCloseEvent* event)
	{
		if (profile_->playerModule->isPlaying)
			event->ignore();
		else
			event->accept();
	}
}