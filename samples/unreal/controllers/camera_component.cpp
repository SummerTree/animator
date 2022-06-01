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

	bool
	CameraComponent::loadAnimation(const std::shared_ptr<octoon::Animation<float>>& animation) noexcept(false)
	{
		if (animation)
		{
			auto& profile = this->getContext()->profile;

			auto animator = this->getModel()->camera.getValue()->getComponent<octoon::AnimatorComponent>();
			if (!animator)
				animator = this->getModel()->camera.getValue()->addComponent<octoon::AnimatorComponent>();

			animator->setAnimation(animation);
			animator->sample(profile->playerModule->curTime);

			return true;
		}

		return false;
	}

	bool
	CameraComponent::loadAnimation(std::string_view filepath) noexcept(false)
	{
		octoon::io::ifstream stream;

		if (stream.open(std::string(filepath)))
		{
			octoon::VMDLoader loader;

			return this->loadAnimation(loader.loadCameraMotion(stream));
		}

		return false;
	}

	void
	CameraComponent::removeAnimation() noexcept(false)
	{
		auto& profile = this->getContext()->profile;
		auto mainCamera = this->getModel()->camera.getValue();
		mainCamera->removeComponent<octoon::AnimatorComponent>();

		profile->cameraModule->reset();
	}

	void
	CameraComponent::onInit() noexcept
	{
		this->getModel()->fov += [this](float value)
		{
			auto camera = this->getModel()->camera.getValue();
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
			auto camera = this->getModel()->camera.getValue();
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
			auto camera = this->getModel()->camera.getValue();
			if (camera)
			{
				auto cameraComponent = camera->getComponent<octoon::FilmCameraComponent>();
				if (cameraComponent)
					cameraComponent->setFocusDistance(value);
			}
		};

		this->getModel()->aperture += [this](float value)
		{
			auto camera = this->getModel()->camera.getValue();
			if (camera)
			{
				auto cameraComponent = camera->getComponent<octoon::FilmCameraComponent>();
				if (cameraComponent)
					cameraComponent->setAperture(this->getModel()->useDepthOfFiled.getValue() ? value : 0.0f);
			}
		};

		this->getModel()->useDepthOfFiled += [this](bool value)
		{
			auto camera = this->getModel()->camera.getValue();
			if (camera)
			{
				auto cameraComponent = camera->getComponent<octoon::FilmCameraComponent>();
				if (cameraComponent)
					cameraComponent->setAperture(value ? this->getModel()->aperture.getValue() : 0.0f);
			}
		};

		this->getModel()->translate += [this](const octoon::math::float3& translate)
		{
			auto camera = this->getModel()->camera.getValue();
			if (camera)
			{
				auto transform = camera->getComponent<octoon::TransformComponent>();
				transform->setTranslate(translate);
			}
		};

		this->getModel()->rotation += [this](const octoon::math::float3& rotation)
		{
			auto camera = this->getModel()->camera.getValue();
			if (camera)
			{
				auto transform = camera->getComponent<octoon::TransformComponent>();
				transform->setEulerAngles(rotation);
			}
		};

		this->getModel()->animation += [this](const std::shared_ptr<octoon::Animation<float>>& value)
		{
			if (value)
				this->loadAnimation(value);
			else
				this->removeAnimation();
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
		auto mainCamera = octoon::GameObject::create("MainCamera");
		mainCamera->addComponent<octoon::FirstPersonCameraComponent>();
		mainCamera->getComponent<octoon::TransformComponent>()->setTranslate(this->getModel()->translate);
		mainCamera->getComponent<octoon::TransformComponent>()->setEulerAngles(this->getModel()->rotation);

		auto camera = mainCamera->addComponent<octoon::FilmCameraComponent>();
		camera->setFov(this->getModel()->fov);
		camera->setAperture(this->getModel()->useDepthOfFiled ? this->getModel()->aperture.getValue() : 0.0f);
		camera->setCameraType(octoon::CameraType::Main);
		camera->setClearColor(octoon::math::float4(0.0f, 0.0f, 0.0f, 1.0f));
		camera->setupFramebuffers(this->getContext()->profile->recordModule->width, this->getContext()->profile->recordModule->height, 0, octoon::GraphicsFormat::R32G32B32SFloat);

		this->getModel()->camera = mainCamera;
	}

	void
	CameraComponent::onDisable() noexcept
	{
	}

	void
	CameraComponent::onFixedUpdate() noexcept
	{
		auto& model = this->getModel();

		auto camera = this->getModel()->camera.getValue();
		if (camera)
		{
			auto transformComponent = camera->getComponent<octoon::TransformComponent>();
			if (transformComponent)
			{
				model->translate = camera->getComponent<octoon::TransformComponent>()->getTranslate();
				model->rotation = camera->getComponent<octoon::TransformComponent>()->getEulerAngles();
			}
		}
	}

	void
	CameraComponent::update() noexcept
	{
		auto camera = this->getModel()->camera.getValue();
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