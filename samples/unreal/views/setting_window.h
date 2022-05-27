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
#include <qcombobox.h>
#include <qdialog.h>
#include <qpropertyanimation.h>
#include <qspinbox.h>

#include "title_bar.h"
#include "unreal_behaviour.h"

namespace unreal
{
	class SettingMainPlaneGeneral final : public QWidget
	{
		Q_OBJECT
	public:
		SettingMainPlaneGeneral(QWidget* parent, const std::shared_ptr<unreal::UnrealBehaviour>& behaviour);

		std::unique_ptr<QLabel> infoLabel;
		std::unique_ptr<QToolButton> infoButton;
		std::unique_ptr<QLabel> versionLabel;
		std::unique_ptr<QLabel> resetLabel;
		std::unique_ptr<QToolButton> resetButton;

	private:
		std::unique_ptr<QVBoxLayout> layout_;
	};

	class SettingMainPlaneInterface final : public QWidget
	{
		Q_OBJECT
	public:
		SettingMainPlaneInterface(QWidget* parent);

		std::unique_ptr<QLabel> renderLabel;
		std::unique_ptr<QLabel> resolutionLabel;
		std::unique_ptr<QComboBox> resolutionCombo;

	private:
		std::unique_ptr<QVBoxLayout> layout_;
	};

	class SettingMainPlaneGraphics final : public QWidget
	{
		Q_OBJECT
	public:
		SettingMainPlaneGraphics(QWidget* parent);
	};

	class SettingContextPlane final : public QWidget
	{
		Q_OBJECT
	public:
		SettingContextPlane(QWidget* parent, const std::shared_ptr<unreal::UnrealBehaviour>& behaviour) noexcept;
		~SettingContextPlane() noexcept;

	public Q_SLOTS:
		void valueChanged(int value);
		void itemClicked(QListWidgetItem* item);

		void onResetButton();
		void onResolutionCombo(int index);
		void onCheckVersion();

	private:
		bool m_sign;
		std::shared_ptr<unreal::UnrealBehaviour> behaviour_;

		std::unique_ptr<QListWidget> listWidget_;
		std::unique_ptr<QListWidgetItem> listWidgetItems_[3];
		std::unique_ptr<SettingMainPlaneGeneral> mainPlane_;
		std::unique_ptr<SettingMainPlaneInterface> mainPlane2_;
		std::unique_ptr<SettingMainPlaneGraphics> mainPlane3_;
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

	private:
		std::unique_ptr<QVBoxLayout> contextLayout_;
		std::unique_ptr<QPropertyAnimation> closeAnimation_;

		std::unique_ptr<TitleBar> settingTitleWindow_;
		std::unique_ptr<SettingContextPlane> settingContextPlane_;
	};
}

#endif