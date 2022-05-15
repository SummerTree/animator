#ifndef FLOWER_TITLE_WINDOW_H_
#define FLOWER_TITLE_WINDOW_H_

#include <qapplication.h>
#include <qlayout.h>
#include <qstyle>
#include <qdockwidget.h>
#include <qtoolButton.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <QtGui/qevent.h>
#include <qtoolbar.h>

#include "flower_behaviour.h"

namespace flower
{
	class ToplevelBar final : public QToolBar
	{
		Q_OBJECT
	public:
		ToplevelBar(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<flower::FlowerProfile>& profile) noexcept;
		~ToplevelBar() noexcept;

		void paintEvent(QPaintEvent* e) noexcept;

	public Q_SLOTS:
		void playEvent() noexcept;
		void resetEvent() noexcept;
		void leftEvent() noexcept;
		void rightEvent() noexcept;
		void volumeEvent() noexcept;

	public:
		bool playEnable_;
		bool volumeEnable_;

		QHBoxLayout layout_;

		QIcon playIcon_;
		QIcon playOnIcon_;
		QIcon leftIcon_;
		QIcon rightIcon_;
		QIcon resetIcon_;
		QIcon volumeOnIcon_;
		QIcon volumeOffIcon_;

		QToolButton playButton;
		QToolButton resetButton;
		QToolButton leftButton;
		QToolButton rightButton;
		QToolButton volumeButton;

		octoon::GameObjectPtr behaviour_;
		std::shared_ptr<flower::FlowerProfile> profile_;
	};
}

#endif