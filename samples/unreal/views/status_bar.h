#ifndef UNREAL_STATUS_BAR_H_
#define UNREAL_STATUS_BAR_H_

#include <qstatusbar.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpushbutton.h>

#include "unreal_profile.h"
#include "unreal_behaviour.h"

namespace unreal
{
	class StatusBar final : public QStatusBar
	{
		Q_OBJECT
	public:
		StatusBar(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile);
		~StatusBar();

	private Q_SLOTS:
		void updateEvent() noexcept;

	private:
		QPushButton* info_;

		QTimer timer;
		octoon::GameObjectPtr behaviour_;

		std::shared_ptr<UnrealProfile> profile_;
	};
}

#endif