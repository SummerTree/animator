#include "views/main_dock.h"
#include "flower_behaviour.h"
#include <qdockwidget.h>
#include <qmessagebox.h>

namespace flower
{
	MainDock::MainDock(SplashScreen* splash) noexcept
		: init_flag(false)
		, profile_(flower::FlowerProfile::load("./config/config.conf"))
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
		this->setTabPosition(Qt::DockWidgetArea::AllDockWidgetAreas, QTabWidget::TabPosition::East);

		toplevelDock_ = std::make_unique<ToplevelDock>(behaviour_, profile_);
		toolDock_ = std::make_unique<ToolDock>(gameApp_, behaviour_, profile_);
		viewDock_ = std::make_unique<ViewDock>(gameApp_, behaviour_, profile_);
		mainLightDock_ = std::make_unique<MainLightDock>(behaviour_, profile_);
		environmentDock_ = std::make_unique<EnvironmentDock>(behaviour_, profile_);
		materialDock_ = std::make_unique<MaterialDock>(behaviour_);
		statusBar_ = std::make_unique<StatusBar>(behaviour_, profile_);

		this->addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, toplevelDock_.get());
		this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, toolDock_.get());
		this->addDockWidget(Qt::DockWidgetArea::AllDockWidgetAreas, materialDock_.get());
		this->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, mainLightDock_.get());

		this->setCentralWidget(viewDock_.get());
		this->tabifyDockWidget(mainLightDock_.get(), materialDock_.get());
		this->tabifyDockWidget(mainLightDock_.get(), environmentDock_.get());
		this->setStatusBar(statusBar_.get());

		environmentDock_->hide();
		materialDock_->hide();

		this->connect(&timer, SIGNAL(timeout()), this, SLOT(updateEvent()));

		timer.start();

		connect(toolDock_.get(), &ToolDock::sunSignal, this, &MainDock::onSunSignal);
		connect(toolDock_.get(), &ToolDock::lightSignal, this, &MainDock::onLightSignal);
		connect(toolDock_.get(), &ToolDock::recordSignal, this, &MainDock::onRecordSignal);
		connect(toolDock_.get(), &ToolDock::environmentSignal, this, &MainDock::onEnvironmentSignal);
		connect(toolDock_.get(), &ToolDock::materialSignal, this, &MainDock::onMaterialSignal);
	}

	MainDock::~MainDock() noexcept
	{
		this->removeDockWidget(toplevelDock_.get());
		this->removeDockWidget(toolDock_.get());
		this->removeDockWidget(viewDock_.get());
		this->removeDockWidget(mainLightDock_.get());
		this->removeDockWidget(environmentDock_.get());
		this->removeDockWidget(materialDock_.get());

		toplevelDock_.reset();
		toolDock_.reset();
		viewDock_.reset();
		mainLightDock_.reset();
		environmentDock_.reset();
		materialDock_.reset();
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
	MainDock::onLightSignal() noexcept
	{
		try
		{
			auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
			if (behaviour)
			{
				if (profile_->entitiesModule->sunLight && !profile_->playerModule->playing_)
				{
					/*if (lightWindow_->isHidden())
					{
						this->hideSliderWindow();
						this->setFixedWidth(this->width() + lightWindow_->minimumWidth());
						lightWindow_->show();
					}
					else
					{
						lightWindow_->close();
						this->setFixedWidth(this->width() - lightWindow_->width());
					}*/
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
			auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
			if (behaviour)
			{
				if (profile_->entitiesModule->sunLight && !profile_->playerModule->playing_)
				{
					if (mainLightDock_->isHidden())
					{
						//this->hideSliderWindow();
						//this->setFixedWidth(this->width() + mainLightDock_->minimumWidth());
						mainLightDock_->show();
						mainLightDock_->raise();
					}
					else
					{
						mainLightDock_->close();
						//this->setFixedWidth(this->width() - mainLightDock_->width());
					}
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
	MainDock::onRecordSignal() noexcept
	{
		try
		{
			if (!profile_->playerModule->playing_ && !profile_->recordModule->active)
			{
				auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
				if (behaviour->isOpen())
				{
					/*if (recordWindow_->isHidden())
					{
						this->hideSliderWindow();
						this->setFixedWidth(this->width() + recordWindow_->minimumWidth());
						recordWindow_->show();
					}
					else
					{
						recordWindow_->close();
						this->setFixedWidth(this->width() - recordWindow_->width());
					}*/
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
	MainDock::onEnvironmentSignal() noexcept
	{
		try
		{
			auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
			if (behaviour)
			{
				if (profile_->entitiesModule->enviromentLight && !profile_->playerModule->playing_)
				{
					if (environmentDock_->isHidden())
					{
						//this->hideSliderWindow();
						//this->setMinimumWidth(this->width() + environmentDock_->minimumWidth());
						environmentDock_->show();
						environmentDock_->raise();
					}
					else
					{
						environmentDock_->close();
						//this->setMinimumWidth(this->width() - environmentDock_->width());
					}
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
			auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
			if (behaviour)
			{
				if (!profile_->playerModule->playing_)
				{
					if (materialDock_->isHidden())
					{
						//this->hideSliderWindow();
						//this->setFixedWidth(this->width() + materialWindow_->minimumWidth());
						materialDock_->show();
						materialDock_->raise();
					}
					else
					{
						materialDock_->close();
						//this->setFixedWidth(this->width() - materialWindow_->width());
					}
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

			auto behaviour = behaviour_->addComponent<flower::FlowerBehaviour>(profile_);
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