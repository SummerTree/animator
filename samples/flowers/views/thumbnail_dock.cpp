#include "thumbnail_dock.h"
#include <qscrollarea.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qmimedata.h>
#include <qprogressdialog.h>
#include <QtConcurrent/qtconcurrentrun.h>

namespace flower
{
	ThumbnailDock::ThumbnailDock(const octoon::GameAppPtr& gameApp, const octoon::GameObjectPtr& behaviour, std::shared_ptr<FlowerProfile> profile) noexcept
		: profile_(profile)
		, behaviour_(behaviour)
	{
		this->setObjectName("ThumbnailDock");
		this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
		this->setFeatures(DockWidgetFeature::DockWidgetMovable | DockWidgetFeature::DockWidgetFloatable);

		recordButton_ = new QToolButton;
		recordButton_->setObjectName("record");
		recordButton_->setText(tr("Record"));
		recordButton_->setToolTip(tr("Open Record Panel"));
		recordButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		materialButton_ = new QToolButton;
		materialButton_->setObjectName("material");
		materialButton_->setText(tr("Material"));
		materialButton_->setToolTip(tr("Open Material Panel"));
		materialButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		lightButton_ = new QToolButton;
		lightButton_->setObjectName("sun");
		lightButton_->setText(tr("Light"));
		lightButton_->setToolTip(tr("Open Light Panel"));
		lightButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		sunButton_ = new QToolButton;
		sunButton_->setObjectName("sun");
		sunButton_->setText(tr("Main Light"));
		sunButton_->setToolTip(tr("Open Main Light Panel"));
		sunButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		environmentButton_ = new QToolButton;
		environmentButton_->setObjectName("environment");
		environmentButton_->setText(tr("Environment Light"));
		environmentButton_->setToolTip(tr("Open Environment Light Panel"));
		environmentButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		cameraButton_ = new QToolButton;
		cameraButton_->setObjectName("camera");
		cameraButton_->setText(tr("Camera"));
		cameraButton_->setToolTip(tr("Open Camera Panel"));
		cameraButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		settingsButton_ = new QToolButton;
		settingsButton_->setObjectName("setting");
		settingsButton_->setText(tr("Settings"));
		settingsButton_->setToolTip(tr("Settings"));
		settingsButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		auto layout = new QVBoxLayout;
		layout->setSpacing(4);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(cameraButton_, 0, Qt::AlignCenter);
		layout->addWidget(recordButton_, 0, Qt::AlignCenter);
		layout->addWidget(materialButton_, 0, Qt::AlignCenter);
		layout->addWidget(lightButton_, 0, Qt::AlignCenter);
		layout->addWidget(sunButton_, 0, Qt::AlignCenter);
		layout->addWidget(environmentButton_, 0, Qt::AlignCenter);
		layout->addWidget(settingsButton_, 0, Qt::AlignCenter);
		layout->addStretch();

		auto contentWidget = new QWidget;
		contentWidget->setLayout(layout);

		auto contentWidgetArea = new QScrollArea();
		contentWidgetArea->setWidget(contentWidget);
		contentWidgetArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		contentWidgetArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		contentWidgetArea->setWidgetResizable(true);

		auto mainLayout = new QVBoxLayout();
		mainLayout->setSpacing(0);
		mainLayout->setContentsMargins(0, 0, 0, 0);
		mainLayout->addWidget(contentWidgetArea);

		auto mainWidget = new QWidget;
		mainWidget->setLayout(mainLayout);

		this->setWidget(mainWidget);

		lightButton_->hide();

		this->connect(recordButton_, SIGNAL(clicked()), this, SLOT(recordEvent()));
		this->connect(lightButton_, SIGNAL(clicked()), this, SLOT(lightEvent()));
		this->connect(sunButton_, SIGNAL(clicked()), this, SLOT(sunEvent()));
		this->connect(environmentButton_, SIGNAL(clicked()), this, SLOT(environmentEvent()));
		this->connect(materialButton_, SIGNAL(clicked()), this, SLOT(materialEvent()));
		this->connect(cameraButton_, SIGNAL(clicked()), this, SLOT(cameraEvent()));
		this->connect(settingsButton_, SIGNAL(clicked()), this, SLOT(settingsEvent()));
	}

	ThumbnailDock::~ThumbnailDock() noexcept
	{
	}

	void
	ThumbnailDock::closeEvent(QCloseEvent* event)
	{
		if (profile_->playerModule->isPlaying)
			event->ignore();
		else
			event->accept();
	}

	void
	ThumbnailDock::recordEvent() noexcept
	{
		emit recordSignal();
	}

	void
	ThumbnailDock::lightEvent() noexcept
	{
		emit lightSignal();
	}

	void
	ThumbnailDock::sunEvent() noexcept
	{
		emit sunSignal();
	}

	void
	ThumbnailDock::materialEvent() noexcept
	{
		emit materialSignal();
	}

	void
	ThumbnailDock::environmentEvent() noexcept
	{
		emit environmentSignal();
	}

	void
	ThumbnailDock::cameraEvent() noexcept
	{
		emit cameraSignal();
	}

	void
	ThumbnailDock::settingsEvent() noexcept
	{
		SettingWindow* window = new SettingWindow(this->behaviour_->getComponent<FlowerBehaviour>());
		window->show();
	}
}