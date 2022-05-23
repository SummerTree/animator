#ifndef UNREAL_THUMBNAIL_DOCK_H_
#define UNREAL_THUMBNAIL_DOCK_H_

#include <qapplication.h>
#include <qlayout.h>
#include <qstyle>
#include <qwidget>
#include <qtoolButton.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qscrollarea.h>
#include <QtGui/qevent.h>
#include <qdockwidget.h>
#include "unreal_profile.h"
#include "unreal_behaviour.h"
#include "setting_window.h"

namespace unreal
{
	class ThumbnailDock final : public QDockWidget
	{
		Q_OBJECT
	public:
		ThumbnailDock(const octoon::GameAppPtr& gameApp, const octoon::GameObjectPtr& behaviour, std::shared_ptr<UnrealProfile> profile) noexcept;
		~ThumbnailDock() noexcept;

		void closeEvent(QCloseEvent* event);

	private Q_SLOTS:
		void recordEvent() noexcept;
		void lightEvent() noexcept;
		void sunEvent() noexcept;
		void materialEvent() noexcept;
		void environmentEvent() noexcept;
		void cameraEvent() noexcept;
		void settingsEvent() noexcept;

	Q_SIGNALS:
		void sunSignal();
		void lightSignal();
		void recordSignal();
		void materialSignal();
		void environmentSignal();
		void cameraSignal();

	public:
		QToolButton* recordButton_;
		QToolButton* lightButton_;
		QToolButton* sunButton_;
		QToolButton* environmentButton_;
		QToolButton* materialButton_;
		QToolButton* cameraButton_;
		QToolButton* settingsButton_;
		QToolButton* motionButton_;

		octoon::GameObjectPtr behaviour_;
		std::shared_ptr<UnrealProfile> profile_;
	};
}

#endif