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
		void recordEvent() noexcept;
		void shotEvent() noexcept;
		void gpuEvent() noexcept;
		void audioEvent() noexcept;
		void cleanupEvent() noexcept;
		void lightEvent() noexcept;
		void sunEvent() noexcept;
		void materialEvent() noexcept;
		void environmentEvent() noexcept;

	Q_SIGNALS:
		void sunSignal();
		void lightSignal();
		void recordSignal();
		void materialSignal();
		void environmentSignal();

	private:
		void paintEvent(QPaintEvent* e) noexcept override;

	public:
		bool gpuEnable_;
		bool recordEnable_;
		bool audioEnable_;
		bool hdrEnable_;
		bool sunEnable_;
		bool environmentEnable_;

		QIcon gpuIcon_;
		QIcon gpuOnIcon_;
		QIcon recordIcon_;
		QIcon audioIcon_;
		QIcon audioOnIcon_;
		QIcon sunIcon_;
		QIcon sunOnIcon_;
		QIcon environmentIcon_;
		QIcon environmentOnIcon_;

		QToolButton importButton;
		QToolButton saveButton;
		QToolButton recordButton;
		QToolButton shotButton;
		QToolButton gpuButton;
		QToolButton audioButton;
		QToolButton cleanupButton;
		QToolButton lightButton;
		QToolButton sunButton;
		QToolButton environmentButton;
		QToolButton materialButton;

		octoon::GameObjectPtr behaviour_;
		std::shared_ptr<flower::FlowerProfile> profile_;
	};
}

#endif