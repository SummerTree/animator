#include "record_dock.h"
#include <qapplication.h>
#include <qdrag.h>
#include <qevent.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qmimedata.h>

namespace unreal
{
	class SpinBox final : public QSpinBox
	{
	  public:
		void
		focusInEvent(QFocusEvent* event) override
		{
			this->grabKeyboard();
			QSpinBox::focusInEvent(event);
		}

		void
		focusOutEvent(QFocusEvent* event) override
		{
			this->releaseKeyboard();
			QSpinBox::focusOutEvent(event);
		}
	};

	class DoubleSpinBox final : public QDoubleSpinBox
	{
	  public:
		void
		focusInEvent(QFocusEvent* event) override
		{
			this->grabKeyboard();
			QDoubleSpinBox::focusInEvent(event);
		}

		void
		focusOutEvent(QFocusEvent* event) override
		{
			this->releaseKeyboard();
			QDoubleSpinBox::focusOutEvent(event);
		}
	};

	RecordDock::RecordDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile) noexcept
		: behaviour_(behaviour)
		, profile_(profile)
	{
		this->setObjectName("RecordDock");
		this->setWindowTitle(tr("Record"));

		markButton_ = new QToolButton();
		markButton_->setObjectName("mark");
		markButton_->setIcon(QIcon::fromTheme(":res/icons/append2.png"));
		markButton_->setIconSize(QSize(139, 143));
		markButton_->installEventFilter(this);

		quality_ = new QLabel();
		quality_->setText(tr("Render Quality"));

		select1_ = new QToolButton();
		select1_->setObjectName("select1");
		select1_->setText(tr("Ultra Render"));
		select1_->setCheckable(true);
		select1_->click();
		select1_->installEventFilter(this);

		select2_ = new QToolButton();
		select2_->setObjectName("select2");
		select2_->setText(tr("Fast Render"));
		select2_->setCheckable(true);
		select2_->installEventFilter(this);

		group_ = new QButtonGroup();
		group_->addButton(select1_, 0);
		group_->addButton(select2_, 1);

		videoRatio_ = new QLabel();
		videoRatio_->setText(tr("Frame Per Second"));

		speed1_ = new QToolButton();
		speed1_->setObjectName("speed1");
		speed1_->setText(tr("24"));
		speed1_->setCheckable(true);
		speed1_->click();
		speed1_->installEventFilter(this);

		speed2_ = new QToolButton();
		speed2_->setObjectName("speed2");
		speed2_->setText(tr("25"));
		speed2_->setCheckable(true);
		speed2_->installEventFilter(this);

		speed3_ = new QToolButton();
		speed3_->setObjectName("speed3");
		speed3_->setText(tr("30"));
		speed3_->setCheckable(true);
		speed3_->installEventFilter(this);

		speed4_ = new QToolButton();
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
		outputType_ = new QLabel();
		outputType_->setText(tr("Output Type"));

		outputTypeCombo_ = new QComboBox();
		outputTypeCombo_->addItem(tr("H265"));
		outputTypeCombo_->addItem(tr("H264"));
		outputTypeCombo_->addItem(tr("Frame Sequence"));
		outputTypeCombo_->setStyleSheet("color:white");
		outputTypeCombo_->installEventFilter(this);

		frame_ = new QLabel();
		frame_->setText(tr("Play:"));

		startLabel_ = new QLabel();
		startLabel_->setText(tr("Start"));

		endLabel_ = new QLabel();
		endLabel_->setText(tr("- End"));

		startFrame_ = new SpinBox();
		startFrame_->setObjectName("start");
		startFrame_->setAlignment(Qt::AlignRight);
		startFrame_->setMinimum(0);
		startFrame_->setMaximum(99999);
		startFrame_->installEventFilter(this);

		endFrame_ = new SpinBox();
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

		bouncesLabel_ = new QLabel();
		bouncesLabel_->setText(tr("Recursion depth per pixel:"));
		bouncesLabel_->setStyleSheet("color: rgb(200,200,200);");

		bouncesSpinbox_ = new SpinBox();
		bouncesSpinbox_->setMinimum(1);
		bouncesSpinbox_->setMaximum(32);
		bouncesSpinbox_->setValue(0);
		bouncesSpinbox_->setAlignment(Qt::AlignRight);
		bouncesSpinbox_->setFixedWidth(100);
		bouncesSpinbox_->installEventFilter(this);

		sppLabel = new QLabel();
		sppLabel->setText(tr("Sample number per pixel:"));
		sppLabel->setStyleSheet("color: rgb(200,200,200);");

		sppSpinbox_ = new SpinBox();
		sppSpinbox_->setMinimum(1);
		sppSpinbox_->setMaximum(9999);
		sppSpinbox_->setValue(0);
		sppSpinbox_->setAlignment(Qt::AlignRight);
		sppSpinbox_->setFixedWidth(100);
		sppSpinbox_->installEventFilter(this);

		crfSpinbox = new DoubleSpinBox();
		crfSpinbox->setMinimum(0);
		crfSpinbox->setMaximum(63.0);
		crfSpinbox->setValue(0);
		crfSpinbox->setAlignment(Qt::AlignRight);
		crfSpinbox->setFixedWidth(100);
		crfSpinbox->installEventFilter(this);

		crfLabel = new QLabel();
		crfLabel->setText(tr("Constant Rate Factor (CRF):"));
		crfLabel->setStyleSheet("color: rgb(200,200,200);");

		frameLayout_ = new QHBoxLayout();
		frameLayout_->addSpacing(20);
		frameLayout_->addWidget(startLabel_, 0, Qt::AlignLeft);
		frameLayout_->addWidget(startFrame_, 0, Qt::AlignLeft);
		frameLayout_->addWidget(endLabel_, 0, Qt::AlignLeft);
		frameLayout_->addWidget(endFrame_, 0, Qt::AlignLeft);
		frameLayout_->addStretch();

		recordButton_ = new QToolButton();
		recordButton_->setObjectName("render");
		recordButton_->setText(tr("Start Render"));
		recordButton_->setContentsMargins(0, 0, 0, 0);

		videoRatioLayout_ = new QHBoxLayout();
		videoRatioLayout_->addStretch();
		videoRatioLayout_->addWidget(speed1_, 0, Qt::AlignRight);
		videoRatioLayout_->addWidget(speed2_, 0, Qt::AlignVCenter);
		videoRatioLayout_->addWidget(speed3_, 0, Qt::AlignVCenter);
		videoRatioLayout_->addWidget(speed4_, 0, Qt::AlignLeft);
		videoRatioLayout_->addStretch();
		videoRatioLayout_->setContentsMargins(0, 0, 0, 0);

		auto selectLayout_ = new QHBoxLayout();
		selectLayout_->addWidget(select1_, 0, Qt::AlignLeft);
		selectLayout_->addWidget(select2_, 0, Qt::AlignLeft);
		selectLayout_->setContentsMargins(0, 0, 0, 0);

		auto videoLayout = new QVBoxLayout;
		videoLayout->addWidget(quality_);
		videoLayout->addLayout(selectLayout_);
		videoLayout->addSpacing(10);
		videoLayout->addWidget(videoRatio_);
		videoLayout->addLayout(videoRatioLayout_);
		videoLayout->addSpacing(10);
		videoLayout->addWidget(frame_);
		videoLayout->addLayout(frameLayout_);
		videoLayout->addSpacing(10);
		videoLayout->addWidget(outputType_);
		videoLayout->addWidget(outputTypeCombo_);
		videoLayout->addSpacing(10);
		videoLayout->addLayout(denoiseLayout_);
		videoLayout->addWidget(sppLabel);
		videoLayout->addWidget(sppSpinbox_);
		videoLayout->addSpacing(10);
		videoLayout->addWidget(bouncesLabel_);
		videoLayout->addWidget(bouncesSpinbox_);
		videoLayout->addSpacing(10);
		videoLayout->addWidget(crfLabel);
		videoLayout->addWidget(crfSpinbox);
		videoLayout->setContentsMargins(20, 10, 0, 0);

		auto markLayout = new QVBoxLayout;
		markLayout->addWidget(markButton_, 0, Qt::AlignCenter);

		markSpoiler_ = new Spoiler(tr("Watermark"));
		markSpoiler_->setContentLayout(*markLayout);

		videoSpoiler_ = new Spoiler(tr("Render Settings"));
		videoSpoiler_->setContentLayout(*videoLayout);
		videoSpoiler_->toggleButton.click();

		auto contentLayout = new QVBoxLayout();
		contentLayout->addWidget(videoSpoiler_);
		contentLayout->addWidget(markSpoiler_);
		contentLayout->addStretch();

		auto contentWidget = new QWidget;
		contentWidget->setLayout(contentLayout);

		contentWidgetArea_ = new QScrollArea();
		contentWidgetArea_->setWidget(contentWidget);
		contentWidgetArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		contentWidgetArea_->setWidgetResizable(true);

		mainLayout_ = new QVBoxLayout();
		mainLayout_->addWidget(contentWidgetArea_);
		mainLayout_->addStretch();
		mainLayout_->addWidget(recordButton_, 0, Qt::AlignCenter);
		mainLayout_->setContentsMargins(0, 0, 0, 10);

		mainWidget_ = new QWidget;
		mainWidget_->setLayout(mainLayout_);

		this->setWidget(mainWidget_);

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
			crfSpinbox->setValue(value);
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
		connect(denoiseButton_, SIGNAL(stateChanged(int)), this, SLOT(denoiseEvent(int)));
		connect(startFrame_, SIGNAL(valueChanged(int)), this, SLOT(startEvent(int)));
		connect(endFrame_, SIGNAL(valueChanged(int)), this, SLOT(endEvent(int)));
		connect(sppSpinbox_, SIGNAL(valueChanged(int)), this, SLOT(onSppChanged(int)));
		connect(bouncesSpinbox_, SIGNAL(valueChanged(int)), this, SLOT(onBouncesChanged(int)));
		connect(crfSpinbox, SIGNAL(valueChanged(double)), this, SLOT(onCrfChanged(double)));
		connect(recordButton_, SIGNAL(clicked(bool)), this, SLOT(recordEvent(bool)));
		connect(outputTypeCombo_, SIGNAL(currentIndexChanged(int)), this, SLOT(outputTypeEvent(int)));
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
					fileName = QFileDialog::getSaveFileName(this, tr("Save Video"), "", tr("MP4 Files (*.mp4)"));
				else if (profile_->encodeModule->encodeMode == EncodeMode::Frame)
					fileName = QFileDialog::getSaveFileName(this, tr("Save Image Sequence"), "", tr("PNG Files (*.png)"));
				else
					throw std::runtime_error("Unknown encode mode");

				if (!fileName.isEmpty())
				{
					if (!behaviour->startRecord(fileName.toUtf8().data()))
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
	RecordDock::paintEvent(QPaintEvent* e) noexcept
	{
		int left, top, bottom, right;
		mainLayout_->getContentsMargins(&left, &top, &right, &bottom);
		contentWidgetArea_->resize(contentWidgetArea_->size().width(), mainWidget_->size().height() - recordButton_->height() - (top + bottom) * 2);

		QDockWidget::paintEvent(e);
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

		denoiseButton_->setCheckState(profile_->recordModule->denoise ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
		sppSpinbox_->setValue(profile_->offlineModule->spp);
		crfSpinbox->setValue(profile_->encodeModule->crf);
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