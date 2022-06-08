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

		virtual void load(nlohmann::json& reader, std::string_view path) noexcept override;
		virtual void save(nlohmann::json& writer, std::string_view path) noexcept override;

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