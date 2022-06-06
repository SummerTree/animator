#include "toplevel_bar.h"
#include <qmessagebox.h>

namespace unreal
{
	ToplevelBar::ToplevelBar(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile) noexcept
		: behaviour_(behaviour)
		, profile_(profile)
		, playEnable_(false)
		, playIcon_(QIcon::fromTheme("res", QIcon(":res/icons/play.png")))
		, playOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/play-on.png")))
		, volumeOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/volumeMiddle.png")))
		, volumeOffIcon_(QIcon::fromTheme("res", QIcon(":res/icons/volumeCross.png")))
	{
		this->setObjectName("ToplevelBar");
		this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
		playButton.setObjectName("play");
		playButton.setToolTip(tr("Play Animation"));
		playButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		leftButton.setObjectName("left");
		leftButton.setToolTip(tr("Backward 1 second"));
		leftButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		
		rightButton.setObjectName("right");
		rightButton.setToolTip(tr("Forward 1 second"));
		rightButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		resetButton.setObjectName("reset");
		resetButton.setToolTip(tr("Reset States"));
		resetButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		volumeButton.setObjectName("volumeMiddle");
		volumeButton.setToolTip(tr("Volume"));
		volumeButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		volumeSlider_.setObjectName("slider");
		volumeSlider_.setToolTip(tr("Time"));
		volumeSlider_.setOrientation(Qt::Horizontal);
		volumeSlider_.setRange(0, 100);
		volumeSlider_.setValue(100);
		volumeSlider_.setFixedWidth(80);
		
		layout_.setObjectName("ToplevelLayout");
		layout_.addStretch();
		layout_.addWidget(&resetButton);
		layout_.addWidget(&leftButton);
		layout_.addWidget(&playButton);
		layout_.addWidget(&rightButton);
		layout_.addWidget(&volumeButton);
		layout_.addWidget(&volumeSlider_);
		layout_.addStretch();
		layout_.setSpacing(10);
		layout_.setContentsMargins(0, 0, 0, 0);

		auto mainWidget = new QWidget;
		mainWidget->setLayout(&layout_);

		this->addWidget(mainWidget);

		profile->playerModule->isPlaying += [this](bool value) {
			this->repaint();
		};

		profile->playerModule->finish += [this](bool value) {
			this->repaint();
		};

		profile->soundModule->volume += [this](float value) {
			volumeSlider_.setValue(value * 100.0f);
		};

		this->connect(&resetButton, SIGNAL(clicked()), this, SLOT(resetEvent()));
		this->connect(&playButton, SIGNAL(clicked()), this, SLOT(playEvent()));
		this->connect(&leftButton, SIGNAL(clicked()), this, SLOT(leftEvent()));
		this->connect(&rightButton, SIGNAL(clicked()), this, SLOT(rightEvent()));
		this->connect(&volumeButton, SIGNAL(clicked()), this, SLOT(volumeEvent()));
		this->connect(&volumeSlider_, SIGNAL(valueChanged(int)), this, SLOT(volumeSliderEvent(int)));
		// this->connect(&slider_, SIGNAL(valueChanged(int)), this, SLOT(sliderEvent(int)));
	}

	ToplevelBar::~ToplevelBar() noexcept
	{
	}

	void
	ToplevelBar::playEvent() noexcept
	{
		try
		{
			if (behaviour_ && !profile_->recordModule->active)
			{
				auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
				if (behaviour)
				{
					if (playEnable_)
					{
						behaviour->pause();
						playButton.setIcon(playIcon_);
						playButton.setToolTip(tr("Play"));
						playEnable_ = false;
					}
					else
					{
						behaviour->play();
						playButton.setIcon(playOnIcon_);
						playButton.setToolTip(tr("Pause"));
						playEnable_ = true;
					}
				}
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(this, tr("Error"), e.what());
		}
	}

	void
	ToplevelBar::resetEvent() noexcept
	{
		auto resetSignal = [this]() -> bool
		{
			try
			{
				if (behaviour_ && !profile_->playerModule->isPlaying)
				{
					auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
					if (behaviour->isOpen())
					{
						auto player = dynamic_cast<PlayerComponent*>(behaviour->getComponent<PlayerComponent>());
						if (player)
						{
							player->reset();
							return true;
						}
					}
				}

				return false;
			}
			catch (const std::exception& e)
			{
				QMessageBox::critical(this, tr("Error"), e.what());
				return false;
			}
		};

		if (resetSignal())
		{
			playButton.setIcon(playIcon_);
			playButton.setToolTip(tr("Play"));
			playEnable_ = false;
		}
	}

	void
	ToplevelBar::leftEvent() noexcept
	{
		try
		{
			if (!profile_->playerModule->isPlaying)
			{
				auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
				if (behaviour->isOpen())
				{
					auto player = dynamic_cast<PlayerComponent*>(behaviour->getComponent<PlayerComponent>());
					if (player)
						player->sample(-1.0f);
				}
				else
				{
					QMessageBox::warning(this, tr("Warning"), tr("Please load a project with pmm extension."));
				}
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(this, tr("Error"), e.what());
		}
	}

	void 
	ToplevelBar::rightEvent() noexcept
	{
		try
		{
			if (!profile_->playerModule->isPlaying)
			{
				auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
				if (behaviour->isOpen())
				{
					auto player = dynamic_cast<PlayerComponent*>(behaviour->getComponent<PlayerComponent>());
					if (player)
						player->sample(1.0f);
				}
				else
				{
					QMessageBox::warning(this, tr("Warning"), tr("Please load a project with pmm extension."));
				}
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(this, tr("Error"), e.what());
		}
	}

	void
	ToplevelBar::volumeEvent() noexcept
	{
		if (!volumeEnable_)
		{
			this->profile_->soundModule->volume = 1.0f;
			volumeButton.setIcon(volumeOnIcon_);
			volumeButton.setToolTip(tr("Volume"));
			volumeEnable_ = true;
		}
		else
		{
			this->profile_->soundModule->volume = 0.0f;
			volumeButton.setIcon(volumeOffIcon_);
			volumeButton.setToolTip(tr("VolumeOff"));
			volumeEnable_ = false;
		}
	}
	
	void
	ToplevelBar::sliderEvent(int value)
	{
		if (behaviour_ && !profile_->playerModule->isPlaying)
		{
			auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
			if (behaviour->isOpen())
			{
				auto player = behaviour->getComponent<PlayerComponent>();
				if (player != nullptr)
				{
					auto& model = player->getModel();
					player->sample(value - model->curTime);
				}
			}
			else
			{
				QMessageBox::information(this, tr("Warning"), tr("Please load a project with pmm extension."));
			}
		}
	}

	void
	ToplevelBar::volumeSliderEvent(int value)
	{
		auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
		if (behaviour->isOpen())
		{
			this->profile_->soundModule->volume = value / 100.0f;

			if (value == 0 && volumeEnable_)
			{
				volumeButton.setIcon(volumeOffIcon_);
				volumeButton.setToolTip(tr("VolumeOff"));
				volumeEnable_ = false;
			}
			else if (value != 0 && !volumeEnable_)
			{
				volumeButton.setIcon(volumeOnIcon_);
				volumeButton.setToolTip(tr("Volume"));
				volumeEnable_ = true;
			}
		}
	}

	void
	ToplevelBar::paintEvent(QPaintEvent* e) noexcept
	{
		if (profile_->playerModule->isPlaying)
		{
			if (!playEnable_)
			{
				playButton.setIcon(playOnIcon_);
				playEnable_ = true;
			}
		}
		else
		{
			if (playEnable_)
			{
				playButton.setIcon(playIcon_);
				playEnable_ = false;
			}
		}
	}
}