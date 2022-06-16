﻿#include "main_dock.h"
#include "unreal_behaviour.h"
#include <qdockwidget.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <QDir>
#include <qfiledialog.h>
#include <qprogressdialog.h>

#include "spdlog/spdlog.h"

namespace unreal
{
	MainDock::MainDock(SplashScreen* splash) noexcept
		: init_flag(false)
		, profile_(std::make_unique<UnrealProfile>())
		, gameApp_(std::make_shared<octoon::GameApp>())
		, behaviour_(std::make_shared<octoon::GameObject>())
		, splash_(splash)
		, timer(this)
		, listener_(std::make_shared<SplashListener>(splash, QDir::homePath().toStdString() + "/.animator/log.txt"))
	{
		this->setObjectName("MainDock");
		this->setWindowTitle(tr("AnimatorGo Lite"));
		this->setDockNestingEnabled(false);
		this->setTabPosition(Qt::DockWidgetArea::AllDockWidgetAreas, QTabWidget::TabPosition::West);

		QImage image(":res/icons/logo.png");
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
		assetBrowseDock_ = std::make_unique<AssetBrowseDock>(behaviour_, profile_);
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

		this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, assetBrowseDock_.get());
		this->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, toolDock_.get());

		this->splitDockWidget(assetBrowseDock_.get(), materialDock_.get(), Qt::Orientation::Horizontal);
		this->splitDockWidget(assetBrowseDock_.get(), motionDock_.get(), Qt::Orientation::Horizontal);
		this->splitDockWidget(assetBrowseDock_.get(), modelDock_.get(), Qt::Orientation::Horizontal);
		this->splitDockWidget(assetBrowseDock_.get(), lightDock_.get(), Qt::Orientation::Horizontal);

		this->splitDockWidget(toolDock_.get(), mainLightDock_.get(), Qt::Orientation::Horizontal);
		this->splitDockWidget(toolDock_.get(), recordDock_.get(), Qt::Orientation::Horizontal);
		this->splitDockWidget(toolDock_.get(), environmentDock_.get(), Qt::Orientation::Horizontal);
		this->splitDockWidget(toolDock_.get(), cameraDock_.get(), Qt::Orientation::Horizontal);

		this->setCentralWidget(viewDock_.get());
		this->setStatusBar(statusBar_.get());

		lightDock_->hide();
		mainLightDock_->hide();
		environmentDock_->hide();
		materialDock_->hide();
		recordDock_->hide();
		cameraDock_->hide();
		modelDock_->hide();
		motionDock_->hide();

		this->connect(&timer, &QTimer::timeout, this, &MainDock::update);

		this->connect(assetBrowseDock_.get(), &AssetBrowseDock::lightSignal, this, &MainDock::onLightSignal);
		this->connect(assetBrowseDock_.get(), &AssetBrowseDock::materialSignal, this, &MainDock::onMaterialSignal);
		this->connect(assetBrowseDock_.get(), &AssetBrowseDock::modelSignal, this, &MainDock::onModelSignal);
		this->connect(assetBrowseDock_.get(), &AssetBrowseDock::motionSignal, this, &MainDock::onMotionSignal);
		this->connect(assetBrowseDock_.get(), &AssetBrowseDock::importSignal, this, &MainDock::onImportSignal);
		
		this->connect(toolDock_.get(), &ToolDock::sunSignal, this, &MainDock::onSunSignal);
		this->connect(toolDock_.get(), &ToolDock::recordSignal, this, &MainDock::onRecordSignal);
		this->connect(toolDock_.get(), &ToolDock::environmentSignal, this, &MainDock::onEnvironmentSignal);
		this->connect(toolDock_.get(), &ToolDock::cameraSignal, this, &MainDock::onCameraSignal);

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
		this->removeDockWidget(assetBrowseDock_.get());
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
		assetBrowseDock_.reset();
		cameraDock_.reset();
		motionDock_.reset();
		statusBar_.reset();

		spdlog::debug("Delete main dock");

		QDir appFolder = QDir(QDir::homePath() + "/.animator");
		if (!appFolder.exists())
			QDir::root().mkpath(QDir::homePath() + "/.animator");

		spdlog::debug("save profile");

		profile_->disconnect();
		profile_.reset();

		behaviour_.reset();
		gameApp_.reset();

		spdlog::debug("shutdown");
	}

	void
	MainDock::restoreLayout() noexcept
	{
		auto layout = QDir::homePath().toStdString() + "/.animator/layout.init";
		if (std::filesystem::exists(layout))
		{
			QSettings settings(QString::fromStdString(layout), QSettings::Format::IniFormat);
			settings.beginGroup("MainDock");
			restoreGeometry(settings.value("geometry").toByteArray());
			restoreState(settings.value("state").toByteArray());
			settings.endGroup();

			if (lightDock_->isVisible())
			{
				assetBrowseDock_->lightButton_->blockSignals(true);
				assetBrowseDock_->lightButton_->setChecked(true);
				assetBrowseDock_->lightButton_->blockSignals(false);
			}
			if (modelDock_->isVisible())
			{
				assetBrowseDock_->modelButton_->blockSignals(true);
				assetBrowseDock_->modelButton_->setChecked(true);
				assetBrowseDock_->modelButton_->blockSignals(false);
			}
			if (motionDock_->isVisible())
			{
				assetBrowseDock_->motionButton_->blockSignals(true);
				assetBrowseDock_->motionButton_->setChecked(true);
				assetBrowseDock_->motionButton_->blockSignals(false);
			}
			if (materialDock_->isVisible())
			{
				assetBrowseDock_->materialButton_->blockSignals(true);
				assetBrowseDock_->materialButton_->setChecked(true);
				assetBrowseDock_->materialButton_->blockSignals(false);
			}
			
			if (recordDock_->isVisible())
			{
				toolDock_->videoButton_->blockSignals(true);
				toolDock_->videoButton_->setChecked(true);
				toolDock_->videoButton_->blockSignals(false);
			}
			if (mainLightDock_->isVisible())
			{
				toolDock_->sunButton_->blockSignals(true);
				toolDock_->sunButton_->setChecked(true);
				toolDock_->sunButton_->blockSignals(false);
			}
			if (environmentDock_->isVisible())
			{
				toolDock_->environmentButton_->blockSignals(true);
				toolDock_->environmentButton_->setChecked(true);
				toolDock_->environmentButton_->blockSignals(false);
			}
			if (cameraDock_->isVisible())
			{
				toolDock_->cameraButton_->blockSignals(true);
				toolDock_->cameraButton_->setChecked(true);
				toolDock_->cameraButton_->blockSignals(false);
			}
		}
		else
		{
			viewDock_->resize(1024, 576);
		}
	}

	void
	MainDock::saveLayout() noexcept
	{
		auto layout = QDir::homePath().toStdString() + "/.animator/layout.init";
		QSettings settings(QString::fromStdString(layout), QSettings::Format::IniFormat);
		settings.beginGroup("MainDock");
		settings.setValue("geometry", saveGeometry());
		settings.setValue("state", saveState());
		settings.endGroup();
	}

	void
	MainDock::setTranslator(std::shared_ptr<QTranslator> translator)
	{
		translator_ = translator;
	}

	std::shared_ptr<QTranslator>
	MainDock::getTranslator()
	{
		return translator_;
	}

	void
	MainDock::onLanguageChanged(QString filename)	
	{
		auto app = QApplication::instance();
		// get all translators
		auto empty = translator_->isEmpty();
		if (translator_ && !translator_->isEmpty())
			app->removeTranslator(translator_.get());
		
		translator_->load(filename, ":res/languages/");
		app->installTranslator(translator_.get());
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

	void
	MainDock::onRecordSignal() noexcept
	{
		try
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
			if (lightDock_->isHidden())
			{
				auto widget = this->assetDock();
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
			if (materialDock_->isHidden())
			{
				auto widget = this->assetDock();
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
			if (modelDock_->isHidden())
			{
				auto widget = this->assetDock();
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
			if (motionDock_->isHidden())
			{
				auto widget = this->assetDock();
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
		catch (const std::exception& e)
		{
			QMessageBox::information(this, tr("Error"), e.what());
		}
	}

	void
	MainDock::onImportSignal() noexcept
	{
		spdlog::debug("Entered importEvent");

		try
		{
			if (!profile_->playerModule->isPlaying)
			{
				QStringList filepaths = QFileDialog::getOpenFileNames(this, tr("Import Resource"), "", tr("All Files(*.pmx *.mdl *.vmd *.hdr);; PMX Files (*.pmx);; VMD Files (*.vmd);; HDRi Files (*.hdr);; Material Files (*.mdl)"));
				if (!filepaths.isEmpty())
				{
					QProgressDialog dialog(tr("Loading..."), tr("Cancel"), 0, filepaths.size(), this);
					dialog.setWindowTitle(tr("Loading..."));
					dialog.setWindowModality(Qt::WindowModal);
					dialog.show();

					for (qsizetype i = 0; i < filepaths.size(); i++)
					{
						dialog.setValue(i);
						dialog.setLabelText(QFileInfo(filepaths[i]).fileName());

						QCoreApplication::processEvents();
						if (dialog.wasCanceled())
							break;

						auto package = octoon::AssetBundle::instance()->importAsset(filepaths[i].toStdWString());
						if (!package.is_null())
						{
							auto ext = std::filesystem::path(filepaths[i].toStdWString()).extension().wstring();
							for (auto& it : ext)
								it = (char)std::tolower(it);

							if (ext == L".pmx")
								this->modelDock_->addItem(package["uuid"].get<nlohmann::json::string_t>());
							else if (ext == L".vmd")
								this->motionDock_->addItem(package["uuid"].get<nlohmann::json::string_t>());
							else if (ext == L".mdl")
							{
								for (auto& it : package)
									this->materialDock_->addItem(it.get<nlohmann::json::string_t>());
							}
						}
					}

					octoon::AssetBundle::instance()->saveAssets();
				}
			}
		}
		catch (const std::exception& e)
		{
			spdlog::error("Function importEvent raised exception: " + std::string(e.what()));

			QCoreApplication::processEvents();
			QMessageBox::critical(this, tr("Error"), tr("Failed to import resource: ") + QString::fromStdString(e.what()));
		}

		spdlog::debug("Exited importEvent");
	}

	QDockWidget*
	MainDock::assetDock() noexcept
	{
		if (lightDock_->isVisible())
			return lightDock_.get();
		if (modelDock_->isVisible())
			return modelDock_.get();
		if (motionDock_->isVisible())
			return motionDock_.get();
		if (materialDock_->isVisible())
			return materialDock_.get();

		return nullptr;
	}

	QDockWidget*
	MainDock::visableDock() noexcept
	{
		if (recordDock_->isVisible())
			return recordDock_.get();
		if (mainLightDock_->isVisible())
			return mainLightDock_.get();
		if (environmentDock_->isVisible())
			return environmentDock_.get();
		if (cameraDock_->isVisible())
			return cameraDock_.get();

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

				spdlog::debug("behaviour init");

				behaviour_->addComponent<UnrealBehaviour>(profile_);

				spdlog::debug("finish");
				
				this->assetBrowseDock_->modelButton_->click();
				this->toolDock_->environmentButton_->click();

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