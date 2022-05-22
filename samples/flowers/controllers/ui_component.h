#ifndef UNREAL_UI_COMPONENT_H_
#define UNREAL_UI_COMPONENT_H_

#include <octoon/game_object.h>
#include <unreal_component.h>
#include "module/record_module.h"

namespace unreal
{
	class UIComponent final : public UnrealComponent<RecordModule>
	{
	public:
		UIComponent() noexcept;
		~UIComponent() noexcept;

		void addMessageListener(const std::string& event, std::function<void(const std::any&)> listener) noexcept;
		void removeMessageListener(const std::string& event, std::function<void(const std::any&)> listener) noexcept;

		virtual void init(const std::shared_ptr<UnrealContext>& context, const std::shared_ptr<RecordModule>& model) noexcept override;

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(UIComponent);
		}

	private:
		void onEnable() noexcept override;
		void onDisable() noexcept override;

	private:
		octoon::GameObjectPtr main_;
	};
}

#endif