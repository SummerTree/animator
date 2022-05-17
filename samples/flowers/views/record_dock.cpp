#include "record_dock.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <qdrag.h>
#include <qmimedata.h>
#include <qapplication.h>

namespace flower
{
	class SpinBox final : public QSpinBox
	{
	public:
		void focusInEvent(QFocusEvent* event) override
		{
			this->grabKeyboard();
			QSpinBox::focusInEvent(event);
		}

		void focusOutEvent(QFocusEvent* event) override
		{
			this->releaseKeyboard();
			QSpinBox::focusOutEvent(event);
		}
	};

	class DoubleSpinBox final : public QDoubleSpinBox
	{
	public:
		void focusInEvent(QFocusEvent* event) override
		{
			this->grabKeyboard();
			QDoubleSpinBox::focusInEvent(event);
		}

		void focusOutEvent(QFocusEvent* event) override
		{
			this->releaseKeyboard();
			QDoubleSpinBox::focusOutEvent(event);
		}
	};

	RecordDock::RecordDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<FlowerProfile>& profile) noexcept
		: behaviour_(behaviour)
	{
		this->setObjectName("RecordDock");
		this->setWindowTitle(tr("Record"));

		markButton_ = new QToolButton();
		markButton_->setObjectName("mark");
		markButton_->setIcon(QIcon::fromTheme(":res/icons/append2.png"));
		markButton_->setIconSize(QSize(139, 143));

		quality_ = new QLabel();
		quality_->setText(tr("Render Quality"));

		select1_ = new QToolButton();
		select1_->setObjectName("select1");
		select1_->setText(tr("Ultra Render"));
		select1_->setCheckable(true);
		select1_->click();

		select2_ = new QToolButton();
		select2_->setObjectName("select2");
		select2_->setText(tr("Fast Render"));
		select2_->setCheckable(true);

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

		speed2_ = new QToolButton();
		speed2_->setObjectName("speed2");
		speed2_->setText(tr("25"));
		speed2_->setCheckable(true);

		speed3_ = new QToolButton();
		speed3_->setObjectName("speed3");
		speed3_->setText(tr("30"));
		speed3_->setCheckable(true);

		speed4_ = new QToolButton();
		speed4_->setObjectName("speed4");
		speed4_->setText(tr("60"));
		speed4_->setCheckable(true);

		speedGroup_ = new QButtonGroup();
		speedGroup_->addButton(speed1_, 0);
		speedGroup_->addButton(speed2_, 1);
		speedGroup_->addButton(speed3_, 2);
		speedGroup_->addButton(speed4_, 3);

		frame_ = new QLabel();
		frame_->setText(tr("Play:"));

		startLabel_ = new QLabel();
		startLabel_->setText(tr("Start"));

		endLabel_ = new QLabel();
		endLabel_->setText(tr("- End"));

		start_ = new SpinBox();
		start_->setObjectName("start");
		start_->setAlignment(Qt::AlignRight);
		start_->setMinimum(0);
		start_->setMaximum(99999);

		end_ = new SpinBox();
		end_->setObjectName("end");
		end_->setAlignment(Qt::AlignRight);
		end_->setMinimum(0);
		end_->setMaximum(99999);

		bouncesLabel_ = new QLabel();
		bouncesLabel_->setText(tr("Recursion depth per pixel:"));
		bouncesLabel_->setStyleSheet("color: rgb(200,200,200);");

		bouncesSpinbox_ = new SpinBox();
		bouncesSpinbox_->setMinimum(1);
		bouncesSpinbox_->setMaximum(32);
		bouncesSpinbox_->setValue(0);
		bouncesSpinbox_->setAlignment(Qt::AlignRight);
		bouncesSpinbox_->setFixedWidth(100);

		sppLabel = new QLabel();
		sppLabel->setText(tr("Sample number per pixel:"));
		sppLabel->setStyleSheet("color: rgb(200,200,200);");

		sppSpinbox_ = new SpinBox();
		sppSpinbox_->setMinimum(1);
		sppSpinbox_->setMaximum(9999);
		sppSpinbox_->setValue(0);
		sppSpinbox_->setAlignment(Qt::AlignRight);
		sppSpinbox_->setFixedWidth(100);

		crfSpinbox = new DoubleSpinBox();
		crfSpinbox->setMinimum(0);
		crfSpinbox->setMaximum(63.0);
		crfSpinbox->setValue(0);
		crfSpinbox->setAlignment(Qt::AlignRight);
		crfSpinbox->setFixedWidth(100);

		crfLabel = new QLabel();
		crfLabel->setText(tr("Constant Rate Factor (CRF):"));
		crfLabel->setStyleSheet("color: rgb(200,200,200);");

		frameLayout_ = new QHBoxLayout();
		frameLayout_->addSpacing(20);
		frameLayout_->addWidget(startLabel_, 0, Qt::AlignLeft);
		frameLayout_->addWidget(start_, 0, Qt::AlignLeft);
		frameLayout_->addWidget(endLabel_, 0, Qt::AlignLeft);
		frameLayout_->addWidget(end_, 0, Qt::AlignLeft);
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
		
		connect(recordButton_, SIGNAL(clicked()), this, SLOT(clickEvent()));
		connect(select1_, SIGNAL(toggled(bool)), this, SLOT(select1Event(bool)));
		connect(select2_, SIGNAL(toggled(bool)), this, SLOT(select2Event(bool)));
		connect(speed1_, SIGNAL(toggled(bool)), this, SLOT(speed1Event(bool)));
		connect(speed2_, SIGNAL(toggled(bool)), this, SLOT(speed2Event(bool)));
		connect(speed3_, SIGNAL(toggled(bool)), this, SLOT(speed3Event(bool)));
		connect(speed4_, SIGNAL(toggled(bool)), this, SLOT(speed4Event(bool)));
		connect(start_, SIGNAL(valueChanged(int)), this, SLOT(startEvent(int)));
		connect(end_, SIGNAL(valueChanged(int)), this, SLOT(endEvent(int)));
		connect(sppSpinbox_, SIGNAL(valueChanged(int)), this, SLOT(onSppChanged(int)));
		connect(bouncesSpinbox_, SIGNAL(valueChanged(int)), this, SLOT(onBouncesChanged(int)));
		connect(crfSpinbox, SIGNAL(valueChanged(double)), this, SLOT(onCrfChanged(double)));
	}

	RecordDock::~RecordDock() noexcept
	{
	}

	void
	RecordDock::startRecord(QString fileName)
	{
		auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
		if (behaviour)
		{
			if (behaviour->startRecord(fileName.toUtf8().data()))
			{
				start_->setEnabled(false);
				end_->setEnabled(false);
				recordButton_->setText(tr("Stop Render"));
			}
			else
			{
				QMessageBox msg(this);
				msg.setWindowTitle(tr("Error"));
				msg.setText(tr("Failed to create file"));
				msg.setIcon(QMessageBox::Information);
				msg.setStandardButtons(QMessageBox::Ok);

				msg.exec();
			}
		}
	}

	void
	RecordDock::stopRecord()
	{
		auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
		if (behaviour)
		{
			start_->setEnabled(true);
			end_->setEnabled(true);
			recordButton_->setText(tr("Start Render"));
			behaviour->stopRecord();
		}
	}

	void
	RecordDock::onSppChanged(int value)
	{
		auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
		if (behaviour)
			behaviour->getProfile()->playerModule->spp = value;
	}

	void
	RecordDock::onBouncesChanged(int value)
	{
		auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
		if (behaviour)
			behaviour->getComponent<OfflineComponent>()->setMaxBounces(value);
	}

	void
	RecordDock::onCrfChanged(double value)
	{
		auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
		if (behaviour)
			behaviour->getProfile()->encodeModule->crf = value;
	}

	void
	RecordDock::showEvent(QShowEvent* event)
	{
		this->repaint();
	}

	void
	RecordDock::resizeEvent(QResizeEvent* e) noexcept
	{
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
	RecordDock::clickEvent()
	{
		VideoQuality quality = VideoQuality::Medium;
		if (select1_->isChecked())
			quality = VideoQuality::High;
		if (select2_->isChecked())
			quality = VideoQuality::Medium;

		auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
		if (behaviour)
		{
			behaviour->getProfile()->encodeModule->setVideoQuality(quality);

			if (recordButton_->text() != tr("Stop Render"))
			{
				QString fileName = QFileDialog::getSaveFileName(this, tr("Save Video"), "", tr("MP4 Files (*.mp4)"));
				if (!fileName.isEmpty())
					this->startRecord(fileName);
			}
			else
			{
				this->stopRecord();
			}
		}
	}

	void
	RecordDock::select1Event(bool checked)
	{
		this->update();
	}
	
	void
	RecordDock::select2Event(bool checked)
	{
		this->update();
	}

	void
	RecordDock::speed1Event(bool checked)
	{
		if (checked)
		{
			auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
			if (behaviour)
				behaviour->getProfile()->playerModule->recordFps = 24;

			this->update();
		}
	}

	void
	RecordDock::speed2Event(bool checked)
	{
		if (checked)
		{
			auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
			if (behaviour)
				behaviour->getProfile()->playerModule->recordFps = 25;

			this->update();
		}
	}

	void
	RecordDock::speed3Event(bool checked)
	{
		if (checked)
		{
			auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
			if (behaviour)
				behaviour->getProfile()->playerModule->recordFps = 30;

			this->update();
		}
	}

	void
	RecordDock::speed4Event(bool checked)
	{
		if (checked)
		{
			auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
			if (behaviour)
				behaviour->getProfile()->playerModule->recordFps = 60;

			this->update();
		}
	}

	void
	RecordDock::startEvent(int value)
	{
		auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
		if (behaviour)
		{
			behaviour->getProfile()->playerModule->startFrame = value;
			this->update();
		}
	}

	void
	RecordDock::endEvent(int value)
	{
		auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
		if (behaviour)
		{
			behaviour->getProfile()->playerModule->endFrame = value;
			this->update();
		}
	}

	void 
	RecordDock::repaint()
	{
		auto behaviour = behaviour_->getComponent<FlowerBehaviour>();
		if (behaviour)
		{
			auto playerComponent = behaviour->getComponent<PlayerComponent>();
			int timeLength = (int)std::round(playerComponent->timeLength() * 30);

			start_->setValue(0);
			end_->setValue(timeLength);

			auto profile = behaviour->getProfile();
			if (profile->playerModule->recordFps == 24)
				speed1_->click();
			else if (profile->playerModule->recordFps == 25)
				speed2_->click();
			else if (profile->playerModule->recordFps == 30)
				speed3_->click();
			else if (profile->playerModule->recordFps == 60)
				speed4_->click();

			sppSpinbox_->setValue(profile->playerModule->spp);
			crfSpinbox->setValue(profile->encodeModule->crf);
			bouncesSpinbox_->setValue(profile->offlineModule->bounces);
		}
	}
}