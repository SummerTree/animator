#include "tool_window.h"
#include <qscrollarea.h>

namespace flower
{
	ToolWindow::ToolWindow(QWidget* parent, const octoon::GameObjectPtr& behaviour, std::shared_ptr<FlowerProfile> profile) noexcept
		: QWidget(parent)
		, profile_(profile)
		, gpuEnable_(false)
		, playEnable_(false)
		, audioEnable_(false)
		, recordEnable_(false)
		, hdrEnable_(false)
		, sunEnable_(false)
		, environmentEnable_(false)
		, behaviour_(behaviour)
		, playIcon_(QIcon::fromTheme("res", QIcon(":res/icons/play.png")))
		, playOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/play-on.png")))
		, gpuIcon_(QIcon::fromTheme("res", QIcon(":res/icons/gpu.png")))
		, gpuOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/gpu-on.png")))
		, recordIcon_(QIcon::fromTheme("res", QIcon(":res/icons/record.png")))
		, recordOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/record-on.png")))
		, audioIcon_(QIcon::fromTheme("res", QIcon(":res/icons/music.svg")))
		, audioOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/music-on.png")))
		, sunIcon_(QIcon::fromTheme("res", QIcon(":res/icons/sun.png")))
		, sunOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/sun-on.png")))
		, environmentIcon_(QIcon::fromTheme("res", QIcon(":res/icons/environment.png")))
		, environmentOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/environment-on.png")))
	{
		this->setObjectName("ToolWindow");
		this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

		hideButton.setObjectName("hide");
		hideButton.setText(tr("Hide"));
		hideButton.setToolTip(tr("Hide side toolbar"));
		hideButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		importButton.setObjectName("import");
		importButton.setText(tr("Import"));
		importButton.setToolTip(tr("Import Resource File(.pmm, .mdl)"));
		importButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		playButton.setObjectName("play");
		playButton.setText(tr("Play"));
		playButton.setToolTip(tr("Play Animation"));
		playButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		resetButton.setObjectName("reset");
		resetButton.setText(tr("Reset"));
		resetButton.setToolTip(tr("Reset States"));
		resetButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		leftButton.setObjectName("left");
		leftButton.setText(tr("Backward"));
		leftButton.setToolTip(tr("Backward 1 second"));
		leftButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		rightButton.setObjectName("right");
		rightButton.setText(tr("Forward"));
		rightButton.setToolTip(tr("Forward 1 second"));
		rightButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		recordButton.setObjectName("record");
		recordButton.setText(tr("Record"));
		recordButton.setToolTip(tr("Record Video"));
		recordButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		audioButton.setObjectName("audio");
		audioButton.setText(tr("Music"));
		audioButton.setToolTip(tr("Set Background Audio File"));
		audioButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		shotButton.setObjectName("shot");
		shotButton.setText(tr("Screenshot"));
		shotButton.setToolTip(tr("Denoising Screenshot"));
		shotButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		gpuButton.setObjectName("gpu");
		gpuButton.setText(tr("Render"));
		gpuButton.setToolTip(tr("Enable High Quality Rendering"));
		gpuButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		cleanupButton.setObjectName("cleanup");
		cleanupButton.setText(tr("Cleanup"));
		cleanupButton.setToolTip(tr("Cleanup Scene"));
		cleanupButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		materialButton.setObjectName("material");
		materialButton.setText(tr("Material"));
		materialButton.setToolTip(tr("Open Material Panel"));
		materialButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		lightButton.setObjectName("sun");
		lightButton.setText(tr("Light"));
		lightButton.setToolTip(tr("Light Settings"));
		lightButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		sunButton.setObjectName("sun");
		sunButton.setText(tr("Main Light"));
		sunButton.setToolTip(tr("Main Light Settings"));
		sunButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		environmentButton.setObjectName("environment");
		environmentButton.setText(tr("Environment Light"));
		environmentButton.setToolTip(tr("Environment Light Settings"));
		environmentButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		auto layout = new QVBoxLayout;
		layout->setSpacing(4);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addSpacing(1);
		layout->addWidget(&hideButton, 0, Qt::AlignCenter);
		layout->addWidget(&importButton, 0, Qt::AlignCenter);
		layout->addWidget(&playButton, 0, Qt::AlignCenter);
		layout->addWidget(&resetButton, 0, Qt::AlignCenter);
		layout->addWidget(&leftButton, 0, Qt::AlignCenter);
		layout->addWidget(&rightButton, 0, Qt::AlignCenter);
		layout->addWidget(&gpuButton, 0, Qt::AlignCenter);
		layout->addWidget(&recordButton, 0, Qt::AlignCenter);
		layout->addWidget(&shotButton, 0, Qt::AlignCenter);
		layout->addWidget(&audioButton, 0, Qt::AlignCenter);
		layout->addWidget(&materialButton, 0, Qt::AlignCenter);
		layout->addWidget(&lightButton, 0, Qt::AlignCenter);
		layout->addWidget(&sunButton, 0, Qt::AlignCenter);
		layout->addWidget(&environmentButton, 0, Qt::AlignCenter);
		layout->addWidget(&cleanupButton, 0, Qt::AlignCenter);
		layout->addStretch();

		contentWidget = new QWidget;
		contentWidget->setLayout(layout);

		contentWidgetArea = new QScrollArea();
		contentWidgetArea->setWidget(contentWidget);
		contentWidgetArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		contentWidgetArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		contentWidgetArea->setWidgetResizable(true);

		auto mainLayout = new QVBoxLayout(this);
		mainLayout->setSpacing(0);
		mainLayout->setContentsMargins(0, 0, 0, 0);
		mainLayout->addWidget(contentWidgetArea);

		this->connect(&hideButton, SIGNAL(clicked()), this, SLOT(hideEvent()));
		this->connect(&importButton, SIGNAL(clicked()), this, SLOT(importEvent()));
		this->connect(&playButton, SIGNAL(clicked()), this, SLOT(playEvent()));
		this->connect(&leftButton, SIGNAL(clicked()), this, SLOT(leftEvent()));
		this->connect(&rightButton, SIGNAL(clicked()), this, SLOT(rightEvent()));
		this->connect(&resetButton, SIGNAL(clicked()), this, SLOT(resetEvent()));
		this->connect(&recordButton, SIGNAL(clicked()), this, SLOT(recordEvent()));
		this->connect(&audioButton, SIGNAL(clicked()), this, SLOT(audioEvent()));
		this->connect(&shotButton, SIGNAL(clicked()), this, SLOT(shotEvent()));
		this->connect(&gpuButton, SIGNAL(clicked()), this, SLOT(gpuEvent()));
		this->connect(&cleanupButton, SIGNAL(clicked()), this, SLOT(cleanupEvent()));
		this->connect(&lightButton, SIGNAL(clicked()), this, SLOT(lightEvent()));
		this->connect(&sunButton, SIGNAL(clicked()), this, SLOT(sunEvent()));
		this->connect(&environmentButton, SIGNAL(clicked()), this, SLOT(environmentEvent()));
		this->connect(&materialButton, SIGNAL(clicked()), this, SLOT(materialEvent()));
	}

	ToolWindow::~ToolWindow() noexcept
	{
	}

	void
	ToolWindow::play()
	{
		if (!playEnable_)
		{
			if (playSignal(true))
			{
				playButton.setIcon(playOnIcon_);
				playButton.setToolTip(tr("Pause"));
				playEnable_ = true;
			}
		}
	}

	void
	ToolWindow::stop()
	{
		if (playEnable_)
		{
			playButton.setIcon(playIcon_);
			playButton.setToolTip(tr("Play"));
			playEnable_ = false;
		}
	}

	void
	ToolWindow::showEvent(QShowEvent* e) noexcept
	{
	}

	void
	ToolWindow::resizeEvent(QResizeEvent* e) noexcept
	{
	}

	void
	ToolWindow::hideEvent() noexcept
	{
		emit hideSignal();
	}

	void
	ToolWindow::importEvent() noexcept
	{
		emit importSignal();
	}

	void
	ToolWindow::playEvent() noexcept
	{
		if (!playEnable_)
		{
			if (playSignal(true))
			{
				playButton.setIcon(playOnIcon_);
				playButton.setToolTip(tr("Pause"));
				playEnable_ = true;
			}
		}
		else
		{
			if (playSignal(false))
			{
				playButton.setIcon(playIcon_);
				playButton.setToolTip(tr("Play"));
				playEnable_ = false;
			}
		}
	}

	void
	ToolWindow::resetEvent() noexcept
	{
		if (resetSignal())
		{
			playButton.setIcon(playIcon_);
			playButton.setToolTip(tr("Play"));
			playEnable_ = false;
		}
	}

	void
	ToolWindow::leftEvent() noexcept
	{
		emit leftSignal();
	}

	void 
	ToolWindow::rightEvent() noexcept
	{
		emit rightSignal();
	}

	void
	ToolWindow::recordEvent() noexcept
	{
		if (!recordEnable_)
		{
			if (recordSignal(true))
			{
				recordButton.setIcon(recordOnIcon_);
				recordEnable_ = true;
			}
		}
		else
		{
			if (recordSignal(false))
			{
				recordButton.setIcon(recordIcon_);
				recordEnable_ = false;
			}
		}
	}

	void 
	ToolWindow::audioEvent() noexcept
	{
		if (!audioEnable_)
		{
			if (audioSignal(true))
			{
				audioButton.setIcon(audioOnIcon_);
				audioEnable_ = true;
			}
		}
		else
		{
			if (audioSignal(false))
			{
				audioButton.setIcon(audioIcon_);
				audioEnable_ = false;
			}
		}
	}

	void
	ToolWindow::shotEvent() noexcept
	{
		emit shotSignal();
	}

	void
	ToolWindow::gpuEvent() noexcept
	{
		if (!gpuEnable_)
		{
			if (gpuSignal(true))
			{
				gpuButton.setIcon(gpuOnIcon_);
				gpuEnable_ = true;
			}
		}
		else
		{
			if (gpuSignal(false))
			{
				gpuButton.setIcon(gpuIcon_);
				gpuEnable_ = false;
			}
		}
	}

	void
	ToolWindow::cleanupEvent() noexcept
	{
		emit cleanupSignal();
	}

	void
	ToolWindow::lightEvent() noexcept
	{
		emit lightSignal();
	}

	void
	ToolWindow::sunEvent() noexcept
	{
		emit sunSignal();
	}

	void
	ToolWindow::materialEvent() noexcept
	{
		emit materialSignal();
	}

	void
	ToolWindow::environmentEvent() noexcept
	{
		emit environmentSignal();
	}

	void
	ToolWindow::paintEvent(QPaintEvent* e) noexcept
	{
		if (this->profile_->offlineModule->getEnable())
		{
			if (!gpuEnable_)
			{
				gpuButton.setIcon(gpuOnIcon_);
				gpuEnable_ = true;
			}
		}
		else
		{
			if (gpuEnable_)
			{
				gpuButton.setIcon(gpuIcon_);
				gpuEnable_ = false;
			}
		}

		if (this->profile_->entitiesModule->sound)
		{
			if (!audioEnable_)
			{
				audioButton.setIcon(audioOnIcon_);
				audioEnable_ = true;
			}
		}
		else
		{
			if (audioEnable_)
			{
				audioButton.setIcon(audioIcon_);
				audioEnable_ = false;
			}
		}
	}

	void
	ToolWindow::mousePressEvent(QMouseEvent* e) noexcept
	{
		allowMove_ = true;
		startPos_ = e->globalPos();
		clickPos_ = mapToParent(e->pos());
	}

	void
	ToolWindow::mouseReleaseEvent(QMouseEvent* e) noexcept
	{
		allowMove_ = false;
	}

	void
	ToolWindow::mouseMoveEvent(QMouseEvent* e) noexcept
	{
		if (allowMove_)
			parentWidget()->move(e->globalPos() - clickPos_);
	}
}