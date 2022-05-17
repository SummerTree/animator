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

		dofInfoLabel_ = new QLabel();
		dofInfoLabel_->setText(u8"* 以下参数设置开启时渲染生效");
		dofInfoLabel_->setStyleSheet("color: rgb(100,100,100);");

		apertureLabel_ = new QLabel();
		apertureLabel_->setText(tr("aperture"));
		apertureLabel_->setStyleSheet("color: rgb(200,200,200);");

		apertureSpinbox_ = new DoubleSpinBox();
		apertureSpinbox_->setMinimum(1.0f);
		apertureSpinbox_->setMaximum(32.0f);
		apertureSpinbox_->setValue(32.0f);
		apertureSpinbox_->setSingleStep(0.1f);
		apertureSpinbox_->setAlignment(Qt::AlignRight);
		apertureSpinbox_->setFixedWidth(100);
		apertureSpinbox_->setPrefix(u8"f/");
		apertureSpinbox_->setDecimals(1);

		focalDistanceLabel_ = new QLabel();
		focalDistanceLabel_->setText(tr("focalPoint"));
		focalDistanceLabel_->setStyleSheet("color: rgb(200,200,200);");

		focalDistanceSpinbox_ = new DoubleSpinBox();
		focalDistanceSpinbox_->setMinimum(0);
		focalDistanceSpinbox_->setMaximum(std::numeric_limits<float>::infinity());
		focalDistanceSpinbox_->setValue(0);
		focalDistanceSpinbox_->setSingleStep(0.1f);
		focalDistanceSpinbox_->setAlignment(Qt::AlignRight);
		focalDistanceSpinbox_->setSuffix(u8"m");
		focalDistanceSpinbox_->setFixedWidth(100);

		focalDistanceName_ = new QLabel();
		focalDistanceName_->setText(u8"目标：无");
		focalDistanceName_->setStyleSheet("color: rgb(200,200,200);");

		focalTargetButton_ = new FocalTargetWindow();
		focalTargetButton_->setIcon(QIcon(":res/icons/target.png"));
		focalTargetButton_->setIconSize(QSize(48, 48));
		focalTargetButton_->setToolTip(u8"通过拖拽该图标到目标模型可绑定模型并开启自动测距");

		auto apertureLayout = new QHBoxLayout;
		apertureLayout->addWidget(apertureLabel_);
		apertureLayout->addWidget(apertureSpinbox_);

		auto focalDistanceLayout = new QHBoxLayout;
		focalDistanceLayout->addWidget(focalDistanceLabel_);
		focalDistanceLayout->addWidget(focalDistanceSpinbox_);

		auto focalTargetLayout = new QHBoxLayout;
		focalTargetLayout->addWidget(focalTargetButton_, 0, Qt::AlignLeft);
		focalTargetLayout->addWidget(focalDistanceName_);
		focalTargetLayout->addStretch();

		mainLayout_ = new QVBoxLayout;
		mainLayout_->addWidget(dofInfoLabel_, 0, Qt::AlignLeft);
		mainLayout_->addLayout(apertureLayout);
		mainLayout_->addLayout(focalDistanceLayout);
		mainLayout_->addLayout(focalTargetLayout);
		mainLayout_->addStretch();
		mainLayout_->setContentsMargins(30, 0, 20, 0);

		mainWidget_ = new QWidget;
		mainWidget_->setLayout(mainLayout_);

		this->setWidget(mainWidget_);
		
		connect(focalTargetButton_, SIGNAL(mouseMoveSignal()), this, SLOT(onUpdateTarget()));
		connect(apertureSpinbox_, SIGNAL(valueChanged(double)), this, SLOT(onApertureChanged(double)));
		connect(focalDistanceSpinbox_, SIGNAL(valueChanged(double)), this, SLOT(onFocalDistanceChanged(double)));
	}

	CameraDock::~CameraDock() noexcept
	{
	}

	void
	CameraDock::onUpdateTarget()
	{
		if (!profile_->playerModule->isPlaying)
		{
			auto hit = profile_->dragModule->selectedItemHover_;
			if (hit)
			{
				profile_->playerModule->dofTarget = hit;

				auto object = hit->object.lock();
				auto renderer = object->getComponent<octoon::MeshRendererComponent>();
				auto material = renderer->getMaterial(profile_->playerModule->dofTarget->mesh);

				focalDistanceName_->setText(QString::fromStdString(u8"目标：" + material->getName()));
				focalDistanceSpinbox_->setValue(0);
				focalDistanceSpinbox_->setSpecialValueText(u8"自动测距");
			}
			else
			{
				profile_->playerModule->dofTarget = hit;

				focalDistanceName_->setText(QString::fromStdString(u8"目标：无"));
				focalDistanceSpinbox_->setValue(10);
				focalDistanceSpinbox_->setSpecialValueText(QString());
			}
		}
	}

	void
	CameraDock::onApertureChanged(double value)
	{
		if (!profile_->playerModule->isPlaying && profile_->entitiesModule->camera)
			profile_->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->setAperture(value == 32.f ? 0.0f : 1.0f / value);
		else
			this->updateDefaultSetting();
	}

	void
	CameraDock::onFocalDistanceChanged(double value)
	{
		if (!profile_->playerModule->isPlaying && profile_->entitiesModule->camera)
		{
			if (!focalDistanceSpinbox_->specialValueText().isEmpty())
			{
				profile_->playerModule->dofTarget = std::nullopt;

				focalDistanceSpinbox_->setSpecialValueText(QString());
				focalDistanceSpinbox_->setValue(profile_->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->getFocalDistance());
			}
			else
			{
				profile_->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->setFocalDistance(value);
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
			float aperture = profile_->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->getAperture();
			apertureSpinbox_->setValue(aperture == 0.0f ? 32.0f : 1.0f / aperture);

			if (profile_->playerModule->dofTarget)
			{
				focalDistanceSpinbox_->setValue(0);
				focalDistanceSpinbox_->setSpecialValueText(u8"自动测距");

				auto object = profile_->playerModule->dofTarget->object.lock();
				auto renderer = object->getComponent<octoon::MeshRendererComponent>();
				auto material = renderer->getMaterial(profile_->playerModule->dofTarget->mesh);

				focalDistanceName_->setText(QString::fromStdString(u8"目标：" + material->getName()));
			}
			else
			{
				focalDistanceName_->setText(QString::fromStdString(u8"目标：无"));
				focalDistanceSpinbox_->setValue(profile_->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->getFocalDistance());
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