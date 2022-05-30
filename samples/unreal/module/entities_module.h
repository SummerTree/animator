#ifndef UNREAL_ENTITIES_MODULE_H_
#define UNREAL_ENTITIES_MODULE_H_

#include <unreal_model.h>
#include <octoon/game_object.h>

namespace unreal
{
	class EntitesObject
	{
	public:
		std::string name;
		std::string filepath;
	};

	class EntitiesModule final : public UnrealModule
	{
	public:
		EntitiesModule() noexcept;
		virtual ~EntitiesModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(octoon::runtime::json& reader) noexcept override;
		virtual void save(octoon::runtime::json& reader) noexcept override;

	private:
		EntitiesModule(const EntitiesModule&) = delete;
		EntitiesModule& operator=(const EntitiesModule&) = delete;

	public:
		octoon::GameObjects objects;

		MutableLiveData<std::vector<EntitesObject>> entities;
	};
}

#endif