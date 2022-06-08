#ifndef UNREAL_ENTITIES_MODULE_H_
#define UNREAL_ENTITIES_MODULE_H_

#include <unreal_model.h>
#include <octoon/game_object.h>

namespace unreal
{
	class EntitiesModule final : public UnrealModule
	{
	public:
		EntitiesModule() noexcept;
		virtual ~EntitiesModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(nlohmann::json& reader, std::string_view path) noexcept(false) override;
		virtual void save(nlohmann::json& writer, std::string_view path) noexcept(false) override;

	private:
		EntitiesModule(const EntitiesModule&) = delete;
		EntitiesModule& operator=(const EntitiesModule&) = delete;

	public:
		MutableLiveData<octoon::GameObjects> objects;
	};
}

#endif