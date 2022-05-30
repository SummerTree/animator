#ifndef UNREAL_SOUND_MODULE_H_
#define UNREAL_SOUND_MODULE_H_

#include <unreal_model.h>
#include <octoon/game_object.h>

namespace unreal
{
	class SoundModule final : public UnrealModule
	{
	public:
		SoundModule() noexcept;
		virtual ~SoundModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(octoon::runtime::json& reader) noexcept override;
		virtual void save(octoon::runtime::json& reader) noexcept override;

	private:
		SoundModule(const SoundModule&) = delete;
		SoundModule& operator=(const SoundModule&) = delete;

	public:
		MutableLiveData<float> volume;
		MutableLiveData<std::string> filepath;
		MutableLiveData<octoon::GameObjectPtr> sound;
	};
}

#endif