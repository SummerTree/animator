#ifndef FLOWER_VIEW_DOCK_H
#define FLOWER_VIEW_DOCK_H

#include <qdockwidget.h>
#include <qtimer.h>
#include <qdrag.h>

#include "flower_profile.h"
#include "flower_behaviour.h"

namespace flower
{
	class ViewDock final : public QDockWidget
	{
		Q_OBJECT
	public:
		ViewDock(const octoon::GameAppPtr& gameApp, const octoon::GameObjectPtr& behaviour, const std::shared_ptr<FlowerProfile>& profile) noexcept;
		~ViewDock() noexcept;

	private Q_SLOTS:
		void paintEvent(QPaintEvent* e) noexcept override;
		void resizeEvent(QResizeEvent* e) noexcept override;
		void mousePressEvent(QMouseEvent* event) noexcept override;
		void mouseMoveEvent(QMouseEvent* event) noexcept override;
		void mouseReleaseEvent(QMouseEvent* event) noexcept override;
		void mouseDoubleClickEvent(QMouseEvent* event) noexcept override;
		void keyPressEvent(QKeyEvent* event) noexcept override;
		void keyReleaseEvent(QKeyEvent* event) noexcept override;
		void wheelEvent(QWheelEvent* event) noexcept override;
		void dragEnterEvent(QDragEnterEvent* event) noexcept override;
		void dragMoveEvent(QDragMoveEvent *event) noexcept override;
		void dropEvent(QDropEvent* event) noexcept override;
		void showEvent(QShowEvent* event) noexcept override;

		virtual QPaintEngine* paintEngine() const noexcept override;

	private:
		octoon::input::InputKey::Code KeyCodetoInputKey(int key) noexcept;

	private:
		octoon::GameAppPtr gameApp_;
		octoon::GameObjectPtr behaviour_;

		std::shared_ptr<flower::FlowerProfile> profile_;
	};
}

#endif