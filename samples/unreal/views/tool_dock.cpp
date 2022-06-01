#include "tool_dock.h"
#include "spdlog/spdlog.h"
#include <QtConcurrent/qtconcurrentrun.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qmimedata.h>
#include <qprogressdialog.h>
#include <qscrollarea.h>

namespace unreal
{
	ToolDock::ToolDock(const octoon::GameAppPtr& gameApp, const octoon::GameObjectPtr& behaviour, std::shared_ptr<UnrealProfile> profile) noexcept
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
		this->setObjectName("ToolDock");
		this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
		this->setFeatures(DockWidgetFeature::DockWidgetMovable | DockWidgetFeature::DockWidgetFloatable);

		openButton_ = new QToolButton;
		openButton_->setObjectName("import");
		openButton_->setText(tr("Open"));
		openButton_->setToolTip(tr("Open Project(.pmm, .apg)"));
		openButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		openButton_->installEventFilter(this);

		importButton_ = new QToolButton;
		importButton_->setObjectName("import");
		importButton_->setText(tr("Import"));
		importButton_->setToolTip(tr("Import Resource File(.pmm, .apg)"));
		importButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		importButton_->installEventFilter(this);

		saveButton_ = new QToolButton;
		saveButton_->setObjectName("save");
		saveButton_->setText(tr("Save"));
		saveButton_->setToolTip(tr("Export Project"));
		saveButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		saveButton_->installEventFilter(this);

		audioButton_ = new QToolButton;
		audioButton_->setObjectName("audio");
		audioButton_->setText(tr("Music"));
		audioButton_->setToolTip(tr("Set Background Audio File"));
		audioButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		audioButton_->installEventFilter(this);

		shotButton_ = new QToolButton;
		shotButton_->setObjectName("shot");
		shotButton_->setText(tr("Screenshot"));
		shotButton_->setToolTip(tr("Denoising Screenshot"));
		shotButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		shotButton_->installEventFilter(this);

		gpuButton_ = new QToolButton;
		gpuButton_->setObjectName("gpu");
		gpuButton_->setText(tr("Render"));
		gpuButton_->setToolTip(tr("Enable High Quality Rendering"));
		gpuButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		gpuButton_->installEventFilter(this);

		cleanupButton_ = new QToolButton;
		cleanupButton_->setObjectName("cleanup");
		cleanupButton_->setText(tr("Cleanup"));
		cleanupButton_->setToolTip(tr("Cleanup Scene"));
		cleanupButton_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		cleanupButton_->installEventFilter(this);

		auto layout = new QVBoxLayout;
		layout->setSpacing(4);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(openButton_, 0, Qt::AlignCenter);
		layout->addWidget(saveButton_, 0, Qt::AlignCenter);
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

		profile->offlineModule->enable += [this](bool value) {
			this->update();
		};

		profile->soundModule->sound += [this](const octoon::GameObjectPtr& value) {
			this->update();
		};

		this->connect(openButton_, SIGNAL(clicked()), this, SLOT(openEvent()));
		this->connect(importButton_, SIGNAL(clicked()), this, SLOT(importEvent()));
		this->connect(saveButton_, SIGNAL(clicked()), this, SLOT(saveEvent()));
		this->connect(audioButton_, SIGNAL(clicked()), this, SLOT(audioEvent()));
		this->connect(shotButton_, SIGNAL(clicked()), this, SLOT(shotEvent()));
		this->connect(gpuButton_, SIGNAL(clicked()), this, SLOT(gpuEvent()));
		this->connect(cleanupButton_, SIGNAL(clicked()), this, SLOT(cleanupEvent()));

		spdlog::debug("create tool dock");
	}

	ToolDock::~ToolDock() noexcept
	{
	}

	void
	ToolDock::openEvent() noexcept
	{
		spdlog::debug("Entered openEvent");
		try
		{
			if (behaviour_ && !profile_->playerModule->isPlaying)
			{
				auto behaviour = behaviour_->getComponent<unreal::UnrealBehaviour>();
				if (behaviour)
				{
					QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"), "", tr("All Files(*.pmm *.agp);; PMM Files (*.pmm);; APG Files (*.apg)"));
					if (!fileName.isEmpty())
					{
#if 1
						behaviour->open(fileName.toUtf8().toStdString());
#else
						// load task
						auto fn = [&]() {
							behaviour->open(fileName.toUtf8().toStdString());
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
				}
			}
		}
		catch (const std::exception& e)
		{
			QCoreApplication::processEvents();

			spdlog::error("Function importEvent raised exception: " + std::string(e.what()));
			QMessageBox::critical(this, tr("Error"), tr("Failed to open project: ") + QString::fromStdString(e.what()));
		}
		spdlog::debug("Exited openEvent");
	}

	void
	ToolDock::importEvent() noexcept
	{
		spdlog::debug("Entered importEvent");
		try
		{
			if (behaviour_ && !profile_->playerModule->isPlaying)
			{
				auto behaviour = behaviour_->getComponent<unreal::UnrealBehaviour>();
				if (behaviour)
				{
					QString fileName = QFileDialog::getOpenFileName(this, tr("Import Resource"), "", tr("All Files(*.pmx *.abc *.mdl *.vmd);; PMX Files (*.pmx);; Abc Files (*.abc);; VMD Files (*.vmd);; Material Files (*.mdl)"));
					if (!fileName.isEmpty())
					{
						behaviour->load(fileName.toUtf8().toStdString());
					}
				}
			}
		}
		catch (const std::exception& e)
		{
			QCoreApplication::processEvents();

			spdlog::error("Function importEvent raised exception: " + std::string(e.what()));
			QMessageBox::critical(this, tr("Error"), tr("Failed to import resource: ") + QString::fromStdString(e.what()));
		}

		spdlog::debug("Exited importEvent");
	}

	void
	ToolDock::saveEvent() noexcept
	{
		try
		{
			spdlog::debug("Entered saveEvent");

			if (behaviour_)
			{
				QString fileName = QFileDialog::getSaveFileName(this, tr("Export Project"), tr("New Project"), tr("APG Files (*.agp)"));
				if (!fileName.isEmpty())
				{
					auto behaviour = behaviour_->getComponent<unreal::UnrealBehaviour>();
					behaviour->save(fileName.toUtf8().toStdString());
				}
			}

			spdlog::debug("Exited saveEvent");
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(this, tr("Error"), e.what());
		}
	}

	void
	ToolDock::audioEvent() noexcept
	{
		spdlog::debug("Entered audioEvent");

		try
		{
			if (behaviour_ && !profile_->playerModule->isPlaying && !profile_->recordModule->active)
			{
				if (!audioEnable_)
				{
					QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"), "", tr("All Files(*.wav *.mp3 *.flac *.ogg);; Wav Files (*.wav);; MP3 Files (*.mp3);; FLAC Files (*.flac);; OGG Files (*.ogg)"));
					if (!fileName.isEmpty())
					{
						profile_->soundModule->filepath = fileName.toUtf8().toStdString();
						audioButton_->setIcon(audioOnIcon_);
						audioEnable_ = true;
					}
				}
				else
				{
					profile_->soundModule->filepath = std::string();
					audioButton_->setIcon(audioIcon_);
					audioEnable_ = false;
				}
			}
		}
		catch (const std::exception& e)
		{
			spdlog::error("Function audioEvent raised exception: " + std::string(e.what()));

			QCoreApplication::processEvents();
			QMessageBox::critical(this, tr("Error"), e.what());
		}

		spdlog::debug("Exited audioEvent");
	}

	void
	ToolDock::shotEvent() noexcept
	{
		try
		{
			spdlog::debug("Entered shotEvent");

			if (behaviour_)
			{
				QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), "", tr("PNG Files (*.png)"));
				if (!fileName.isEmpty())
				{
					auto behaviour = behaviour_->getComponent<unreal::UnrealBehaviour>();
					behaviour->renderPicture(fileName.toUtf8().toStdString());
				}
			}

			spdlog::debug("Exited shotEvent");
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(this, tr("Error"), e.what());
		}
	}

	void
	ToolDock::gpuEvent() noexcept
	{
		try
		{
			spdlog::debug("Entered gpuEvent");

			if (!gpuEnable_)
			{
				profile_->offlineModule->setEnable(true);
				gpuButton_->setIcon(gpuOnIcon_);
				gpuEnable_ = true;
			}
			else
			{
				profile_->offlineModule->setEnable(false);
				gpuButton_->setIcon(gpuIcon_);
				gpuEnable_ = false;
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(this, tr("Error"), e.what());
		}
	}

	void
	ToolDock::cleanupEvent() noexcept
	{
		try
		{
			spdlog::debug("Entered cleanupEvent");

			auto behaviour = behaviour_->getComponent<unreal::UnrealBehaviour>();
			if (behaviour)
				behaviour->close();

			spdlog::debug("Exited cleanupEvent");
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(this, tr("Error"), e.what());
		}
	}

	void
	ToolDock::update() noexcept
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

		if (this->profile_->soundModule->sound.getValue())
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

	bool
	ToolDock::eventFilter(QObject* watched, QEvent* event)
	{
		if (event->type() != QEvent::Paint)
		{
			if (profile_->playerModule->isPlaying)
			{
				return true;
			}
		}

		return QWidget::eventFilter(watched, event);
	}
}