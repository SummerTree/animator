#ifndef UNREAL_RECORD_DOCK_H_
#define UNREAL_RECORD_DOCK_H_

#include "unreal_behaviour.h"
#include "spoiler.h"
#include <QComboBox>
#include <octoon/game_object.h>
#include <optional>
#include <qboxlayout.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qdialog.h>
#include <qdockwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qtimer.h>
#include <qtoolbutton.h>

namespace unreal
{
	class RecordDock final : public QDockWidget
	{
		Q_OBJECT
	  public:
		RecordDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile) noexcept;
		~RecordDock() noexcept;

		void showEvent(QShowEvent* event) override;
		void paintEvent(QPaintEvent* e) noexcept override;
		void closeEvent(QCloseEvent* event) override;

		bool eventFilter(QObject* watched, QEvent* event);

	  private Q_SLOTS:
		void recordEvent(bool);
		void select1Event(bool checked);
		void select2Event(bool checked);
		void denoiseEvent(int checked);
		void speed1Event(bool checked);
		void speed2Event(bool checked);
		void speed3Event(bool checked);
		void speed4Event(bool checked);
		void startEvent(int);
		void endEvent(int);
		void outputTypeEvent(int);
		void onSppChanged(int);
		void onBouncesChanged(int);
		void onCrfChanged(double);

	  public:
		QLabel* quality_;
		QLabel* videoRatio_;
		QLabel* frame_;
		QLabel* outputType_;
		QLabel* sppLabel;
		QLabel* bouncesLabel_;
		QLabel* crfLabel;
		QLabel* startLabel_;
		QLabel* endLabel_;
		QLabel* denoiseLabel_;

		QButtonGroup* group_;
		QButtonGroup* speedGroup_;

		QComboBox* outputTypeCombo_;

		QToolButton* select1_;
		QToolButton* select2_;
		QToolButton* speed1_;
		QToolButton* speed2_;
		QToolButton* speed3_;
		QToolButton* speed4_;
		QToolButton* recordButton_;
		QToolButton* markButton_;
		QCheckBox* denoiseButton_;

		QSpinBox* start_;
		QSpinBox* end_;
		QSpinBox* sppSpinbox_;
		QSpinBox* bouncesSpinbox_;

		QDoubleSpinBox* crfSpinbox;

		QHBoxLayout* videoRatioLayout_;
		QHBoxLayout* frameLayout_;
		QVBoxLayout* mainLayout_;

		QWidget* mainWidget_;

		Spoiler* markSpoiler_;
		Spoiler* videoSpoiler_;
		QScrollArea* contentWidgetArea_;

		octoon::GameObjectPtr behaviour_;
		std::shared_ptr<UnrealProfile> profile_;
	};
}

#endif