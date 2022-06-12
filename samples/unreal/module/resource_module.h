#ifndef UNREAL_RESOURCE_MODULE_H_
#define UNREAL_RESOURCE_MODULE_H_

#include <unreal_model.h>

namespace unreal
{
	class ResourceModule final : public UnrealModule
	{
	public:
		ResourceModule() noexcept;
		virtual ~ResourceModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(nlohmann::json& reader, std::shared_ptr<octoon::AssetBundle>& ab) noexcept override;
		virtual void save(nlohmann::json& writer, std::shared_ptr<octoon::AssetBundle>& ab) noexcept override;

	private:
		ResourceModule(const ResourceModule&) = delete;
		ResourceModule& operator=(const ResourceModule&) = delete;

	public:
		std::filesystem::path rootPath;
		std::filesystem::path cachePath;
	};
}

#endif