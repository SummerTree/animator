#include "environment_component.h"
#include "../unreal_profile.h"
#include "../unreal_behaviour.h"

#include <octoon/PMREM_loader.h>
#include <octoon/material/mesh_basic_material.h>

namespace unreal
{
	EnvironmentComponent::EnvironmentComponent() noexcept
	{
	}

	EnvironmentComponent::~EnvironmentComponent() noexcept
	{
	}

	void
	EnvironmentComponent::onInit() noexcept
	{
		this->getModel()->showBackground += [this](bool value)
		{
			auto environmentLight = this->getContext()->profile->entitiesModule->environmentLight;
			if (environmentLight)
			{
				auto enviromentComponent = environmentLight->getComponent<octoon::EnvironmentLightComponent>();
				if (enviromentComponent)
					enviromentComponent->setShowBackground(value);

				auto meshRenderer = environmentLight->getComponent<octoon::MeshRendererComponent>();
				if (meshRenderer)
					meshRenderer->setActive(value);
			}
		};

		this->getModel()->intensity += [this](float value)
		{
			auto& model = this->getModel();
			auto& profile = this->getContext()->profile;

			auto environmentLight = profile->entitiesModule->environmentLight;
			if (environmentLight)
			{
				auto enviromentComponent = environmentLight->getComponent<octoon::EnvironmentLightComponent>();
				if (enviromentComponent)
					enviromentComponent->setIntensity(value);

				auto meshRenderer = environmentLight->getComponent<octoon::MeshRendererComponent>();
				if (meshRenderer)
				{
					
					auto basicMaterial = meshRenderer->getMaterial()->downcast<octoon::MeshBasicMaterial>();
					basicMaterial->setColor(octoon::math::srgb2linear<float>(model->color.getValue()) * model->intensity);
				}
			}
		};

		this->getModel()->color += [this](const octoon::math::float3& value)
		{
			auto& model = this->getModel();
			auto& profile = this->getContext()->profile;

			auto environmentLight = profile->entitiesModule->environmentLight;
			if (environmentLight)
			{
				auto srgb = octoon::math::srgb2linear<float>(value);

				auto environmentComponent = environmentLight->getComponent<octoon::EnvironmentLightComponent>();
				if (environmentComponent)
					environmentComponent->setColor(srgb);

				auto meshRenderer = environmentLight->getComponent<octoon::MeshRendererComponent>();
				if (meshRenderer)
				{
					auto basicMaterial = meshRenderer->getMaterial()->downcast<octoon::MeshBasicMaterial>();
					basicMaterial->setColor(srgb * model->intensity);
				}
			}
		};

		this->getModel()->offset += [this](const octoon::math::float2& offset)
		{
			auto& model = this->getModel();
			auto& profile = this->getContext()->profile;

			auto environmentLight = profile->entitiesModule->environmentLight;
			if (environmentLight)
			{
				auto environmentComponent = environmentLight->getComponent<octoon::EnvironmentLightComponent>();
				if (environmentComponent)
					environmentComponent->setOffset(offset);

				auto meshRenderer = environmentLight->getComponent<octoon::MeshRendererComponent>();
				if (meshRenderer)
				{
					auto material = meshRenderer->getMaterial();
					if (material->isInstanceOf<octoon::MeshBasicMaterial>())
					{
						auto basicMaterial = material->downcast<octoon::MeshBasicMaterial>();
						basicMaterial->setOffset(offset);
					}
				}
			}
		};

		this->getModel()->useTexture += [this](bool useTexture)
		{
			auto& model = this->getModel();
			auto& profile = this->getContext()->profile;

			auto environmentLight = profile->entitiesModule->environmentLight;
			if (environmentLight)
			{
				auto environmentComponent = environmentLight->getComponent<octoon::EnvironmentLightComponent>();
				if (environmentComponent)
				{
					environmentComponent->setBackgroundMap(model->useTexture ? texture_ : nullptr);
					environmentComponent->setEnvironmentMap(model->useTexture ? this->irradianceTexture_ : nullptr);
				}

				auto meshRenderer = environmentLight->getComponent<octoon::MeshRendererComponent>();
				if (meshRenderer)
				{
					auto material = meshRenderer->getMaterial()->downcast<octoon::MeshBasicMaterial>();
					material->setColorMap(model->useTexture ? texture_ : nullptr);
				}
			}
		};

		this->getModel()->texture += [this](const std::shared_ptr<octoon::GraphicsTexture>& texture)
		{
			auto& model = this->getModel();
			auto& profile = this->getContext()->profile;

			this->texture_ = texture;
			this->irradianceTexture_ = octoon::PMREMLoader::load(this->texture_);

			auto environmentLight = profile->entitiesModule->environmentLight;
			if (environmentLight)
			{
				auto environmentComponent = environmentLight->getComponent<octoon::EnvironmentLightComponent>();
				if (environmentComponent)
				{
					environmentComponent->setBackgroundMap(model->useTexture ? texture : nullptr);
					environmentComponent->setEnvironmentMap(model->useTexture ? this->irradianceTexture_ : nullptr);
				}

				auto meshRenderer = environmentLight->getComponent<octoon::MeshRendererComponent>();
				if (meshRenderer)
				{
					auto material = meshRenderer->getMaterial()->downcast<octoon::MeshBasicMaterial>();
					material->setColorMap(model->useTexture ? texture : nullptr);
				}
			}
		};
	}

	void
	EnvironmentComponent::onEnable() noexcept
	{
	}

	void
	EnvironmentComponent::onDisable() noexcept
	{
	}
}