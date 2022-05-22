#include "ui_component.h"
#include "controllers/theme_manager.h"
#include "unreal_behaviour.h"

namespace unreal
{
	UIComponent::UIComponent() noexcept
	{
	}

	UIComponent::~UIComponent() noexcept
	{
	}

	void
	UIComponent::addMessageListener(const std::string& event, std::function<void(const std::any&)> listener) noexcept
	{
		main_->addMessageListener(event, listener);
	}
	
	void
	UIComponent::removeMessageListener(const std::string& event, std::function<void(const std::any&)> listener) noexcept
	{
		main_->removeMessageListener(event, listener);
	}

	void
	UIComponent::init(const std::shared_ptr<UnrealContext>& context, const std::shared_ptr<RecordModule>& model) noexcept
	{
		this->setContext(context);
		this->setModel(model);
		this->onEnable();
	}

	void
	UIComponent::onEnable() noexcept
	{
		if (!main_)
		{
			main_ = octoon::GameObject::create("MainView");
			main_->addComponent<ThemeManager>();

			this->getContext()->behaviour->addMessageListener("editor:camera:set", std::bind([&](const std::any& data) { main_->sendMessage("editor:camera:set", data); }, std::placeholders::_1));
		}
	}

	void
	UIComponent::onDisable() noexcept
	{
		main_.reset();
	}
}