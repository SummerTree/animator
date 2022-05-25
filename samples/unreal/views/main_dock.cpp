#include "main_dock.h"
#include "unreal_behaviour.h"
#include <qdockwidget.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <QDir>

#include "spdlog/spdlog.h"

namespace unreal
{
	MainDock::MainDock(SplashScreen* splash) noexcept
		: init_flag(false)
		, profile_(UnrealProfile::load(QDir::homePath().toStdString() + "/.flower/config.json"))
		, gameApp_(std::make_shared<octoon::GameApp>())
		, behaviour_(octoon::GameObject::create())
		, splash_(splash)
		, timer(this)
		, listener_(std::make_shared<SplashListener>(splash, QDir::homePath().toStdString() + "/.flower/log.txt"))
	{
		this->setObjectName("MainDock");
		this->setWindowTitle(tr("Render Toolbox (Alpha Version)"));
		this->setDockNestingEnabled(false);
		this->installEventFilter(this);
		this->setTabPosition(Qt::DockWidgetArea::AllDockWidgetAreas, QTabWidget::TabPosition::West);

		QImage image(":res/icons/rabbit.png");
		auto w = image.width();
		auto h = image.height();
		auto bits = image.bits();
		auto channel = image.bitPlaneCount() / 8;

		profile_->markModule->width = w;
		profile_->markModule->height = h;
		profile_->markModule->channel = channel;
		profile_->markModule->pixels.resize(w * h * 4);
		std::memcpy(profile_->markModule->pixels.data(), bits, w * h * 4);

		toplevelDock_ = std::make_unique<ToplevelBar>(behaviour_, profile_);
		toolDock_ = std::make_unique<ToolDock>(gameApp_, behaviour_, profile_);
		thumbnailDock_ = std::make_unique<ThumbnailDock>(gameApp_, behaviour_, profile_);
		viewDock_ = std::make_unique<ViewDock>(gameApp_, behaviour_, profile_);
		recordDock_ = std::make_unique<RecordDock>(behaviour_, profile_);
		lightDock_ = std::make_unique<LightDock>(profile_);
		mainLightDock_ = std::make_unique<MainLightDock>(behaviour_, profile_);
		environmentDock_ = std::make_unique<EnvironmentDock>(behaviour_, profile_);
		cameraDock_ = std::make_unique<CameraDock>(behaviour_, profile_);
		materialDock_ = std::make_unique<MaterialDock>(behaviour_, profile_);
		modelDock_ = std::make_unique<ModelDock>(behaviour_, profile_);
		motionDock_ = std::make_unique<MotionDock>(behaviour_, profile_);
		statusBar_ = std::make_unique<StatusBar>(behaviour_, profile_);

		this->addToolBar(toplevelDock_.get());

		this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, toolDock_.get());
		this->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, thumbnailDock_.get());

		this->splitDockWidget(thumbnailDock_.get(), mainLightDock_.get(), Qt::Orientation::Horizontal);
		this->splitDockWidget(mainLightDock_.get(), lightDock_.get(), Qt::Orientation::Vertical);
		this->splitDockWidget(mainLightDock_.get(), materialDock_.get(), Qt::Orientation::Vertical);
		this->splitDockWidget(mainLightDock_.get(), motionDock_.get(), Qt::Orientation::Vertical);
		this->splitDockWidget(mainLightDock_.get(), modelDock_.get(), Qt::Orientation::Vertical);
		this->splitDockWidget(mainLightDock_.get(), recordDock_.get(), Qt::Orientation::Vertical);
		this->splitDockWidget(mainLightDock_.get(), environmentDock_.get(), Qt::Orientation::Vertical);
		this->splitDockWidget(mainLightDock_.get(), cameraDock_.get(), Qt::Orientation::Vertical);

		this->setCentralWidget(viewDock_.get());
		this->setStatusBar(statusBar_.get());

		lightDock_->hide();
		mainLightDock_->hide();
		environmentDock_->show();
		materialDock_->hide();
		recordDock_->hide();
		cameraDock_->hide();
		motionDock_->hide();

		this->connect(&timer, &QTimer::timeout, this, &MainDock::update);
		this->connect(thumbnailDock_.get(), &ThumbnailDock::sunSignal, this, &MainDock::onSunSignal);
		this->connect(thumbnailDock_.get(), &ThumbnailDock::lightSignal, this, &MainDock::onLightSignal);
		this->connect(thumbnailDock_.get(), &ThumbnailDock::recordSignal, this, &MainDock::onRecordSignal);
		this->connect(thumbnailDock_.get(), &ThumbnailDock::environmentSignal, this, &MainDock::onEnvironmentSignal);
		this->connect(thumbnailDock_.get(), &ThumbnailDock::materialSignal, this, &MainDock::onMaterialSignal);
		this->connect(thumbnailDock_.get(), &ThumbnailDock::modelSignal, this, &MainDock::onModelSignal);
		this->connect(thumbnailDock_.get(), &ThumbnailDock::cameraSignal, this, &MainDock::onCameraSignal);
		this->connect(thumbnailDock_.get(), &ThumbnailDock::motionSignal, this, &MainDock::onMotionSignal);

		timer.start();

		spdlog::debug("create main dock");
	}

	MainDock::~MainDock() noexcept
	{
		timer.stop();

		this->saveLayout();
		this->removeToolBar(toplevelDock_.get());
		this->removeDockWidget(toolDock_.get());
		this->removeDockWidget(viewDock_.get());
		this->removeDockWidget(lightDock_.get());
		this->removeDockWidget(mainLightDock_.get());
		this->removeDockWidget(environmentDock_.get());
		this->removeDockWidget(materialDock_.get());
		this->removeDockWidget(modelDock_.get());
		this->removeDockWidget(recordDock_.get());
		this->removeDockWidget(thumbnailDock_.get());
		this->removeDockWidget(cameraDock_.get());

		this->setStatusBar(nullptr);
		this->setCentralWidget(nullptr);
		
		toplevelDock_.reset();
		toolDock_.reset();
		viewDock_.reset();
		lightDock_.reset();
		mainLightDock_.reset();
		environmentDock_.reset();
		materialDock_.reset();
		modelDock_.reset();
		recordDock_.reset();
		thumbnailDock_.reset();
		cameraDock_.reset();
		motionDock_.reset();
		statusBar_.reset();

		spdlog::debug("Delete main dock");

		QDir appFolder = QDir(QDir::homePath() + "/.flower");
		if (!appFolder.exists())
			QDir::root().mkpath(QDir::homePath() + "/.flower");

		UnrealProfile::save(QDir::homePath().toStdString() + "/.flower/config.json", *profile_);

		spdlog::debug("save profile");

		behaviour_.reset();
		profile_.reset();
		gameApp_.reset();

		spdlog::debug("shutdown");
	}

	void
	MainDock::restoreLayout() noexcept
	{
		auto layout = QDir::homePath().toStdString() + "/.flower/layout.init";
		QSettings settings(QString::fromStdString(layout), QSettings::Format::IniFormat);
		settings.beginGroup("MainDock");
		restoreGeometry(settings.value("geometry").toByteArray());
		restoreState(settings.value("state").toByteArray());
		settings.endGroup();
	}

	void
	MainDock::saveLayout() noexcept
	{
		auto layout = QDir::homePath().toStdString() + "/.flower/layout.init";
		QSettings settings(QString::fromStdString(layout), QSettings::Format::IniFormat);
		settings.beginGroup("MainDock");
		settings.setValue("geometry", saveGeometry());
		settings.setValue("state", saveState());
		settings.endGroup();
	}

	void
	MainDock::dragEnterEvent(QDragEnterEvent* e) noexcept
	{
	}

	void
	MainDock::dragMoveEvent(QDragMoveEvent *e) noexcept
	{
	}

	void
	MainDock::dropEvent(QDropEvent* e) noexcept
	{
	}

	void
	MainDock::showEvent(QShowEvent* e) noexcept
	{
		this->open();

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
		QRect rect = QApplication::primaryScreen()->geometry();
#else
		int currentScreen = QApplication::desktop()->screenNumber(this);
		QRect rect = QGuiApplication::screens().at(currentScreen)->geometry();
#endif
		this->move((rect.width() - this->width()) / 2, (rect.height() - this->height()) / 2);
	}

	bool
	MainDock::eventFilter(QObject* watched, QEvent* event)
	{
		if (watched == this)
		{
			if (QEvent::WindowActivate == event->type())
			{
				this->activateWindow();
				return true;
			}
		}

		return QWidget::eventFilter(watched, event);
	}

	void
	MainDock::onRecordSignal() noexcept
	{
		try
		{
			if (!profile_->playerModule->isPlaying && !profile_->recordModule->active)
			{
				auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
				if (behaviour)
				{
					if (recordDock_->isHidden())
					{
						auto widget = this->visableDock();
						if (widget)
						{
							this->tabifyDockWidget(widget, recordDock_.get());
							widget->hide();
						}

						recordDock_->show();
					}
					else
					{
						recordDock_->close();
					}
				}
				else
				{
					QMessageBox::warning(this, tr("Warning"), tr("Please load a project with pmm extension."));
				}
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox::information(this, tr("Error"), e.what());
			spdlog::error("Function onRecordSignal raised exception: " + std::string(e.what()));
		}
	}

	void
	MainDock::onLightSignal() noexcept
	{
		try
		{
			auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
			if (behaviour)
			{
				if (profile_->entitiesModule->mainLight && !profile_->playerModule->isPlaying)
				{
					if (lightDock_->isHidden())
					{
						auto widget = this->visableDock();
						if (widget)
						{
							this->tabifyDockWidget(widget, lightDock_.get());
							widget->hide();
						}

						lightDock_->show();
					}
					else
					{
						lightDock_->close();
					}
				}
			}
			else
			{
				QMessageBox::warning(this, tr("Warning"), tr("Please load a project with pmm extension."));
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox::information(this, tr("Error"), e.what());
			spdlog::error("Function onLightSignal raised exception: " + std::string(e.what()));
		}
	}

	void
	MainDock::onSunSignal() noexcept
	{
		try
		{
			auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
			if (behaviour)
			{
				if (profile_->entitiesModule->mainLight && !profile_->playerModule->isPlaying)
				{
					if (mainLightDock_->isHidden())
					{
						auto widget = this->visableDock();
						if (widget)
						{
							this->tabifyDockWidget(widget, mainLightDock_.get());
							widget->hide();
						}

						mainLightDock_->show();
					}
					else
					{
						mainLightDock_->close();
					}
				}
			}
			else
			{
				QMessageBox::warning(this, tr("Warning"), tr("Fail to get core component."));
				spdlog::warn("Fail to get core component");
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox::information(this, tr("Error"), e.what());
			spdlog::error("Function onSunSignal raised exception: " + std::string(e.what()));
		}
	}

	void
	MainDock::onEnvironmentSignal() noexcept
	{
		try
		{
			auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
			if (behaviour)
			{
				if (profile_->entitiesModule->environmentLight && !profile_->playerModule->isPlaying)
				{
					if (environmentDock_->isHidden())
					{
						auto widget = this->visableDock();
						if (widget)
						{
							this->tabifyDockWidget(widget, environmentDock_.get());
							widget->hide();
						}

						environmentDock_->show();
					}
					else
					{
						environmentDock_->close();
					}
				}
			}
			else
			{
				QMessageBox::warning(this, tr("Warning"), tr("Fail to get core component."));
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox::information(this, tr("Error"), e.what());
		}
	}

	void
	MainDock::onMaterialSignal() noexcept
	{
		try
		{
			auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
			if (behaviour)
			{
				if (!profile_->playerModule->isPlaying)
				{
					if (materialDock_->isHidden())
					{
						auto widget = this->visableDock();
						if (widget)
						{
							this->tabifyDockWidget(widget, materialDock_.get());
							widget->hide();
						}

						materialDock_->show();
					}
					else
					{
						materialDock_->close();
					}
				}
			}
			else
			{
				QMessageBox::warning(this, tr("Warning"), tr("Fail to get core component."));
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(this, tr("Error"), e.what());
		}
	}

	void
	MainDock::onModelSignal() noexcept
	{
		try
		{
			auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
			if (behaviour)
			{
				if (!profile_->playerModule->isPlaying)
				{
					if (modelDock_->isHidden())
					{
						auto widget = this->visableDock();
						if (widget)
						{
							this->tabifyDockWidget(widget, modelDock_.get());
							widget->hide();
						}

						modelDock_->show();
					}
					else
					{
						modelDock_->close();
					}
				}
			}
			else
			{
				QMessageBox::warning(this, tr("Warning"), tr("Fail to get core component."));
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(this, tr("Error"), e.what());
		}
	}

	void
	MainDock::onCameraSignal() noexcept
	{
		try
		{
			auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
			if (behaviour)
			{
				if (!profile_->playerModule->isPlaying)
				{
					if (cameraDock_->isHidden())
					{
						auto widget = this->visableDock();
						if (widget)
						{
							this->tabifyDockWidget(widget, cameraDock_.get());
							widget->hide();
						}

						cameraDock_->show();
					}
					else
					{
						cameraDock_->close();
					}
				}
			}
			else
			{
				QMessageBox::warning(this, tr("Warning"), tr("Fail to get core component."));
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox::critical(this, tr("Error"), e.what());
		}
	}

	void
	MainDock::onMotionSignal() noexcept
	{
		try
		{
			auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
			if (behaviour)
			{
				if (!profile_->playerModule->isPlaying)
				{
					if (motionDock_->isHidden())
					{
						auto widget = this->visableDock();
						if (widget)
						{
							this->tabifyDockWidget(widget, motionDock_.get());
							widget->hide();
						}

						motionDock_->show();
					}
					else
					{
						motionDock_->close();
					}
				}
			}
			else
			{
				QMessageBox::warning(this, tr("Warning"), tr("Fail to get core component."));
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox::information(this, tr("Error"), e.what());
		}
	}

	QDockWidget*
	MainDock::visableDock() noexcept
	{
		if (recordDock_->isVisible())
			return recordDock_.get();
		if (lightDock_->isVisible())
			return lightDock_.get();
		if (mainLightDock_->isVisible())
			return mainLightDock_.get();
		if (environmentDock_->isVisible())
			return environmentDock_.get();
		if (materialDock_->isVisible())
			return materialDock_.get();
		if (cameraDock_->isVisible())
			return cameraDock_.get();
		if (modelDock_->isVisible())
			return modelDock_.get();
		if (motionDock_->isVisible())
			return motionDock_.get();

		return nullptr;
	}

	void
	MainDock::open() noexcept(false)
	{
		try
		{
			if (!init_flag)
			{
				spdlog::debug("game application init");

				auto w = viewDock_->width();
				auto h = viewDock_->height();

				gameApp_->setGameListener(listener_);
				gameApp_->open((octoon::WindHandle)viewDock_->winId(), w, h, w, h);
				gameApp_->setActive(true);

				listener_->splash_ = nullptr;

				spdlog::debug("flower behaviour init");

				behaviour_->addComponent<UnrealBehaviour>(profile_);

				spdlog::debug("finish");

				this->restoreLayout();

				init_flag = true;
			}
		}
		catch (const std::exception& e)
		{
			QMessageBox msg(this);
			msg.setWindowTitle(tr("Error"));
			msg.setText(tr("Current GPU does not support OpenCL or you are using an integrated GPU accelerator."));
			msg.setIcon(QMessageBox::Information);
			msg.setStandardButtons(QMessageBox::Ok);

			msg.exec();

			listener_->onMessage(e.what());

			gameApp_.reset();
			gameApp_ = nullptr;

			exit(0);
		}
	}

	void
	MainDock::update() noexcept
	{
		try
		{
			if (gameApp_ && init_flag)
				gameApp_->update();
		}
		catch (const std::exception& e)
		{
			QMessageBox msg(this);
			msg.setWindowTitle(tr("Error"));
			msg.setText(e.what());
			msg.setIcon(QMessageBox::Information);
			msg.setStandardButtons(QMessageBox::Ok);

			msg.exec();

			listener_->onMessage(e.what());

			gameApp_.reset();
			gameApp_ = nullptr;

			exit(0);
		}
	}
}