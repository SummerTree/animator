#include "tool_dock.h"
#include <qscrollarea.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qmimedata.h>
#include <qprogressdialog.h>
#include <QtConcurrent/qtconcurrentrun.h>

namespace flower
{
	ToolDock::ToolDock(const octoon::GameAppPtr& gameApp, const octoon::GameObjectPtr& behaviour, std::shared_ptr<FlowerProfile> profile) noexcept
		: profile_(profile)
		, gpuEnable_(false)
		, audioEnable_(false)
		, hdrEnable_(false)
		, behaviour_(behaviour)
		, gpuIcon_(QIcon::fromTheme("res", QIcon(":res/icons/gpu.png")))
		, gpuOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/gpu-on.png")))
		, audioIcon_(QIcon::fromTheme("res", QIcon(":res/icons/music.svg")))
		, audioOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/music-on.png")))
	{
		this->setWindowTitle("Tool");
		this->setObjectName("ToolDock");
		this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
		this->setFeatures(DockWidgetFeature::DockWidgetMovable | DockWidgetFeature::DockWidgetFloatable);

		importButton_ = new QToolButton;
		importButton_->setObjectName("import");
		importButton_->setText(tr("Import"));
		importButton_->setToolTip(tr("Import Resource File(.pmm, .mdl)"));
		importButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		audioButton_ = new QToolButton;
		audioButton_->setObjectName("audio");
		audioButton_->setText(tr("Music"));
		audioButton_->setToolTip(tr("Set Background Audio File"));
		audioButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		shotButton_ = new QToolButton;
		shotButton_->setObjectName("shot");
		shotButton_->setText(tr("Screenshot"));
		shotButton_->setToolTip(tr("Denoising Screenshot"));
		shotButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		gpuButton_ = new QToolButton;
		gpuButton_->setObjectName("gpu");
		gpuButton_->setText(tr("Render"));
		gpuButton_->setToolTip(tr("Enable High Quality Rendering"));
		gpuButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		cleanupButton_ = new QToolButton;
		cleanupButton_->setObjectName("cleanup");
		cleanupButton_->setText(tr("Cleanup"));
		cleanupButton_->setToolTip(tr("Cleanup Scene"));
		cleanupButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

		auto layout = new QVBoxLayout;
		layout->setSpacing(4);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(importButton_, 0, Qt::AlignCenter);
		layout->addWidget(gpuButton_, 0, Qt::AlignCenter);
		layout->addWidget(shotButton_, 0, Qt::AlignCenter);
		layout->addWidget(audioButton_, 0, Qt::AlignCenter);
		layout->addWidget(cleanupButton_, 0, Qt::AlignCenter);
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

		this->connect(importButton_, SIGNAL(clicked()), this, SLOT(importEvent()));
		this->connect(audioButton_, SIGNAL(clicked()), this, SLOT(audioEvent()));
		this->connect(shotButton_, SIGNAL(clicked()), this, SLOT(shotEvent()));
		this->connect(gpuButton_, SIGNAL(clicked()), this, SLOT(gpuEvent()));
		this->connect(cleanupButton_, SIGNAL(clicked()), this, SLOT(cleanupEvent()));
	}

	ToolDock::~ToolDock() noexcept
	{
	}

	void
	ToolDock::importEvent() noexcept
	{
		try
		{
			if (behaviour_ && !profile_->playerModule->playing_)
			{
				auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
				if (behaviour)
				{
					QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"), "", tr("All Files(*.pmm *.pmx *.abc *.mdl);; PMM Files (*.pmm);; PMX Files (*.pmx);; Abc Files (*.abc);; Material Files (*.mdl)"));
					if (!fileName.isEmpty())
					{
						try
						{
#if 1
							behaviour->open(fileName.toUtf8().data());
#else
							// load task
							auto fn = [&]() {
								behaviour->open(fileName.toUtf8().data());
							};
							QFuture<void> fu = QtConcurrent::run(fn);

							// progress dialog
							QProgressDialog dialog(tr("Opening"), tr("Cancel"), 0, 2000, this);
							dialog.setWindowTitle(tr("Open Progress"));
							dialog.setWindowModality(Qt::WindowModal);
							dialog.show();
							for (int i = 0; i < 1900; i++)
							{
								dialog.setValue(i);
								QCoreApplication::processEvents();
								if (dialog.wasCanceled())
									break;
							}
							
							// wait finish
							fu.waitForFinished();

							// left progress
							for (int i = 1900; i < 2000; i++)
							{
								dialog.setValue(i);
								QCoreApplication::processEvents();
								if (dialog.wasCanceled())
									break;
							}
							dialog.setValue(2000);
#endif
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
	ToolDock::audioEvent() noexcept
	{
		auto audioSignal = [this](bool enable) -> bool
		{
			if (behaviour_ && !profile_->playerModule->playing_ && !profile_->recordModule->active)
			{
				auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
				if (behaviour)
				{
					try
					{
						if (enable)
						{
							QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"), "", tr("All Files(*.wav *.mp3 *.flac *.ogg);; Wav Files (*.wav);; MP3 Files (*.mp3);; FLAC Files (*.flac);; OGG Files (*.ogg)"));
							if (!fileName.isEmpty())
							{
								behaviour->loadAudio(fileName.toUtf8().data());
								return true;
							}
						}
						else
						{
							behaviour->clearAudio();
							return true;
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
			}

			return false;
		};

		if (!audioEnable_)
		{
			if (audioSignal(true))
			{
				audioButton_->setIcon(audioOnIcon_);
				audioEnable_ = true;
			}
		}
		else
		{
			if (audioSignal(false))
			{
				audioButton_->setIcon(audioIcon_);
				audioEnable_ = false;
			}
		}
	}

	void
	ToolDock::shotEvent() noexcept
	{
		try
		{
			if (behaviour_ && !profile_->playerModule->playing_ && !profile_->recordModule->active)
			{
				QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), "", tr("PNG Files (*.png)"));
				if (!fileName.isEmpty())
				{
					auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
					behaviour->renderPicture(fileName.toUtf8().data());
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
	ToolDock::gpuEvent() noexcept
	{
		auto gpuSignal = [this](bool enable) -> bool
		{
			try
			{
				if (behaviour_ && !profile_->recordModule->active)
				{
					auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
					if (behaviour)
					{
						auto offline = behaviour->getComponent<OfflineComponent>();
						if (offline)
						{
							if (enable)
								offline->setActive(true);
							else
								offline->setActive(false);
						}

						return true;
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

		if (!gpuEnable_)
		{
			if (gpuSignal(true))
			{
				gpuButton_->setIcon(gpuOnIcon_);
				gpuEnable_ = true;
			}
		}
		else
		{
			if (gpuSignal(false))
			{
				gpuButton_->setIcon(gpuIcon_);
				gpuEnable_ = false;
			}
		}
	}

	void
	ToolDock::cleanupEvent() noexcept
	{
		try
		{
			if (behaviour_ && !profile_->playerModule->playing_)
			{
				auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
				if (behaviour->isOpen())
					behaviour->close();
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
	ToolDock::paintEvent(QPaintEvent* e) noexcept
	{
		if (this->profile_->offlineModule->getEnable())
		{
			if (!gpuEnable_)
			{
				gpuButton_->setIcon(gpuOnIcon_);
				gpuEnable_ = true;
			}
		}
		else
		{
			if (gpuEnable_)
			{
				gpuButton_->setIcon(gpuIcon_);
				gpuEnable_ = false;
			}
		}

		if (this->profile_->entitiesModule->sound)
		{
			if (!audioEnable_)
			{
				audioButton_->setIcon(audioOnIcon_);
				audioEnable_ = true;
			}
		}
		else
		{
			if (audioEnable_)
			{
				audioButton_->setIcon(audioIcon_);
				audioEnable_ = false;
			}
		}
	}
}