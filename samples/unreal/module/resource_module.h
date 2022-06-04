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

		virtual void load(octoon::runtime::json& reader, std::string_view path) noexcept override;
		virtual void save(octoon::runtime::json& writer, std::string_view path) noexcept override;

	private:
		ResourceModule(const ResourceModule&) = delete;
		ResourceModule& operator=(const ResourceModule&) = delete;

	public:
		std::string rootPath;
		std::string modelPath;
		std::u8string motionPath;
		std::string materialPath;
		std::string cachePath;
		std::string hdriPath;
	};
}

#endif