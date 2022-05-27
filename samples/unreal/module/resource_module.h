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

		virtual void load(octoon::runtime::json& reader) noexcept override;
		virtual void save(octoon::runtime::json& reader) noexcept override;

	private:
		ResourceModule(const ResourceModule&) = delete;
		ResourceModule& operator=(const ResourceModule&) = delete;

	public:
		std::string rootPath;
		std::string hdriPath;
		std::string modelPath;
		std::string motionPath;
		std::string materialPath;

		MutableLiveData<nlohmann::json> hdriIndexList_;
		MutableLiveData<nlohmann::json> modelIndexList_;
		MutableLiveData<nlohmann::json> motionIndexList_;
	};
}

#endif