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
		, recordEnable_(false)
		, hdrEnable_(false)
		, sunEnable_(false)
		, environmentEnable_(false)
		, behaviour_(behaviour)
		, sunIcon_(QIcon::fromTheme("res", QIcon(":res/icons/sun.png")))
		, sunOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/sun-on.png")))
		, environmentIcon_(QIcon::fromTheme("res", QIcon(":res/icons/environment.png")))
		, environmentOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/environment-on.png")))
	{
		this->setWindowTitle("Thumbnail");
		this->setObjectName("ThumbnailDock");
		this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
		this->setFeatures(DockWidgetFeature::DockWidgetMovable | DockWidgetFeature::DockWidgetFloatable);

		recordButton_ = new QToolButton;
		recordButton_->setObjectName("record");
		recordButton_->setText(tr("Record"));
		recordButton_->setToolTip(tr("Record Video"));
		recordButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		materialButton_ = new QToolButton;
		materialButton_->setObjectName("material");
		materialButton_->setText(tr("Material"));
		materialButton_->setToolTip(tr("Open Material Panel"));
		materialButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		lightButton_ = new QToolButton;
		lightButton_->setObjectName("sun");
		lightButton_->setText(tr("Light"));
		lightButton_->setToolTip(tr("Light Settings"));
		lightButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		sunButton_ = new QToolButton;
		sunButton_->setObjectName("sun");
		sunButton_->setText(tr("Main Light"));
		sunButton_->setToolTip(tr("Main Light Settings"));
		sunButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		environmentButton_ = new QToolButton;
		environmentButton_->setObjectName("environment");
		environmentButton_->setText(tr("Environment Light"));
		environmentButton_->setToolTip(tr("Environment Light Settings"));
		environmentButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		auto layout = new QVBoxLayout;
		layout->setSpacing(4);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(recordButton_, 0, Qt::AlignCenter);
		layout->addWidget(materialButton_, 0, Qt::AlignCenter);
		layout->addWidget(lightButton_, 0, Qt::AlignCenter);
		layout->addWidget(sunButton_, 0, Qt::AlignCenter);
		layout->addWidget(environmentButton_, 0, Qt::AlignCenter);
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

		this->connect(recordButton_, SIGNAL(clicked()), this, SLOT(recordEvent()));
		this->connect(lightButton_, SIGNAL(clicked()), this, SLOT(lightEvent()));
		this->connect(sunButton_, SIGNAL(clicked()), this, SLOT(sunEvent()));
		this->connect(environmentButton_, SIGNAL(clicked()), this, SLOT(environmentEvent()));
		this->connect(materialButton_, SIGNAL(clicked()), this, SLOT(materialEvent()));
	}

	ThumbnailDock::~ThumbnailDock() noexcept
	{
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
	ThumbnailDock::paintEvent(QPaintEvent* e) noexcept
	{
	}
}