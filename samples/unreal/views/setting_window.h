#ifndef UNREAL_SETTING_WINDOW_H_
#define UNREAL_SETTING_WINDOW_H_

#include <qapplication.h>
#include <qlayout.h>
#include <qstyle>
#include <qwidget>
#include <qtoolButton.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <QtGui/qevent.h>
#include <qlistwidget.h>
#include <qscrollarea.h>
#include <QDialog>
#include <qpropertyanimation.h>
#include <qspinbox.h>
#include <QTranslator>

#include "../widgets/ulabel.h"
#include "../widgets/ucombobox.h"

#include "title_bar.h"
#include "unreal_behaviour.h"

namespace unreal
{
	class SettingMainPlaneGeneral final : public QWidget
	{
		Q_OBJECT
	public:
		SettingMainPlaneGeneral(QWidget* parent, const std::shared_ptr<unreal::UnrealBehaviour>& behaviour);

		bool eventFilter(QObject* watched, QEvent* event);
		void retranslate();

		ULabel* infoLabel;
		QToolButton* infoButton;
		ULabel* versionLabel;
		ULabel* resetLabel;
		QToolButton* resetButton;

	private:
		std::unique_ptr<QVBoxLayout> layout_;
	};

	class SettingMainPlaneInterface final : public QWidget
	{
		Q_OBJECT
	public:
		SettingMainPlaneInterface(QWidget* parent);

		bool eventFilter(QObject* watched, QEvent* event);
		void retranslate();

		QLabel* langLabel_;
		UComboBox* langCombo_;
		std::unique_ptr<QLabel> renderLabel;
		std::unique_ptr<QLabel> resolutionLabel;
		std::unique_ptr<UComboBox> resolutionCombo;

		std::vector<QString> languages_;
	private:
		std::unique_ptr<QVBoxLayout> layout_;
	};

	class SettingMainPlaneGraphics final : public QWidget
	{
		Q_OBJECT
	public:
		SettingMainPlaneGraphics(QWidget* parent);

		bool eventFilter(QObject* watched, QEvent* event);
		void retranslate();
	};

	class SettingContextPlane final : public QWidget
	{
		Q_OBJECT
	public:
		SettingContextPlane(QWidget* parent, const std::shared_ptr<unreal::UnrealBehaviour>& behaviour) noexcept(false);
		~SettingContextPlane() noexcept;
		
		bool eventFilter(QObject* watched, QEvent* event);
		void retranslate();

	public Q_SLOTS:
		void valueChanged(int value);
		void itemClicked(QListWidgetItem* item);

		void onResetButton();
		void onResolutionCombo(int index);
		void onCheckVersion();
		void onLangCombo(int index);
	
	Q_SIGNALS:
		void languageChangeSignal(QString filename);
	private:
		bool m_sign;
		std::shared_ptr<unreal::UnrealBehaviour> behaviour_;

		std::unique_ptr<QListWidget> listWidget_;
		std::unique_ptr<QListWidgetItem> listWidgetItems_[3];
		std::unique_ptr<SettingMainPlaneGeneral> mainPlaneGeneral_;
		std::unique_ptr<SettingMainPlaneInterface> mainPlaneInterface_;
		std::unique_ptr<SettingMainPlaneGraphics> mainPlaneGraphics_;
		std::unique_ptr<QVBoxLayout> gridLayout_;
		std::unique_ptr<QScrollArea> scrollArea_;
		std::unique_ptr<QWidget> scrollWidget_;
		std::unique_ptr<QHBoxLayout> layout_;
	};

	class SettingWindow final : public QDialog
	{
		Q_OBJECT
	public:
		SettingWindow(const std::shared_ptr<unreal::UnrealBehaviour>& behaviour) noexcept;
		~SettingWindow() noexcept;

	public Q_SLOTS:
		void closeEvent();

	Q_SIGNALS:
		void languageChangeSignal(QString filename);		

	private:
		std::unique_ptr<QVBoxLayout> contextLayout_;
		std::unique_ptr<QPropertyAnimation> closeAnimation_;

		std::unique_ptr<TitleBar> settingTitleWindow_;
		std::unique_ptr<SettingContextPlane> settingContextPlane_;
	};
}

#endif