#include "status_bar.h"

namespace flower
{
	StatusBar::StatusBar(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<FlowerProfile>& profile)
		: timer(this)
		, behaviour_(behaviour)
		, profile_(profile)
	{
		this->setObjectName("StatusBar");
		this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
		this->connect(&timer, SIGNAL(timeout()), this, SLOT(updateEvent()));

		timer.start();
	}

	StatusBar::~StatusBar()
	{
	}

	void
	StatusBar::updateEvent() noexcept
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
		{
			auto playerComponent = dynamic_cast<PlayerComponent*>(behaviour->getComponent<PlayerComponent>());
			auto animLength = std::max<int>(1, std::round(playerComponent->timeLength() * 30.0f));

			auto time = std::max<int>(0, std::round(behaviour->getProfile()->playerModule->curTime * 30.0f));

			this->showMessage(tr("Animation Frame: %1 | Current Frame: %2").arg(animLength).arg(time));
		}
	}
}