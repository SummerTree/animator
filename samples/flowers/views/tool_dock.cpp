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
		, recordEnable_(false)
		, hdrEnable_(false)
		, sunEnable_(false)
		, environmentEnable_(false)
		, behaviour_(behaviour)
		, gpuIcon_(QIcon::fromTheme("res", QIcon(":res/icons/gpu.png")))
		, gpuOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/gpu-on.png")))
		, recordIcon_(QIcon::fromTheme("res", QIcon(":res/icons/record.png")))
		, audioIcon_(QIcon::fromTheme("res", QIcon(":res/icons/music.svg")))
		, audioOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/music-on.png")))
		, sunIcon_(QIcon::fromTheme("res", QIcon(":res/icons/sun.png")))
		, sunOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/sun-on.png")))
		, environmentIcon_(QIcon::fromTheme("res", QIcon(":res/icons/environment.png")))
		, environmentOnIcon_(QIcon::fromTheme("res", QIcon(":res/icons/environment-on.png")))
	{
		this->setWindowTitle("Tool");
		this->setObjectName("ToolDock");
		this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
		this->setFeatures(DockWidgetFeature::DockWidgetMovable | DockWidgetFeature::DockWidgetFloatable);

		importButton.setObjectName("import");
		importButton.setText(tr("Import"));
		importButton.setToolTip(tr("Import Resource File(.pmm, .mdl)"));
		importButton.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

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
		layout->addWidget(&importButton, 0, Qt::AlignCenter);
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

		this->connect(&importButton, SIGNAL(clicked()), this, SLOT(importEvent()));
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
	ToolDock::recordEvent() noexcept
	{
		emit recordSignal();
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
	ToolDock::lightEvent() noexcept
	{
		emit lightSignal();
	}

	void
	ToolDock::sunEvent() noexcept
	{
		emit sunSignal();
	}

	void
	ToolDock::materialEvent() noexcept
	{
		emit materialSignal();
	}

	void
	ToolDock::environmentEvent() noexcept
	{
		emit environmentSignal();
	}

	void
	ToolDock::paintEvent(QPaintEvent* e) noexcept
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
}