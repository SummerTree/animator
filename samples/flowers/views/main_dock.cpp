#include "main_dock.h"
#include "flower_behaviour.h"
#include <qdockwidget.h>
#include <qmessagebox.h>

namespace flower
{
	MainDock::MainDock(SplashScreen* splash) noexcept
		: init_flag(false)
		, profile_(FlowerProfile::load("./config/config.conf"))
		, gameApp_(std::make_shared<octoon::GameApp>())
		, behaviour_(octoon::GameObject::create())
		, splash_(splash)
		, timer(this)
		, listener_(std::make_shared<SplashListener>(splash, "./log.txt"))
	{
		this->setObjectName("MainDock");
		this->setWindowTitle(tr("Flower Render Toolbox (Alpha Version)"));
		this->setDockNestingEnabled(true);
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
		materialDock_ = std::make_unique<MaterialDock>(behaviour_);
		statusBar_ = std::make_unique<StatusBar>(behaviour_, profile_);

		this->addToolBar(toplevelDock_.get());

		this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, toolDock_.get());
		this->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, thumbnailDock_.get());

		this->splitDockWidget(thumbnailDock_.get(), mainLightDock_.get(), Qt::Orientation::Horizontal);
		this->splitDockWidget(mainLightDock_.get(), lightDock_.get(), Qt::Orientation::Vertical);
		this->splitDockWidget(mainLightDock_.get(), materialDock_.get(), Qt::Orientation::Vertical);
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

		this->connect(&timer, SIGNAL(timeout()), this, SLOT(updateEvent()));

		timer.start();

		connect(thumbnailDock_.get(), &ThumbnailDock::sunSignal, this, &MainDock::onSunSignal);
		connect(thumbnailDock_.get(), &ThumbnailDock::lightSignal, this, &MainDock::onLightSignal);
		connect(thumbnailDock_.get(), &ThumbnailDock::recordSignal, this, &MainDock::onRecordSignal);
		connect(thumbnailDock_.get(), &ThumbnailDock::environmentSignal, this, &MainDock::onEnvironmentSignal);
		connect(thumbnailDock_.get(), &ThumbnailDock::materialSignal, this, &MainDock::onMaterialSignal);
		connect(thumbnailDock_.get(), &ThumbnailDock::cameraSignal, this, &MainDock::onCameraSignal);
	}

	MainDock::~MainDock() noexcept
	{
		this->removeToolBar(toplevelDock_.get());
		this->removeDockWidget(toolDock_.get());
		this->removeDockWidget(viewDock_.get());
		this->removeDockWidget(mainLightDock_.get());
		this->removeDockWidget(environmentDock_.get());
		this->removeDockWidget(materialDock_.get());
		this->removeDockWidget(recordDock_.get());
		this->removeDockWidget(thumbnailDock_.get());

		FlowerProfile::save("./config/config.conf", *profile_);
	}

	void 
	MainDock::paintEvent(QPaintEvent* e) noexcept
	{
	}

	void 
	MainDock::resizeEvent(QResizeEvent* e) noexcept
	{
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
		this->setupEvent();

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
		QRect rect = QApplication::primaryScreen()->geometry();
#else
		int currentScreen = QApplication::desktop()->screenNumber(this);
		QRect rect = QGuiApplication::screens().at(currentScreen)->geometry();
#endif
		this->move((rect.width() - this->width()) / 2, (rect.height() - this->height()) / 2);
	}

	void
	MainDock::updateEvent() noexcept
	{
		if (gameApp_ && init_flag)
			gameApp_->update();
	}

	void
	MainDock::setupEvent() noexcept
	{
		if (!init_flag)
		{
			open(viewDock_->width(), viewDock_->height());
			init_flag = true;
		}
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
			if (!profile_->playerModule->playing_ && !profile_->recordModule->active)
			{
				auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
				if (behaviour)
				{
					if (recordDock_->isHidden())
						recordDock_->show();
					else
						recordDock_->close();
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
	MainDock::onLightSignal() noexcept
	{
		try
		{
			auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
			if (behaviour)
			{
				if (profile_->entitiesModule->sunLight && !profile_->playerModule->playing_)
				{
					if (lightDock_->isHidden())
						lightDock_->show();
					else
						lightDock_->close();
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
	MainDock::onSunSignal() noexcept
	{
		try
		{
			auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
			if (behaviour)
			{
				if (profile_->entitiesModule->sunLight && !profile_->playerModule->playing_)
				{
					if (mainLightDock_->isHidden())
						mainLightDock_->show();
					else
						mainLightDock_->close();
				}
			}
			else
			{
				QMessageBox msg(this);
				msg.setWindowTitle(tr("Warning"));
				msg.setText(tr("Fail to get core component."));
				msg.setIcon(QMessageBox::Information);
				msg.setStandardButtons(QMessageBox::Ok);

				msg.exec();
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
	MainDock::onEnvironmentSignal() noexcept
	{
		try
		{
			auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
			if (behaviour)
			{
				if (profile_->entitiesModule->enviromentLight && !profile_->playerModule->playing_)
				{
					if (environmentDock_->isHidden())
						environmentDock_->show();
					else
						environmentDock_->close();
				}
			}
			else
			{
				QMessageBox msg(this);
				msg.setWindowTitle(tr("Warning"));
				msg.setText(tr("Fail to get core component."));
				msg.setIcon(QMessageBox::Information);
				msg.setStandardButtons(QMessageBox::Ok);

				msg.exec();
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
	MainDock::onMaterialSignal() noexcept
	{
		try
		{
			auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
			if (behaviour)
			{
				if (!profile_->playerModule->playing_)
				{
					if (materialDock_->isHidden())
						materialDock_->show();
					else
						materialDock_->close();
				}
			}
			else
			{
				QMessageBox msg(this);
				msg.setWindowTitle(tr("Warning"));
				msg.setText(tr("Fail to get core component."));
				msg.setIcon(QMessageBox::Information);
				msg.setStandardButtons(QMessageBox::Ok);

				msg.exec();
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
	MainDock::onCameraSignal() noexcept
	{
		try
		{
			auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
			if (behaviour)
			{
				if (!profile_->playerModule->playing_)
				{
					if (cameraDock_->isHidden())
						cameraDock_->show();
					else
						cameraDock_->close();
				}
			}
			else
			{
				QMessageBox msg(this);
				msg.setWindowTitle(tr("Warning"));
				msg.setText(tr("Fail to get core component."));
				msg.setIcon(QMessageBox::Information);
				msg.setStandardButtons(QMessageBox::Ok);

				msg.exec();
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
	MainDock::open(int w, int h) noexcept(false)
	{
		try
		{
			gameApp_->setGameListener(listener_);
			gameApp_->open((octoon::WindHandle)viewDock_->winId(), w, h, w, h);
			gameApp_->setActive(true);
			listener_->splash_ = nullptr;

			auto behaviour = behaviour_->addComponent<FlowerBehaviour>(profile_);
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
}