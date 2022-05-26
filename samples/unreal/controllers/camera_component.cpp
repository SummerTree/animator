#include "camera_component.h"
#include "../unreal_profile.h"
#include "../unreal_behaviour.h"

namespace unreal
{
	CameraComponent::CameraComponent() noexcept
	{
	}

	CameraComponent::~CameraComponent() noexcept
	{
	}

	void
	CameraComponent::onInit() noexcept
	{
		this->getModel()->fov += [this](float value)
		{
			auto camera = this->getContext()->profile->entitiesModule->camera;
			if (camera)
			{
				auto cameraComponent = camera->getComponent<octoon::FilmCameraComponent>();
				if (cameraComponent)
				{
					cameraComponent->setFov(value);
					this->getModel()->focalLength = cameraComponent->getFocalLength();
				}
			}
		};

		this->getModel()->focalLength += [this](float value)
		{
			auto camera = this->getContext()->profile->entitiesModule->camera;
			if (camera)
			{
				auto cameraComponent = camera->getComponent<octoon::FilmCameraComponent>();
				if (cameraComponent)
				{
					cameraComponent->setFocalLength(value);
					this->getModel()->fov = cameraComponent->getFov();
				}
			}
		};

		this->getModel()->focusDistance += [this](float value)
		{
			auto camera = this->getContext()->profile->entitiesModule->camera;
			if (camera)
			{
				auto cameraComponent = camera->getComponent<octoon::FilmCameraComponent>();
				if (cameraComponent)
					cameraComponent->setFocusDistance(value);
			}
		};

		this->getModel()->aperture += [this](float value)
		{
			auto camera = this->getContext()->profile->entitiesModule->camera;
			if (camera)
			{
				auto cameraComponent = camera->getComponent<octoon::FilmCameraComponent>();
				if (cameraComponent)
					cameraComponent->setAperture(this->getModel()->useDepthOfFiled.getValue() ? value : 0.0f);
			}
		};

		this->getModel()->useDepthOfFiled += [this](bool value)
		{
			auto camera = this->getContext()->profile->entitiesModule->camera;
			if (camera)
			{
				auto cameraComponent = camera->getComponent<octoon::FilmCameraComponent>();
				if (cameraComponent)
					cameraComponent->setAperture(value ? this->getModel()->aperture.getValue() : 0.0f);
			}
		};

		this->getModel()->translate += [this](const octoon::math::float3& translate)
		{
			auto camera = this->getContext()->profile->entitiesModule->camera;
			if (camera)
			{
				auto transform = camera->getComponent<octoon::TransformComponent>();
				transform->setTranslate(translate);
			}
		};

		this->getModel()->rotation += [this](const octoon::math::float3& rotation)
		{
			auto camera = this->getContext()->profile->entitiesModule->camera;
			if (camera)
			{
				auto transform = camera->getComponent<octoon::TransformComponent>();
				transform->setEulerAngles(rotation);
			}
		};

		this->getContext()->profile->playerModule->isPlaying += [this](bool value)
		{
			this->update();
		};

		this->getContext()->profile->playerModule->finish += [this](bool value)
		{
			this->update();
		};

		this->addMessageListener("editor:project:open", [this](const std::any& data)
		{
			this->update();
		});

		this->addMessageListener("editor:player:sample", [this](const std::any& data)
		{
			this->update();
		});
	}

	void
	CameraComponent::onEnable() noexcept
	{
	}

	void
	CameraComponent::onDisable() noexcept
	{
	}

	void
	CameraComponent::update() noexcept
	{
		auto camera = this->getContext()->profile->entitiesModule->camera;
		if (camera)
		{
			auto& model = this->getModel();

			auto transformComponent = camera->getComponent<octoon::TransformComponent>();
			if (transformComponent)
			{
				model->translate = camera->getComponent<octoon::TransformComponent>()->getTranslate();
				model->rotation = camera->getComponent<octoon::TransformComponent>()->getEulerAngles();
			}

			auto cameraComponent = camera->getComponent<octoon::FilmCameraComponent>();
			if (cameraComponent)
			{
				model->fov = cameraComponent->getFov();
				model->focusDistance = cameraComponent->getFocalDistance();

				if (cameraComponent->getAperture() > 0.0f)
				{
					model->useDepthOfFiled = true;
					model->aperture = cameraComponent->getAperture();
				}
				else
				{
					model->useDepthOfFiled = false;
				}
			}
		}
	}
}