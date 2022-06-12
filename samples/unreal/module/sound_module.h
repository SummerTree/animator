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

		virtual void load(nlohmann::json& reader, std::shared_ptr<octoon::AssetBundle>& ab) noexcept override;
		virtual void save(nlohmann::json& writer, std::shared_ptr<octoon::AssetBundle>& ab) noexcept override;

	private:
		SoundModule(const SoundModule&) = delete;
		SoundModule& operator=(const SoundModule&) = delete;

	public:
		MutableLiveData<float> volume;
		MutableLiveData<std::filesystem::path> filepath;
		MutableLiveData<octoon::GameObjectPtr> sound;
	};
}

#endif