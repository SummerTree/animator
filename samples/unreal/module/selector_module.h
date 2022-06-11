#ifndef UNREAL_SELECTOR_MODULE_H_
#define UNREAL_SELECTOR_MODULE_H_

#include <unreal_model.h>
#include <optional>
#include <octoon/raycaster.h>

namespace unreal
{
	class SelectorModule final : public UnrealModule
	{
	public:
		SelectorModule() noexcept;
		virtual ~SelectorModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(nlohmann::json& reader, std::shared_ptr<octoon::AssetBundle>& ab) noexcept override;
		virtual void save(nlohmann::json& writer, std::shared_ptr<octoon::AssetBundle>& ab) noexcept override;

	private:
		SelectorModule(const SelectorModule&) = delete;
		SelectorModule& operator=(const SelectorModule&) = delete;

	public:
		MutableLiveData<std::optional<octoon::RaycastHit>> selectedItem_;
		MutableLiveData<std::optional<octoon::RaycastHit>> selectedItemHover_;
	};
}

#endif