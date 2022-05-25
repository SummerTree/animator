#include "status_bar.h"

namespace unreal
{
	StatusBar::StatusBar(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile)
		: behaviour_(behaviour)
		, profile_(profile)
	{
		this->setObjectName("StatusBar");
		this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
		profile->playerModule->curTime += [this](float time) {
			this->updateEvent();
		};
		profile->playerModule->timeLength += [this](float time) {
			this->updateEvent();
		};
	}

	StatusBar::~StatusBar()
	{
	}

	void
	StatusBar::showEvent(QShowEvent* event) noexcept
	{
		this->updateEvent();
	}

	void
	StatusBar::updateEvent() noexcept
	{
		auto time = std::max<int>(1, std::round(profile_->playerModule->curTime * 30.0f));
		auto timeLength = std::max<int>(1, std::round(profile_->playerModule->timeLength * 30.0f));

		ulong ulHour = profile_->playerModule->estimatedTime / 3600;
		ulong ulMinute = (profile_->playerModule->estimatedTime - ulHour * 3600) / 60;
		ulong ulSecond = (profile_->playerModule->estimatedTime - ulHour * 3600 - ulMinute * 60);

		if (profile_->recordModule->active)
		{
			if (ulHour > 0)
				this->showMessage(tr("Animation Frame: %1 | Current Frame: %2 | Estimated Time: %3 Hour %4 Minute").arg(timeLength).arg(time).arg(ulHour).arg(ulMinute));
			else
				this->showMessage(tr("Animation Frame: %1 | Current Frame: %2 | Estimated Time: %3 Minute %4 Second").arg(timeLength).arg(time).arg(ulMinute).arg(ulSecond));
		}
		else
		{
			this->showMessage(tr("Animation Frame: %1 | Current Frame: %2").arg(timeLength).arg(time));
		}
	}
}