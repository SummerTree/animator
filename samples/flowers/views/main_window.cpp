#include "main_window.h"
#include "flower_behaviour.h"
#include <qmimedata.h>
#include <qfiledialog.h>
#include <qcolordialog.h>
#include <qsplashscreen.h>
#include <qapplication.h>
// #include <qdesktopwidget.h>

#include <qscreen.h>
#include <qmessagebox.h>
#include <fstream>
#include <filesystem>
#include "controllers/offline_component.h"

namespace flower
{
	MainWindow::MainWindow(SplashScreen* splash) noexcept(false)
		: init_flag(false)
		, profile_(flower::FlowerProfile::load("./config/config.conf"))
		, behaviour_(octoon::GameObject::create())
		, splash_(splash)
		, listener_(std::make_shared<SplashListener>(splash, "./log.txt"))
	{
		this->setFrameShape(Panel);
		this->setObjectName("mainWindow");
		this->setWindowFlags(Qt::FramelessWindowHint);
		this->installEventFilter(this);
		
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

		//titleBar_ = std::make_unique<TitleDock>(behaviour_, profile_);
		//hideBar_ = std::make_unique<HideBar>(this);
		//toolBar_ = std::make_unique<ToolWindow>(behaviour_, profile_);
		//viewPanel_ = std::make_unique<ViewWidget>(behaviour_, profile_);
		lightWindow_ = std::make_unique<LightWindow>(profile_);
		//sunWindow_ = std::make_unique<MainLightDock>(profile_);
		//environmentWindow_ = std::make_unique<EnvironmentDock>(behaviour_, profile_);
		recordWindow_ = std::make_unique<RecordWindow>(this, behaviour_);
		//materialWindow_ = std::make_unique<MaterialWindow>(this, behaviour_);

		contextLayout_ = std::make_unique<QVBoxLayout>();
		contextLayout_->addWidget(toplevelDock_.get());
		contextLayout_->addWidget(viewDock_.get());
		contextLayout_->setContentsMargins(0, 0, 0, 0);
		contextLayout_->setSpacing(0);

		mainLayout_ = std::make_unique<QHBoxLayout>(this);
		mainLayout_->setContentsMargins(0, 0, 0, 0);
		mainLayout_->setSpacing(0);
		mainLayout_->addLayout(contextLayout_.get());
		mainLayout_->addWidget(toolBar_.get());
		mainLayout_->addWidget(lightWindow_.get());
		mainLayout_->addWidget(sunWindow_.get());
		mainLayout_->addWidget(environmentWindow_.get());
		mainLayout_->addWidget(recordWindow_.get());
		mainLayout_->addWidget(materialWindow_.get());
	}

	MainWindow::~MainWindow()
	{
		toolBar_.reset();
		viewDock_.reset();
		loginWindow_.reset();
		settingWindow_.reset();
		lightWindow_.reset();
		sunWindow_.reset();
		environmentWindow_.reset();
		toplevelDock_.reset();
		recordWindow_.reset();
		materialWindow_.reset();
		std::filesystem::create_directories("config");

		FlowerProfile::save("./config/config.conf", *profile_);

		behaviour_.reset();
		profile_.reset();		
		gameApp_.reset();
	}

	void
	MainWindow::onUpdateSignal() noexcept
	{
		if (!init_flag)
		{
			init_flag = true;
			open(viewDock_->width(), viewDock_->height());
		}

		if (gameApp_)
			gameApp_->update();
	}

	bool
	MainWindow::open(int w, int h) noexcept
	{
		assert(!gameApp_);

		try
		{
			gameApp_ = std::make_shared<octoon::GameApp>();
			gameApp_->setGameListener(listener_);
			gameApp_->open((octoon::WindHandle)viewDock_->winId(), w, h, w, h);
			gameApp_->setActive(true);
			listener_->splash_ = nullptr;

			auto behaviour = behaviour_->addComponent<flower::FlowerBehaviour>(profile_);
			behaviour->addMessageListener("flower:project:open", [this](const std::any&)
				{
					recordWindow_->repaint();
					sunWindow_->repaint();
					environmentWindow_->repaint();
				});

			/*behaviour->addMessageListener("flower:player:finish", [this](const std::any&)
				{
					if (toolBar_ && toolBar_->playEnable_)
						toolBar_->stop();
					else
						recordWindow_->stopRecord();
				});

			behaviour->addMessageListener("flower:offline", [this](const std::any& enable)
				{
					if (toolBar_)
					{
						if (std::any_cast<bool>(enable))
						{
							toolBar_->gpuButton.setIcon(toolBar_->gpuOnIcon_);
							toolBar_->gpuEnable_ = true;
						}
						else
						{
							toolBar_->gpuButton.setIcon(toolBar_->gpuIcon_);
							toolBar_->gpuEnable_ = false;
						}
					}
				});*/

			return true;
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