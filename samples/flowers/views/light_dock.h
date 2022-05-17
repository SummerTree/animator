#ifndef FLOWER_LIGHT_DOCK_H_
#define FLOWER_LIGHT_DOCK_H_

#include <qwidget.h>
#include <qlayout.h>
#include <qscrollarea.h>
#include <qlistwidget.h>

#include "flower_profile.h"
#include "color_dialog.h"

namespace flower
{
	class LightListWindow final : public QListWidget
	{
		Q_OBJECT
	public:
		LightListWindow() noexcept(false);
		~LightListWindow() noexcept;

		void mouseMoveEvent(QMouseEvent* event) override;
		void mousePressEvent(QMouseEvent* event) override;

	private:
		QPoint startPos;
	};

	class LightDock final : public QDockWidget
	{
		Q_OBJECT
	public:
		LightDock(const std::shared_ptr<flower::FlowerProfile>& profile);
		~LightDock();

		void repaint();

		void showEvent(QShowEvent* event) override;
		void resizeEvent(QResizeEvent* event) override;
		void closeEvent(QCloseEvent* event) override;

	public:
		LightListWindow* listWidget_;
		std::shared_ptr<flower::FlowerProfile> profile_;

		QWidget* mainWidget_;
		QVBoxLayout* mainLayout_;
	};
}

#endif