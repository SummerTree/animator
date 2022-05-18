#ifndef FLOWER_SELECTOR_MODULE_H_
#define FLOWER_SELECTOR_MODULE_H_

#include <flower_model.h>
#include <optional>
#include <octoon/raycaster.h>

namespace flower
{
	class SelectorModule final : public FlowerModule
	{
	public:
		SelectorModule() noexcept;
		virtual ~SelectorModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(octoon::runtime::json& reader) noexcept override;
		virtual void save(octoon::runtime::json& reader) noexcept override;

	private:
		SelectorModule(const SelectorModule&) = delete;
		SelectorModule& operator=(const SelectorModule&) = delete;

	public:
		std::optional<octoon::RaycastHit> selectedItem_;
		std::optional<octoon::RaycastHit> selectedItemHover_;
	};
}

#endif