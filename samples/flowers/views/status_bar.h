#ifndef FLOWER_STATUS_BAR_H_
#define FLOWER_STATUS_BAR_H_

#include <qstatusbar.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpushbutton.h>

#include "flower_profile.h"
#include "flower_behaviour.h"

namespace flower
{
	class StatusBar final : public QStatusBar
	{
		Q_OBJECT
	public:
		StatusBar(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<FlowerProfile>& profile);
		~StatusBar();

	private Q_SLOTS:
		void updateEvent() noexcept;

	private:
		QPushButton* info_;

		QTimer timer;
		octoon::GameObjectPtr behaviour_;

		std::shared_ptr<FlowerProfile> profile_;
	};
}

#endif