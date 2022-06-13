#include "tool_dock.h"
#include "spdlog/spdlog.h"

namespace unreal
{
	ToolDock::ToolDock(const octoon::GameAppPtr& gameApp, const octoon::GameObjectPtr& behaviour, std::shared_ptr<UnrealProfile> profile) noexcept
		: profile_(profile)
		, behaviour_(behaviour)
	{
		this->setObjectName("ToolDock");
		this->setFeatures(DockWidgetFeature::NoDockWidgetFeatures);

		auto oldTitleBar = this->titleBarWidget();
		this->setTitleBarWidget(new QWidget());
		delete oldTitleBar;

		recordButton_ = new QToolButton;
		recordButton_->setObjectName("record");
		recordButton_->setText(tr("Record"));
		recordButton_->setToolTip(tr("Open Record Panel"));
		recordButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		recordButton_->setCheckable(true);
		recordButton_->installEventFilter(this);

		sunButton_ = new QToolButton;
		sunButton_->setObjectName("sun");
		sunButton_->setText(tr("Main Light"));
		sunButton_->setToolTip(tr("Open Main Light Panel"));
		sunButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		sunButton_->setCheckable(true);
		sunButton_->installEventFilter(this);

		environmentButton_ = new QToolButton;
		environmentButton_->setObjectName("environment");
		environmentButton_->setText(tr("Environment Light"));
		environmentButton_->setToolTip(tr("Open Environment Light Panel"));
		environmentButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		environmentButton_->setCheckable(true);
		environmentButton_->installEventFilter(this);
		environmentButton_->click();
		
		cameraButton_ = new QToolButton;
		cameraButton_->setObjectName("camera");
		cameraButton_->setText(tr("Camera"));
		cameraButton_->setToolTip(tr("Open Camera Panel"));
		cameraButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		cameraButton_->setCheckable(true);
		cameraButton_->installEventFilter(this);
		
		auto buttonGroup_ = new QButtonGroup();
		buttonGroup_->addButton(sunButton_, 0);
		buttonGroup_->addButton(environmentButton_, 1);
		buttonGroup_->addButton(cameraButton_, 2);
		buttonGroup_->addButton(recordButton_, 3);

		auto layout = new QVBoxLayout;
		layout->setSpacing(4);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(sunButton_, 0, Qt::AlignCenter);
		layout->addWidget(environmentButton_, 0, Qt::AlignCenter);
		layout->addWidget(cameraButton_, 0, Qt::AlignCenter);
		layout->addWidget(recordButton_, 0, Qt::AlignCenter);
		layout->addStretch();

		auto mainWidget = new QWidget;
		mainWidget->setObjectName("ToolWidget");
		mainWidget->setLayout(layout);

		this->setWidget(mainWidget);

		this->connect(recordButton_, SIGNAL(clicked()), this, SLOT(recordEvent()));
		this->connect(sunButton_, SIGNAL(clicked()), this, SLOT(sunEvent()));
		this->connect(environmentButton_, SIGNAL(clicked()), this, SLOT(environmentEvent()));
		this->connect(cameraButton_, SIGNAL(clicked()), this, SLOT(cameraEvent()));

		spdlog::debug("create tool dock");
	}

	ToolDock::~ToolDock() noexcept
	{
	}

	void
	ToolDock::recordEvent() noexcept
	{
		emit recordSignal();
	}

	void
	ToolDock::sunEvent() noexcept
	{
		emit sunSignal();
	}

	void
	ToolDock::environmentEvent() noexcept
	{
		emit environmentSignal();
	}

	void
	ToolDock::cameraEvent() noexcept
	{
		emit cameraSignal();
	}

	bool
	ToolDock::eventFilter(QObject* watched, QEvent* event)
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