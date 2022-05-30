#include "entities_component.h"
#include "../unreal_profile.h"
#include "../unreal_behaviour.h"
#include <octoon/ass_loader.h>

#include <fstream>
#include <unordered_map>
#include <omp.h>

namespace unreal
{
	EntitiesComponent::EntitiesComponent() noexcept
	{
	}

	EntitiesComponent::~EntitiesComponent() noexcept
	{
	}

	void
	EntitiesComponent::importAbc(std::string_view path) noexcept(false)
	{
		auto model = octoon::GameObject::create();
		model->addComponent<octoon::MeshAnimationComponent>(path);

		this->getContext()->profile->entitiesModule->objects.push_back(model);
		this->sendMessage("editor:project:open");
	}

	void
	EntitiesComponent::importAss(std::string_view path) noexcept(false)
	{
		auto& context = this->getContext();

		octoon::GameObjects objects = octoon::ASSLoader::load(path);

		for (auto& it : objects)
		{
			if (it->getComponent<octoon::CameraComponent>())
				context->profile->cameraModule->camera = it;
			else
			{
				auto renderer = it->getComponent<octoon::MeshRendererComponent>();
				if (renderer)
					renderer->setGlobalIllumination(true);
				context->profile->entitiesModule->objects.push_back(it);
			}
		}

		this->sendMessage("editor:project:open");
	}

	bool
	EntitiesComponent::importModel(std::string_view path) noexcept
	{
		auto model = octoon::MeshLoader::load(path);
		if (model)
		{
			auto smr = model->getComponent<octoon::SkinnedMeshRendererComponent>();
			if (smr)
				smr->setAutomaticUpdate(!this->getContext()->profile->offlineModule->getEnable());

			this->getContext()->profile->entitiesModule->objects.push_back(model);
			return true;
		}

		return false;
	}

	bool
	EntitiesComponent::exportModel(std::string_view path) noexcept
	{
		return false;
	}

	void
	EntitiesComponent::onInit() noexcept
	{
		this->getModel()->entities += [this](const std::vector<EntitesObject>& entites)
		{
			for (auto& it : entites)
			{
				if (!it.filepath.empty())
				{
					this->getModel()->objects.push_back(octoon::MeshLoader::load(it.filepath));
				}
			}
		};
	}

	void
	EntitiesComponent::onEnable() noexcept
	{
		auto mainLight = octoon::GameObject::create("DirectionalLight");
		mainLight->addComponent<octoon::DirectionalLightComponent>();
		mainLight->getComponent<octoon::DirectionalLightComponent>()->setShadowEnable(true);
		mainLight->getComponent<octoon::DirectionalLightComponent>()->setShadowMapSize(octoon::math::uint2(2048, 2048));
		mainLight->getComponent<octoon::DirectionalLightComponent>()->setSize(this->getContext()->profile->mainLightModule->size);
		mainLight->getComponent<octoon::DirectionalLightComponent>()->setIntensity(this->getContext()->profile->mainLightModule->intensity);
		mainLight->getComponent<octoon::DirectionalLightComponent>()->setColor(this->getContext()->profile->mainLightModule->color);
		mainLight->getComponent<octoon::TransformComponent>()->setQuaternion(octoon::math::Quaternion(octoon::math::radians(this->getContext()->profile->mainLightModule->rotation)));

		auto envMaterial = octoon::MeshBasicMaterial::create(octoon::math::srgb2linear<float>(this->getContext()->profile->environmentLightModule->color));
		envMaterial->setCullMode(octoon::CullMode::Off);
		envMaterial->setGamma(1.0f);
		envMaterial->setDepthEnable(false);
		envMaterial->setDepthWriteEnable(false);

		auto enviromentLight = octoon::GameObject::create("EnvironmentLight");
		enviromentLight->addComponent<octoon::EnvironmentLightComponent>();
		enviromentLight->getComponent<octoon::EnvironmentLightComponent>()->setColor(octoon::math::srgb2linear<float>(this->getContext()->profile->environmentLightModule->color));
		enviromentLight->getComponent<octoon::EnvironmentLightComponent>()->setIntensity(this->getContext()->profile->environmentLightModule->intensity);
		enviromentLight->getComponent<octoon::EnvironmentLightComponent>()->setOffset(this->getContext()->profile->environmentLightModule->offset);
		enviromentLight->addComponent<octoon::MeshFilterComponent>(octoon::SphereMesh(10000, 32, 24, octoon::math::PI * 0.5));
		enviromentLight->addComponent<octoon::MeshRendererComponent>(envMaterial)->setRenderOrder(-2);

		auto mainCamera = octoon::GameObject::create("MainCamera");
		mainCamera->addComponent<octoon::FirstPersonCameraComponent>();
		mainCamera->getComponent<octoon::TransformComponent>()->setTranslate(this->getContext()->profile->cameraModule->translate);
		mainCamera->getComponent<octoon::TransformComponent>()->setEulerAngles(this->getContext()->profile->cameraModule->rotation);

		auto camera = mainCamera->addComponent<octoon::FilmCameraComponent>();
		camera->setFov(this->getContext()->profile->cameraModule->fov);
		camera->setAperture(this->getContext()->profile->cameraModule->useDepthOfFiled ? this->getContext()->profile->cameraModule->aperture.getValue() : 0.0f);
		camera->setCameraType(octoon::CameraType::Main);
		camera->setClearColor(octoon::math::float4(0.0f, 0.0f, 0.0f, 1.0f));
		camera->setupFramebuffers(this->getContext()->profile->recordModule->width, this->getContext()->profile->recordModule->height, 0, octoon::GraphicsFormat::R32G32B32SFloat);

		this->getContext()->profile->cameraModule->camera = mainCamera;
		this->getContext()->profile->mainLightModule->mainLight = mainLight;
		this->getContext()->profile->environmentLightModule->environmentLight = enviromentLight;	
	}

	void
	EntitiesComponent::onDisable() noexcept
	{
	}
}