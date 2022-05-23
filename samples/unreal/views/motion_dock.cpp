#include "motion_dock.h"
#include <qapplication.h>
#include <qdrag.h>
#include <qevent.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qmimedata.h>

namespace unreal
{
	MotionDock::MotionDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile) noexcept
		: behaviour_(behaviour)
		, profile_(profile)
	{
		this->setObjectName("MotionDock");
		this->setWindowTitle(tr("Motion"));
	}

	MotionDock::~MotionDock() noexcept
	{
	}

	void
	MotionDock::recordEvent(bool)
	{
		
	}

	void
	MotionDock::showEvent(QShowEvent* event)
	{
	}

	void
	MotionDock::closeEvent(QCloseEvent* event)
	{
		if (profile_->playerModule->isPlaying)
			event->ignore();
		else
			event->accept();
	}

	void
	MotionDock::resizeEvent(QResizeEvent* e) noexcept
	{
	}

	void
	MotionDock::paintEvent(QPaintEvent* e) noexcept
	{
		// int left, top, bottom, right;
		// mainLayout_->getContentsMargins(&left, &top, &right, &bottom);
		// contentWidgetArea_->resize(contentWidgetArea_->size().width(), mainWidget_->size().height() - recordButton_->height() - (top + bottom) * 2);

		QDockWidget::paintEvent(e);
	}
}