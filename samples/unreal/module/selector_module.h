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

		virtual void load(octoon::runtime::json& reader, std::string_view path) noexcept override;
		virtual void save(octoon::runtime::json& writer, std::string_view path) noexcept override;

	private:
		SelectorModule(const SelectorModule&) = delete;
		SelectorModule& operator=(const SelectorModule&) = delete;

	public:
		std::optional<octoon::RaycastHit> selectedItem_;
		std::optional<octoon::RaycastHit> selectedItemHover_;
	};
}

#endif