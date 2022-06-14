#include "record_dock.h"
#include <qapplication.h>
#include <qdrag.h>
#include <qevent.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qmimedata.h>

namespace unreal
{
	RecordDock::RecordDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile) noexcept
		: behaviour_(behaviour)
		, profile_(profile)
	{
		this->setObjectName("RecordDock");
		this->setWindowTitle(tr("Video"));
		this->setFeatures(QDockWidget::NoDockWidgetFeatures);

		auto oldTitleBar = this->titleBarWidget();
		this->setTitleBarWidget(new QWidget());
		delete oldTitleBar;

		auto title_ = new QLabel;
		title_->setObjectName("title");
		title_->setText(tr("Video Editor"));
		title_->setContentsMargins(0, 10, 0, 10);

		auto headerLine = new QFrame;
		headerLine->setObjectName("Separator");
		headerLine->setFrameShape(QFrame::HLine);
		headerLine->setFrameShadow(QFrame::Sunken);
		headerLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		headerLine->setContentsMargins(0, 10, 0, 10);

		resolutionLabel = new QLabel(this);
		resolutionLabel->setText(tr("Resolution"));

		resolutionCombo = new QComboBox(this);
		resolutionCombo->addItem("720*480");
		resolutionCombo->addItem("800*480");
		resolutionCombo->addItem("1024*576");
		resolutionCombo->addItem("1280x720");
		resolutionCombo->addItem("1920x1080");
		resolutionCombo->addItem("540x960");
		resolutionCombo->addItem("720x1280");
		resolutionCombo->addItem("1080x1920");
		resolutionCombo->installEventFilter(this);

		quality_ = new QLabel();
		quality_->setText(tr("Render Pipeline"));

		select1_ = new QPushButton();
		select1_->setObjectName("select1");
		select1_->setText(tr("Ultra Render"));
		select1_->setCheckable(true);
		select1_->click();
		select1_->installEventFilter(this);

		select2_ = new QPushButton();
		select2_->setObjectName("select2");
		select2_->setText(tr("Realtime Render"));
		select2_->setCheckable(true);
		select2_->installEventFilter(this);

		group_ = new QButtonGroup();
		group_->addButton(select1_, 0);
		group_->addButton(select2_, 1);

		videoRatio_ = new QLabel();
		videoRatio_->setText(tr("Frame Per Second"));

		speed1_ = new QPushButton();
		speed1_->setObjectName("speed1");
		speed1_->setText(tr("24"));
		speed1_->setCheckable(true);
		speed1_->installEventFilter(this);

		speed2_ = new QPushButton();
		speed2_->setObjectName("speed2");
		speed2_->setText(tr("25"));
		speed2_->setCheckable(true);
		speed2_->installEventFilter(this);
		
		speed3_ = new QPushButton();
		speed3_->setObjectName("speed3");
		speed3_->setText(tr("30"));
		speed3_->setCheckable(true);
		speed3_->click();
		speed3_->installEventFilter(this);

		speed4_ = new QPushButton();
		speed4_->setObjectName("speed4");
		speed4_->setText(tr("60"));
		speed4_->setCheckable(true);
		speed4_->installEventFilter(this);

		speedGroup_ = new QButtonGroup();
		speedGroup_->addButton(speed1_, 0);
		speedGroup_->addButton(speed2_, 1);
		speedGroup_->addButton(speed3_, 2);
		speedGroup_->addButton(speed4_, 3);

		// output video type
		encodeType_ = new QLabel();
		encodeType_->setText(tr("Encode Type"));

		mode1_ = new QPushButton();
		mode1_->setObjectName("mode1");
		mode1_->setText(tr("H264"));
		mode1_->setCheckable(true);
		mode1_->installEventFilter(this);

		mode2_ = new QPushButton();
		mode2_->setObjectName("mode2");
		mode2_->setText(tr("H265"));
		mode2_->setCheckable(true);
		mode2_->installEventFilter(this);

		mode3_ = new QPushButton();
		mode3_->setObjectName("mode3");
		mode3_->setText(tr("Images"));
		mode3_->setCheckable(true);
		mode3_->click();
		mode3_->installEventFilter(this);

		modeGroup_ = new QButtonGroup();
		modeGroup_->addButton(mode1_, 0);
		modeGroup_->addButton(mode2_, 1);
		modeGroup_->addButton(mode3_, 2);
		
		frame_ = new QLabel();
		frame_->setText(tr("Play:"));

		startLabel_ = new QLabel();
		startLabel_->setText(tr("Start"));

		endLabel_ = new QLabel();
		endLabel_->setText(tr("- End"));

		startFrame_ = new USpinBox();
		startFrame_->setObjectName("start");
		startFrame_->setAlignment(Qt::AlignRight);
		startFrame_->setMinimum(0);
		startFrame_->setMaximum(99999);
		startFrame_->installEventFilter(this);

		endFrame_ = new USpinBox();
		endFrame_->setObjectName("end");
		endFrame_->setAlignment(Qt::AlignRight);
		endFrame_->setMinimum(0);
		endFrame_->setMaximum(99999);
		endFrame_->installEventFilter(this);

		denoiseLabel_ = new QLabel();
		denoiseLabel_->setText(tr("Denoise:"));

		denoiseButton_ = new QCheckBox();
		denoiseButton_->setCheckState(Qt::CheckState::Checked);
		denoiseButton_->installEventFilter(this);

		auto denoiseLayout_ = new QHBoxLayout();
		denoiseLayout_->addWidget(denoiseLabel_, 0, Qt::AlignLeft);
		denoiseLayout_->addWidget(denoiseButton_, 0, Qt::AlignLeft);
		denoiseLayout_->setSpacing(0);
		denoiseLayout_->setContentsMargins(0, 0, 0, 0);

		bouncesSpinbox_ = USpinLine::create(this, tr("Recursion depth per pixel:"), 1, 32, 1, 0);

		sppSpinbox_ = USpinLine::create(this, tr("Sample number per pixel:"), 1, 9999, 1, 0);

		crfSpinbox = UDoubleSpinLine::create(this, tr("Constant Rate Factor (CRF):"), 0.0f, 63.0f, 0);

		frameLayout_ = new QHBoxLayout();
		frameLayout_->addSpacing(20);
		frameLayout_->addWidget(startLabel_, 0, Qt::AlignLeft);
		frameLayout_->addWidget(startFrame_, 0, Qt::AlignLeft);
		frameLayout_->addWidget(endLabel_, 0, Qt::AlignLeft);
		frameLayout_->addWidget(endFrame_, 0, Qt::AlignLeft);
		frameLayout_->addStretch();

		recordButton_ = new QPushButton();
		recordButton_->setObjectName("render");
		recordButton_->setText(tr("Start Render"));
		recordButton_->setContentsMargins(0, 0, 0, 0);
		
		auto selectLayout_ = new QHBoxLayout();
		selectLayout_->addWidget(select1_, 0, Qt::AlignRight);
		selectLayout_->addWidget(select2_, 0, Qt::AlignLeft);
		selectLayout_->setSpacing(0);
		selectLayout_->setContentsMargins(0, 0, 0, 0);

		videoRatioLayout_ = new QHBoxLayout();
		videoRatioLayout_->addStretch();
		videoRatioLayout_->addWidget(speed1_, 0, Qt::AlignRight);
		videoRatioLayout_->addWidget(speed2_, 0, Qt::AlignVCenter);
		videoRatioLayout_->addWidget(speed3_, 0, Qt::AlignVCenter);
		videoRatioLayout_->addWidget(speed4_, 0, Qt::AlignLeft);
		videoRatioLayout_->addStretch();
		videoRatioLayout_->setContentsMargins(0, 0, 0, 0);

		auto encodeLayout_ = new QHBoxLayout();
		encodeLayout_->addStretch();
		encodeLayout_->addWidget(mode1_, 0, Qt::AlignLeft);
		encodeLayout_->addWidget(mode2_, 0, Qt::AlignVCenter);
		encodeLayout_->addWidget(mode3_, 0, Qt::AlignRight);
		encodeLayout_->addStretch();
		encodeLayout_->setContentsMargins(0, 0, 0, 0);

		auto videoLayout = new QVBoxLayout;
		videoLayout->addSpacing(10);
		videoLayout->addWidget(resolutionLabel);
		videoLayout->addWidget(resolutionCombo);
		videoLayout->addWidget(quality_);
		videoLayout->addLayout(selectLayout_);
		videoLayout->addSpacing(10);
		videoLayout->addWidget(videoRatio_);
		videoLayout->addLayout(videoRatioLayout_);
		videoLayout->addSpacing(10);
		videoLayout->addWidget(encodeType_);
		videoLayout->addLayout(encodeLayout_);
		videoLayout->addSpacing(15);
		videoLayout->addWidget(frame_);
		videoLayout->addLayout(frameLayout_);
		videoLayout->addLayout(denoiseLayout_);
		videoLayout->addWidget(sppSpinbox_);
		videoLayout->addWidget(bouncesSpinbox_);
		videoLayout->addWidget(crfSpinbox);
		videoLayout->setContentsMargins(20, 0, 20, 10);

		videoSpoiler_ = new Spoiler(tr("Render Settings"));
		videoSpoiler_->setContentLayout(*videoLayout);
		videoSpoiler_->toggleButton.click();

		mainLayout_ = new QVBoxLayout();
		mainLayout_->addWidget(title_);
		mainLayout_->addWidget(headerLine);
		mainLayout_->addWidget(videoSpoiler_);
		mainLayout_->addStretch();
		mainLayout_->addWidget(recordButton_, 0, Qt::AlignCenter);
		mainLayout_->setContentsMargins(10, 10, 10, 10);

		mainWidget_ = new QWidget;
		mainWidget_->setObjectName("RecordWidget");
		mainWidget_->setLayout(mainLayout_);
		
		this->setWidget(mainWidget_);

		profile_->cameraModule->framebufferSize += [this](const octoon::math::uint2& framebufferSize)
		{
			resolutionCombo->blockSignals(true);

			if (framebufferSize.x == 720 && framebufferSize.y == 480)
				resolutionCombo->setCurrentIndex(0);
			else if (framebufferSize.x == 800 && framebufferSize.y == 480)
				resolutionCombo->setCurrentIndex(1);
			else if (framebufferSize.x == 1024 && framebufferSize.y == 576)
				resolutionCombo->setCurrentIndex(2);
			else if (framebufferSize.x == 1280 && framebufferSize.y == 720)
				resolutionCombo->setCurrentIndex(3);
			else if (framebufferSize.x == 1920 && framebufferSize.y == 1080)
				resolutionCombo->setCurrentIndex(4);
			else if (framebufferSize.x == 540 && framebufferSize.y == 960)
				resolutionCombo->setCurrentIndex(5);
			else if (framebufferSize.x == 720 && framebufferSize.y == 1280)
				resolutionCombo->setCurrentIndex(6);
			else if (framebufferSize.x == 1080 && framebufferSize.y == 1920)
				resolutionCombo->setCurrentIndex(7);
			else
				throw std::runtime_error("resolution not found");

			resolutionCombo->blockSignals(false);
		};

		profile_->offlineModule->bounces += [this](std::uint32_t value)
		{
			bouncesSpinbox_->blockSignals(true);
			bouncesSpinbox_->setValue(value);
			bouncesSpinbox_->blockSignals(false);
		};

		profile_->offlineModule->spp += [this](std::uint32_t value)
		{
			sppSpinbox_->blockSignals(true);
			sppSpinbox_->setValue(value);
			sppSpinbox_->blockSignals(false);
		};

		profile_->encodeModule->crf += [this](float value)
		{
			crfSpinbox->blockSignals(true);
			crfSpinbox->doublespinbox_->setValue(value);
			crfSpinbox->blockSignals(false);
		};

		profile_->recordModule->denoise += [this](bool value)
		{
			denoiseButton_->blockSignals(true);
			denoiseButton_->setChecked(value);
			denoiseButton_->blockSignals(false);
		};

		profile_->playerModule->startFrame += [this](std::uint32_t value)
		{
			startFrame_->blockSignals(true);
			startFrame_->setValue(value);
			startFrame_->blockSignals(false);
		};

		profile_->playerModule->endFrame += [this](std::uint32_t value)
		{
			endFrame_->blockSignals(true);
			endFrame_->setValue(value);
			endFrame_->blockSignals(false);
		};

		profile_->playerModule->recordFps += [this](float value)
		{
			if (value == 24)
				speed1_->click();
			else if (value == 25)
				speed2_->click();
			else if (value == 30)
				speed3_->click();
			else if (value == 60)
				speed4_->click();
		};

		profile_->playerModule->finish += [this](bool value)
		{
			if (profile_->recordModule->active)
			{
				if (value)
					recordButton_->setText(tr("Start Render"));
				else
					recordButton_->setText(tr("Stop Render"));
			}
		};

		profile_->recordModule->active += [this](bool value)
		{
			if (value)
				recordButton_->setText(tr("Stop Render"));
			else
				recordButton_->setText(tr("Start Render"));
		};

		connect(select1_, SIGNAL(toggled(bool)), this, SLOT(select1Event(bool)));
		connect(select2_, SIGNAL(toggled(bool)), this, SLOT(select2Event(bool)));
		connect(speed1_, SIGNAL(toggled(bool)), this, SLOT(speed1Event(bool)));
		connect(speed2_, SIGNAL(toggled(bool)), this, SLOT(speed2Event(bool)));
		connect(speed3_, SIGNAL(toggled(bool)), this, SLOT(speed3Event(bool)));
		connect(speed4_, SIGNAL(toggled(bool)), this, SLOT(speed4Event(bool)));
		connect(mode1_, SIGNAL(toggled(bool)), this, SLOT(mode1Event(bool)));
		connect(mode2_, SIGNAL(toggled(bool)), this, SLOT(mode2Event(bool)));
		connect(mode3_, SIGNAL(toggled(bool)), this, SLOT(mode3Event(bool)));
		connect(denoiseButton_, SIGNAL(stateChanged(int)), this, SLOT(denoiseEvent(int)));
		connect(startFrame_, SIGNAL(valueChanged(int)), this, SLOT(startEvent(int)));
		connect(endFrame_, SIGNAL(valueChanged(int)), this, SLOT(endEvent(int)));
		connect(sppSpinbox_, SIGNAL(valueChanged(int)), this, SLOT(onSppChanged(int)));
		connect(bouncesSpinbox_, SIGNAL(valueChanged(int)), this, SLOT(onBouncesChanged(int)));
		connect(crfSpinbox, SIGNAL(valueChanged(double)), this, SLOT(onCrfChanged(double)));
		connect(recordButton_, SIGNAL(clicked(bool)), this, SLOT(recordEvent(bool)));
		connect(resolutionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onResolutionCombo(int)));
	}

	RecordDock::~RecordDock() noexcept
	{
	}

	void
	RecordDock::onSppChanged(int value)
	{
		profile_->offlineModule->spp = value;
	}

	void
	RecordDock::onBouncesChanged(int value)
	{
		profile_->offlineModule->bounces = value;
	}

	void
	RecordDock::onCrfChanged(double value)
	{
		profile_->encodeModule->crf = value;
	}

	void
	RecordDock::select1Event(bool checked)
	{
		if (checked)
			profile_->encodeModule->quality = VideoQuality::High;
	}

	void
	RecordDock::select2Event(bool checked)
	{
		if (checked)
			profile_->encodeModule->quality = VideoQuality::Medium;
	}

	void
	RecordDock::denoiseEvent(int checked)
	{
		if (checked == Qt::CheckState::Checked)
			profile_->recordModule->denoise = true;
		else
			profile_->recordModule->denoise = false;
	}

	void
	RecordDock::speed1Event(bool checked)
	{
		if (checked)
			profile_->playerModule->recordFps = 24;
	}

	void
	RecordDock::speed2Event(bool checked)
	{
		if (checked)
			profile_->playerModule->recordFps = 25;
	}

	void
	RecordDock::speed3Event(bool checked)
	{
		if (checked)
			profile_->playerModule->recordFps = 30;
	}

	void
	RecordDock::speed4Event(bool checked)
	{
		if (checked)
			profile_->playerModule->recordFps = 60;
	}

	void
	RecordDock::mode1Event(bool checked)
	{
		if (checked)
			profile_->encodeModule->encodeMode = EncodeMode::H264;
	}

	void
	RecordDock::mode2Event(bool checked)
	{
		if (checked)
			profile_->encodeModule->encodeMode = EncodeMode::H265;
	}

	void
	RecordDock::mode3Event(bool checked)
	{
		if (checked)
			profile_->encodeModule->encodeMode = EncodeMode::Frame;
	}

	void
	RecordDock::startEvent(int value)
	{
		profile_->playerModule->startFrame = value;
	}

	void
	RecordDock::endEvent(int value)
	{
		profile_->playerModule->endFrame = value;
	}

	void
	RecordDock::outputTypeEvent(int index)
	{
		if (index == 0)
			profile_->encodeModule->encodeMode = EncodeMode::H265;
		else if (index == 1)
			profile_->encodeModule->encodeMode = EncodeMode::H264;
		else if (index == 2)
			profile_->encodeModule->encodeMode = EncodeMode::Frame;
		else
			throw std::runtime_error("Unsupported EncodeMode");
	}

	void
	RecordDock::recordEvent(bool)
	{
		auto behaviour = behaviour_->getComponent<UnrealBehaviour>();
		if (behaviour)
		{
			if (!profile_->recordModule->active)
			{
				if (profile_->playerModule->endFrame <= profile_->playerModule->startFrame)
				{
					QMessageBox::warning(this, tr("Warning"), tr("Start frame must be less than end frame."));
					return;
				}

				QString fileName;
				if (profile_->encodeModule->encodeMode == EncodeMode::H264 || profile_->encodeModule->encodeMode == EncodeMode::H265)
					fileName = QFileDialog::getSaveFileName(this, tr("Save Video"), tr("New Video"), tr("MP4 Files (*.mp4)"));
				else if (profile_->encodeModule->encodeMode == EncodeMode::Frame)
					fileName = QFileDialog::getSaveFileName(this, tr("Save Image Sequence"), "", tr("PNG Files (*.png)"));
				else
					throw std::runtime_error("Unknown encode mode");

				if (!fileName.isEmpty())
				{
					if (!behaviour->startRecord(fileName.toStdWString()))
						QMessageBox::information(this, tr("Error"), tr("Failed to create file"));
				}
			}
			else
			{
				behaviour->stopRecord();
			}
		}
	}

	void
	RecordDock::onResolutionCombo(int index)
	{
		switch (resolutionCombo->currentIndex())
		{
		case 0: profile_->cameraModule->framebufferSize = octoon::math::uint2(720, 480); break;
		case 1: profile_->cameraModule->framebufferSize = octoon::math::uint2(800, 480); break;
		case 2: profile_->cameraModule->framebufferSize = octoon::math::uint2(1024, 576); break;
		case 3: profile_->cameraModule->framebufferSize = octoon::math::uint2(1280, 720); break;
		case 4: profile_->cameraModule->framebufferSize = octoon::math::uint2(1920, 1080); break;
		case 5: profile_->cameraModule->framebufferSize = octoon::math::uint2(540, 960); break;
		case 6: profile_->cameraModule->framebufferSize = octoon::math::uint2(720, 1280); break;
		case 7:  profile_->cameraModule->framebufferSize = octoon::math::uint2(1080, 1920); break;
		}
	}

	void
	RecordDock::showEvent(QShowEvent* event)
	{
		startFrame_->setValue(0);
		endFrame_->setValue(profile_->playerModule->endFrame);

		if (profile_->recordModule->active)
			recordButton_->setText(tr("Stop Render"));
		else
			recordButton_->setText(tr("Start Render"));

		auto quality = profile_->encodeModule->quality;
		if (quality == VideoQuality::High)
			select1_->click();
		else if (quality == VideoQuality::Medium)
			select2_->click();

		if (profile_->playerModule->recordFps == 24)
			speed1_->click();
		else if (profile_->playerModule->recordFps == 25)
			speed2_->click();
		else if (profile_->playerModule->recordFps == 30)
			speed3_->click();
		else if (profile_->playerModule->recordFps == 60)
			speed4_->click();

		if (profile_->encodeModule->encodeMode == EncodeMode::H264)
			mode1_->click();
		else if (profile_->encodeModule->encodeMode == EncodeMode::H265)
			mode2_->click();
		else if (profile_->encodeModule->encodeMode == EncodeMode::Frame)
			mode3_->click();

		auto& framebufferSize = profile_->cameraModule->framebufferSize.getValue();
		if (framebufferSize.x == 720 && framebufferSize.y == 480)
			resolutionCombo->setCurrentIndex(0);
		else if (framebufferSize.x == 800 && framebufferSize.y == 480)
			resolutionCombo->setCurrentIndex(1);
		else if (framebufferSize.x == 1024 && framebufferSize.y == 576)
			resolutionCombo->setCurrentIndex(2);
		else if (framebufferSize.x == 1280 && framebufferSize.y == 720)
			resolutionCombo->setCurrentIndex(3);
		else if (framebufferSize.x == 1920 && framebufferSize.y == 1080)
			resolutionCombo->setCurrentIndex(4);
		else if (framebufferSize.x == 540 && framebufferSize.y == 960)
			resolutionCombo->setCurrentIndex(5);
		else if (framebufferSize.x == 720 && framebufferSize.y == 1280)
			resolutionCombo->setCurrentIndex(6);
		else if (framebufferSize.x == 1080 && framebufferSize.y == 1920)
			resolutionCombo->setCurrentIndex(7);
		else
			throw std::runtime_error("resolution not found");

		denoiseButton_->setCheckState(profile_->recordModule->denoise ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
		sppSpinbox_->setValue(profile_->offlineModule->spp);
		crfSpinbox->doublespinbox_->setValue(profile_->encodeModule->crf);
		bouncesSpinbox_->setValue(profile_->offlineModule->bounces);
	}

	void
	RecordDock::closeEvent(QCloseEvent* event)
	{
		if (profile_->playerModule->isPlaying)
			event->ignore();
		else
			event->accept();
	}

	bool
	RecordDock::eventFilter(QObject* watched, QEvent* event)
	{
		if (event->type() != QEvent::Paint &&
			event->type() != QEvent::Resize)
		{
			if (profile_->playerModule->isPlaying)
			{
				return true;
			}
		}

		return QWidget::eventFilter(watched, event);
	}
}