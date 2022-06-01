#ifndef UNREAL_TOOL_DOCK_H_
#define UNREAL_TOOL_DOCK_H_

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

namespace unreal
{
	class ToolDock final : public QDockWidget
	{
		Q_OBJECT
	public:
		ToolDock(const octoon::GameAppPtr& gameApp, const octoon::GameObjectPtr& behaviour, std::shared_ptr<UnrealProfile> profile) noexcept;
		~ToolDock() noexcept;

		bool eventFilter(QObject* watched, QEvent* event);

	private Q_SLOTS:
		void openEvent() noexcept;
		void importEvent() noexcept;
		void saveEvent() noexcept;
		void shotEvent() noexcept;
		void gpuEvent() noexcept;
		void audioEvent() noexcept;
		void cleanupEvent() noexcept;

	private:
		void update() noexcept;

	public:
		bool gpuEnable_;
		bool audioEnable_;
		bool hdrEnable_;

		QIcon gpuIcon_;
		QIcon gpuOnIcon_;
		QIcon audioIcon_;
		QIcon audioOnIcon_;

		QToolButton* openButton_;
		QToolButton* saveButton_;
		QToolButton* importButton_;
		QToolButton* shotButton_;
		QToolButton* gpuButton_;
		QToolButton* audioButton_;
		QToolButton* cleanupButton_;

		octoon::GameObjectPtr behaviour_;
		std::shared_ptr<UnrealProfile> profile_;
	};
}

#endif