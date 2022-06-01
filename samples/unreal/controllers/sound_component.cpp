#include "sound_component.h"
#include "../unreal_profile.h"
#include "../unreal_behaviour.h"

namespace unreal
{
	SoundComponent::SoundComponent() noexcept
	{
	}

	SoundComponent::~SoundComponent() noexcept
	{
	}

	void
	SoundComponent::onInit() noexcept
	{
		this->getModel()->enable += [this](bool enable)
		{
			auto sound = this->getContext()->profile->soundModule->sound.getValue();
			if (sound)
				sound->setActive(enable);
		};

		this->getModel()->volume += [this](float value)
		{
			auto sound = this->getContext()->profile->soundModule->sound.getValue();
			if (sound)
			{
				auto audioSource = sound->getComponent<octoon::AudioSourceComponent>();
				audioSource->setVolume(value);
			}
		};

		this->getModel()->filepath += [this](const std::string& path)
		{
			if (!path.empty())
			{
				auto audio = octoon::GameObject::create();
				audio->setName(path);
				audio->addComponent<octoon::AudioSourceComponent>()->setAudioReader(octoon::AudioLoader::load(path));

				this->getContext()->profile->soundModule->sound = audio;
			}
			else
			{
				this->getContext()->profile->soundModule->sound = nullptr;
			}
		};
	}

	void
	SoundComponent::onEnable() noexcept
	{
	}

	void
	SoundComponent::onDisable() noexcept
	{
	}
}