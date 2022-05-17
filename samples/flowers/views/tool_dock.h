#ifndef FLOWER_TOOL_DOCK_H_
#define FLOWER_TOOL_DOCK_H_

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
#include "flower_profile.h"
#include "flower_behaviour.h"

namespace flower
{
	class ToolDock final : public QDockWidget
	{
		Q_OBJECT
	public:
		ToolDock(const octoon::GameAppPtr& gameApp, const octoon::GameObjectPtr& behaviour, std::shared_ptr<FlowerProfile> profile) noexcept;
		~ToolDock() noexcept;

	private Q_SLOTS:
		void importEvent() noexcept;
		void shotEvent() noexcept;
		void gpuEvent() noexcept;
		void audioEvent() noexcept;
		void cleanupEvent() noexcept;

	private:
		void paintEvent(QPaintEvent* e) noexcept override;

	public:
		bool gpuEnable_;
		bool audioEnable_;
		bool hdrEnable_;

		QIcon gpuIcon_;
		QIcon gpuOnIcon_;
		QIcon audioIcon_;
		QIcon audioOnIcon_;

		QToolButton* importButton_;
		QToolButton* saveButton_;
		QToolButton* shotButton_;
		QToolButton* gpuButton_;
		QToolButton* audioButton_;
		QToolButton* cleanupButton_;

		octoon::GameObjectPtr behaviour_;
		std::shared_ptr<FlowerProfile> profile_;
	};
}

#endif