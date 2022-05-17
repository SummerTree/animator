#include "toplevel_bar.h"
#include <qmessagebox.h>

namespace flower
{
	ToplevelBar::ToplevelBar(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<FlowerProfile>& profile) noexcept
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
		
		layout_.setObjectName("ToplevelLayout");
		layout_.addStretch();
		layout_.addWidget(&resetButton);
		layout_.addWidget(&leftButton);
		layout_.addWidget(&playButton);
		layout_.addWidget(&rightButton);
		layout_.addWidget(&volumeButton);
		layout_.addStretch();
		layout_.setSpacing(10);
		layout_.setContentsMargins(10, 5, 10, 10);

		auto mainWidget = new QWidget;
		mainWidget->setLayout(&layout_);

		this->addWidget(mainWidget);

		behaviour->addMessageListener("flower:player:finish", [this](const std::any&) {
			this->repaint();
		});

		this->connect(&resetButton, SIGNAL(clicked()), this, SLOT(resetEvent()));
		this->connect(&playButton, SIGNAL(clicked()), this, SLOT(playEvent()));
		this->connect(&leftButton, SIGNAL(clicked()), this, SLOT(leftEvent()));
		this->connect(&rightButton, SIGNAL(clicked()), this, SLOT(rightEvent()));
		this->connect(&volumeButton, SIGNAL(clicked()), this, SLOT(volumeEvent()));
	}

	ToplevelBar::~ToplevelBar() noexcept
	{
	}

	void
	ToplevelBar::playEvent() noexcept
	{
		auto playSignal = [this](bool enable) noexcept
		{
			try
			{
				if (behaviour_ && !profile_->recordModule->active)
				{
					auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
					if (behaviour->isOpen())
					{
						if (enable)
							behaviour->play();
						else
							behaviour->pause();

						return true;
					}
					else
					{
						QMessageBox msg(this);
						msg.setWindowTitle(tr("Warning"));
						msg.setText(tr("Please load a project with pmm extension."));
						msg.setIcon(QMessageBox::Information);
						msg.setStandardButtons(QMessageBox::Ok);

						msg.exec();
						return false;
					}
				}

				return false;
			}
			catch (const std::exception& e)
			{
				QMessageBox msg(this);
				msg.setWindowTitle(tr("Error"));
				msg.setText(e.what());
				msg.setIcon(QMessageBox::Information);
				msg.setStandardButtons(QMessageBox::Ok);

				msg.exec();
				return false;
			}
		};

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
	ToplevelBar::resetEvent() noexcept
	{
		auto resetSignal = [this]() -> bool
		{
			try
			{
				if (behaviour_ && !profile_->playerModule->isPlaying)
				{
					auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
					if (behaviour->isOpen())
					{
						auto player = dynamic_cast<PlayerComponent*>(behaviour->getComponent<PlayerComponent>());
						if (player)
						{
							player->reset();
							return true;
						}
					}
					else
					{
						QMessageBox msg(this);
						msg.setWindowTitle(tr("Warning"));
						msg.setText(tr("Please load a project with pmm extension."));
						msg.setIcon(QMessageBox::Information);
						msg.setStandardButtons(QMessageBox::Ok);

						msg.exec();
					}
				}

				return false;
			}
			catch (const std::exception& e)
			{
				QMessageBox msg(this);
				msg.setWindowTitle(tr("Error"));
				msg.setText(e.what());
				msg.setIcon(QMessageBox::Information);
				msg.setStandardButtons(QMessageBox::Ok);

				msg.exec();
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
				auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
				if (behaviour->isOpen())
				{
					auto player = dynamic_cast<PlayerComponent*>(behaviour->getComponent<PlayerComponent>());
					if (player)
						player->sample(-1.0f);
				}
				else
				{
					QMessageBox msg(this);
					msg.setWindowTitle(tr("Warning"));
					msg.setText(tr("Please load a project with pmm extension."));
					msg.setIcon(QMessageBox::Information);
					msg.setStandardButtons(QMessageBox::Ok);

					msg.exec();
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
	ToplevelBar::rightEvent() noexcept
	{
		try
		{
			if (!profile_->playerModule->isPlaying)
			{
				auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
				if (behaviour->isOpen())
				{
					auto player = dynamic_cast<PlayerComponent*>(behaviour->getComponent<PlayerComponent>());
					if (player)
						player->sample(1.0f);
				}
				else
				{
					QMessageBox msg(this);
					msg.setWindowTitle(tr("Warning"));
					msg.setText(tr("Please load a project with pmm extension."));
					msg.setIcon(QMessageBox::Information);
					msg.setStandardButtons(QMessageBox::Ok);

					msg.exec();
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
	ToplevelBar::volumeEvent() noexcept
	{
		if (!volumeEnable_)
		{
			auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
			if (behaviour->isOpen())
			{
				behaviour->setVolume(1.0f);
				volumeButton.setIcon(volumeOnIcon_);
				volumeButton.setToolTip(tr("Volume"));
				volumeEnable_ = true;
			}
		}
		else
		{
			auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
			if (behaviour->isOpen())
			{
				behaviour->setVolume(0.0f);
				volumeButton.setIcon(volumeOffIcon_);
				volumeButton.setToolTip(tr("VolumeOff"));
				volumeEnable_ = false;
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