#ifndef UNREAL_RECORD_DOCK_H_
#define UNREAL_RECORD_DOCK_H_

#include "unreal_behaviour.h"
#include "../widgets/spoiler.h"
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

#include "../widgets/uspinbox.h"
#include "../widgets/udoublespinbox.h"
#include "../widgets/ulabel.h"
#include "../widgets/ucombobox.h"
#include "../widgets/udockwidget.h"
#include "../widgets/upushbutton.h"
#include "../widgets/uspinline.h"
#include "../widgets/udoublespinline.h"


namespace unreal
{
	class RecordDock final : public UDockWidget
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
		ULabel* quality_;
		ULabel* videoRatio_;
		ULabel* frame_;
		ULabel* outputType_;
		ULabel* sppLabel;
		ULabel* bouncesLabel_;
		ULabel* crfLabel;
		ULabel* startLabel_;
		ULabel* endLabel_;
		ULabel* denoiseLabel_;

		QButtonGroup* group_;
		QButtonGroup* speedGroup_;

		UComboBox* outputTypeCombo_;

		UPushButton* select1_;
		UPushButton* select2_;
		UPushButton* speed1_;
		UPushButton* speed2_;
		UPushButton* speed3_;
		UPushButton* speed4_;
		QPushButton* recordButton_;
		UPushButton* markButton_;
		QCheckBox* denoiseButton_;

		USpinBox* startFrame_;
		USpinBox* endFrame_;
		USpinLine* sppSpinbox_;
		USpinLine* bouncesSpinbox_;

		UDoubleSpinLine* crfSpinbox;

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