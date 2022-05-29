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
		if (!context->profile->entitiesModule->objects.empty())
		{
			context->profile->entitiesModule->objects.clear();
		}

		octoon::GameObjects objects = octoon::ASSLoader::load(path);

		for (auto& it : objects)
		{
			if (it->getComponent<octoon::CameraComponent>())
				context->profile->entitiesModule->camera = it;
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

	void
	EntitiesComponent::importPMM(std::string_view path) noexcept(false)
	{
		auto& context = this->getContext();
		if (!context->profile->entitiesModule->objects.empty())
		{
			context->profile->entitiesModule->objects.clear();
			context->profile->entitiesModule->camera.getValue().reset();
		}

		octoon::GameObjects objects;
		octoon::GameObjects rigidbodies;

		auto stream = octoon::io::ifstream(std::string(path));
		auto pmm = octoon::PMMFile::load(stream).value();

		auto rotation = octoon::math::Quaternion(octoon::math::float3(-0.1, octoon::math::PI + 0.5f, 0.0f));

		auto& mainLight = this->getContext()->profile->entitiesModule->mainLight.getValue();
		mainLight->getComponent<octoon::DirectionalLightComponent>()->setIntensity(this->getContext()->profile->mainLightModule->intensity);
		mainLight->getComponent<octoon::DirectionalLightComponent>()->setColor(this->getContext()->profile->mainLightModule->color);
		mainLight->getComponent<octoon::DirectionalLightComponent>()->setShadowMapSize(octoon::math::uint2(2048, 2048));
		mainLight->getComponent<octoon::DirectionalLightComponent>()->setShadowEnable(true);
		mainLight->getComponent<octoon::TransformComponent>()->setQuaternion(rotation);
		mainLight->getComponent<octoon::TransformComponent>()->setTranslate(-octoon::math::rotate(rotation, octoon::math::float3::UnitZ) * 50);

		for (auto& it : pmm.model)
		{
			auto model = octoon::MeshLoader::load(it.path);
			if (model)
			{
				octoon::AnimationClips<float> boneClips;
				this->setupBoneAnimation(it, boneClips);

				octoon::AnimationClip<float> morphClip;
				this->setupMorphAnimation(it, morphClip);

				model->setName(it.name);
				model->addComponent<octoon::AnimatorComponent>(octoon::Animation(std::move(boneClips)), model->getComponent<octoon::SkinnedMeshRendererComponent>()->getTransforms());
				model->addComponent<octoon::AnimatorComponent>(octoon::Animation(std::move(morphClip)));

				auto smr = model->getComponent<octoon::SkinnedMeshRendererComponent>();
				if (smr)
					smr->setAutomaticUpdate(!this->getContext()->profile->offlineModule->getEnable());

				objects.emplace_back(std::move(model));
			}
			else
			{
				throw std::runtime_error("Failed to find the file: " + it.path);
			}
		}

		context->profile->soundModule->filepath = pmm.wave_path;
		context->profile->mainLightModule->rotation = octoon::math::degrees(octoon::math::eulerAngles(rotation));
		context->profile->entitiesModule->camera = this->createCamera(pmm);
		context->profile->entitiesModule->objects = objects;

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
	EntitiesComponent::importHDRi(const std::shared_ptr<octoon::GraphicsTexture>& texture) noexcept
	{
		auto& environmentLight = this->getContext()->profile->entitiesModule->environmentLight.getValue();
		if (environmentLight)
		{
			auto envLight = environmentLight->getComponent<octoon::EnvironmentLightComponent>();
			if (envLight)
				envLight->setRadianceMap(octoon::PMREMLoader::load(texture));

			auto material = environmentLight->getComponent<octoon::MeshRendererComponent>()->getMaterial()->downcast<octoon::MeshBasicMaterial>();
			material->setColorMap(texture);
		}
	}

	void
	EntitiesComponent::importHDRi(std::string_view filepath) noexcept
	{
		auto texture = octoon::TextureLoader::load(filepath);
		if (texture)
			this->importHDRi(texture);
	}

	void
	EntitiesComponent::clearHDRi() noexcept
	{
		auto& environmentLight = this->getContext()->profile->entitiesModule->environmentLight.getValue();
		if (environmentLight)
		{
			auto envLight = environmentLight->getComponent<octoon::EnvironmentLightComponent>();
			if (envLight)
				envLight->setRadianceMap(nullptr);

			auto material = environmentLight->getComponent<octoon::MeshRendererComponent>()->getMaterial()->downcast<octoon::MeshBasicMaterial>();
			material->setColorMap(nullptr);
		}
	}

	octoon::GameObjectPtr
	EntitiesComponent::createCamera(const octoon::PMMFile& pmm) noexcept
	{
		octoon::Animation<float> animtion;
		this->setupCameraAnimation(pmm.camera_keyframes, animtion);

		auto eye = octoon::math::float3(pmm.camera.eye.x, pmm.camera.eye.y, pmm.camera.eye.z);
		auto target = octoon::math::float3(pmm.camera.target.x, pmm.camera.target.y, pmm.camera.target.z);
		auto quat = octoon::math::Quaternion(-octoon::math::float3(pmm.camera.rotation.x, pmm.camera.rotation.y, pmm.camera.rotation.z));

		auto mainCamera = octoon::GameObject::create("MainCamera");
		mainCamera->addComponent<octoon::FirstPersonCameraComponent>();
		mainCamera->addComponent<octoon::AudioListenerComponent>();
		mainCamera->addComponent<octoon::AnimatorComponent>(animtion);

		auto camera = mainCamera->addComponent<octoon::FilmCameraComponent>();
		camera->setFov((float)pmm.camera_keyframes[0].fov);
		camera->setCameraType(octoon::CameraType::Main);
		camera->setClearFlags(octoon::ClearFlagBits::AllBit);
		camera->setClearColor(octoon::math::float4(0.0f, 0.0f, 0.0f, 1.0f));
		camera->setupFramebuffers(this->getContext()->profile->recordModule->width, this->getContext()->profile->recordModule->height, 0, octoon::GraphicsFormat::R32G32B32SFloat);

		auto transform = mainCamera->getComponent<octoon::TransformComponent>();
		transform->setQuaternion(quat);
		transform->setTranslate(eye);
		transform->setTranslateAccum(octoon::math::rotate(quat, octoon::math::float3::Forward) * octoon::math::distance(eye, target));

		this->getContext()->behaviour->sendMessage("editor:camera:set", mainCamera);

		return mainCamera;
	}

	void
	EntitiesComponent::setupCameraAnimation(const std::vector<octoon::PmmKeyframeCamera>& camera_keyframes, octoon::Animation<float>& animation) noexcept
	{
		octoon::Keyframes<float> distance;
		octoon::Keyframes<float> eyeX;
		octoon::Keyframes<float> eyeY;
		octoon::Keyframes<float> eyeZ;
		octoon::Keyframes<float> rotationX;
		octoon::Keyframes<float> rotationY;
		octoon::Keyframes<float> rotationZ;
		octoon::Keyframes<float> fov;

		distance.reserve(camera_keyframes.size());
		eyeX.reserve(camera_keyframes.size());
		eyeY.reserve(camera_keyframes.size());
		eyeZ.reserve(camera_keyframes.size());
		rotationX.reserve(camera_keyframes.size());
		rotationY.reserve(camera_keyframes.size());
		rotationZ.reserve(camera_keyframes.size());
		fov.reserve(camera_keyframes.size());

		for (auto& it : camera_keyframes)
		{
			auto interpolationDistance = std::make_shared<octoon::PathInterpolator<float>>(it.interpolation_distance[0] / 127.0f, it.interpolation_distance[2] / 127.0f, it.interpolation_distance[1] / 127.0f, it.interpolation_distance[3] / 127.0f);
			auto interpolationX = std::make_shared<octoon::PathInterpolator<float>>(it.interpolation_x[0] / 127.0f, it.interpolation_x[2] / 127.0f, it.interpolation_x[1] / 127.0f, it.interpolation_x[3] / 127.0f);
			auto interpolationY = std::make_shared<octoon::PathInterpolator<float>>(it.interpolation_y[0] / 127.0f, it.interpolation_y[2] / 127.0f, it.interpolation_y[1] / 127.0f, it.interpolation_y[3] / 127.0f);
			auto interpolationZ = std::make_shared<octoon::PathInterpolator<float>>(it.interpolation_z[0] / 127.0f, it.interpolation_z[2] / 127.0f, it.interpolation_z[1] / 127.0f, it.interpolation_z[3] / 127.0f);
			auto interpolationRotation = std::make_shared<octoon::PathInterpolator<float>>(it.interpolation_rotation[0] / 127.0f, it.interpolation_rotation[2] / 127.0f, it.interpolation_rotation[1] / 127.0f, it.interpolation_rotation[3] / 127.0f);
			auto interpolationAngleView = std::make_shared<octoon::PathInterpolator<float>>(it.interpolation_angleview[0] / 127.0f, it.interpolation_angleview[2] / 127.0f, it.interpolation_angleview[1] / 127.0f, it.interpolation_angleview[3] / 127.0f);

			distance.emplace_back((float)it.frame / 30.0f, it.distance, interpolationDistance);
			eyeX.emplace_back((float)it.frame / 30.0f, it.eye.x, interpolationX);
			eyeY.emplace_back((float)it.frame / 30.0f, it.eye.y, interpolationY);
			eyeZ.emplace_back((float)it.frame / 30.0f, it.eye.z, interpolationZ);
			rotationX.emplace_back((float)it.frame / 30.0f, it.rotation.x, interpolationRotation);
			rotationY.emplace_back((float)it.frame / 30.0f, it.rotation.y, interpolationRotation);
			rotationZ.emplace_back((float)it.frame / 30.0f, it.rotation.z, interpolationRotation);
			fov.emplace_back((float)it.frame / 30.0f, (float)it.fov, interpolationAngleView);
		}

		octoon::AnimationClip clip;
		clip.setCurve("LocalPosition.x", octoon::AnimationCurve(std::move(eyeX)));
		clip.setCurve("LocalPosition.y", octoon::AnimationCurve(std::move(eyeY)));
		clip.setCurve("LocalPosition.z", octoon::AnimationCurve(std::move(eyeZ)));
		clip.setCurve("LocalEulerAnglesRaw.x", octoon::AnimationCurve(std::move(rotationX)));
		clip.setCurve("LocalEulerAnglesRaw.y", octoon::AnimationCurve(std::move(rotationY)));
		clip.setCurve("LocalEulerAnglesRaw.z", octoon::AnimationCurve(std::move(rotationZ)));
		clip.setCurve("Transform:move", octoon::AnimationCurve(std::move(distance)));
		clip.setCurve("Camera:fov", octoon::AnimationCurve(std::move(fov)));

		animation.addClip(std::move(clip));
	}

	void
	EntitiesComponent::setupBoneAnimation(const octoon::PmmModel& it, octoon::AnimationClips<float>& clips) noexcept
	{
		std::size_t numBone = it.bone_init_frame.size();

		clips.resize(numBone);

#		pragma omp parallel for num_threads(omp_get_num_procs() / 2)
		for (std::int32_t i = 0; i < numBone; i++)
		{
			auto keyframeCount = 1u;
			auto next_index = it.bone_init_frame[i].next_index;
			auto final_frame = it.bone_init_frame[i].frame;
			while (next_index > 0)
			{
				keyframeCount++;
				next_index -= it.bone_init_frame.size();
				final_frame = it.bone_key_frame[next_index].frame;
				next_index = it.bone_key_frame[next_index].next_index;
			}

			octoon::Keyframes<float> translateX;
			octoon::Keyframes<float> translateY;
			octoon::Keyframes<float> translateZ;
			octoon::Keyframes<float> rotationX;
			octoon::Keyframes<float> rotationY;
			octoon::Keyframes<float> rotationZ;

			translateX.reserve(keyframeCount);
			translateY.reserve(keyframeCount);
			translateZ.reserve(keyframeCount);
			rotationX.reserve(final_frame * 20u + 1u);
			rotationY.reserve(final_frame * 20u + 1u);
			rotationZ.reserve(final_frame * 20u + 1u);

			auto& initKey = it.bone_init_frame[i];
			auto interpolationX = std::make_shared<octoon::PathInterpolator<float>>(initKey.interpolation_x[0] / 127.0f, initKey.interpolation_x[2] / 127.0f, initKey.interpolation_x[1] / 127.0f, initKey.interpolation_x[3] / 127.0f);
			auto interpolationY = std::make_shared<octoon::PathInterpolator<float>>(initKey.interpolation_y[0] / 127.0f, initKey.interpolation_y[2] / 127.0f, initKey.interpolation_y[1] / 127.0f, initKey.interpolation_y[3] / 127.0f);
			auto interpolationZ = std::make_shared<octoon::PathInterpolator<float>>(initKey.interpolation_z[0] / 127.0f, initKey.interpolation_z[2] / 127.0f, initKey.interpolation_z[1] / 127.0f, initKey.interpolation_z[3] / 127.0f);
			auto interpolationRotation = std::make_shared<octoon::PathInterpolator<float>>(initKey.interpolation_rotation[0] / 127.0f, initKey.interpolation_rotation[2] / 127.0f, initKey.interpolation_rotation[1] / 127.0f, initKey.interpolation_rotation[3] / 127.0f);

			auto euler = octoon::math::eulerAngles(octoon::math::Quaternion(initKey.quaternion.x, initKey.quaternion.y, initKey.quaternion.z, initKey.quaternion.w));

			translateX.emplace_back((float)initKey.frame / 30.0f, initKey.translation.x, interpolationX);
			translateY.emplace_back((float)initKey.frame / 30.0f, initKey.translation.y, interpolationY);
			translateZ.emplace_back((float)initKey.frame / 30.0f, initKey.translation.z, interpolationZ);
			rotationX.emplace_back((float)initKey.frame / 30.0f, euler.x, interpolationRotation);
			rotationY.emplace_back((float)initKey.frame / 30.0f, euler.y, interpolationRotation);
			rotationZ.emplace_back((float)initKey.frame / 30.0f, euler.z, interpolationRotation);

			auto next = it.bone_init_frame[i].next_index;
			while (next > 0)
			{
				auto& key = it.bone_key_frame[next - numBone];
				auto& keyLast = key.pre_index < numBone ? it.bone_init_frame[key.pre_index] : it.bone_key_frame[key.pre_index - numBone];

				interpolationX = std::make_shared<octoon::PathInterpolator<float>>(key.interpolation_x[0] / 127.0f, key.interpolation_x[2] / 127.0f, key.interpolation_x[1] / 127.0f, key.interpolation_x[3] / 127.0f);
				interpolationY = std::make_shared<octoon::PathInterpolator<float>>(key.interpolation_y[0] / 127.0f, key.interpolation_y[2] / 127.0f, key.interpolation_y[1] / 127.0f, key.interpolation_y[3] / 127.0f);
				interpolationZ = std::make_shared<octoon::PathInterpolator<float>>(key.interpolation_z[0] / 127.0f, key.interpolation_z[2] / 127.0f, key.interpolation_z[1] / 127.0f, key.interpolation_z[3] / 127.0f);
				interpolationRotation = std::make_shared<octoon::PathInterpolator<float>>(keyLast.interpolation_rotation[0] / 127.0f, keyLast.interpolation_rotation[2] / 127.0f, keyLast.interpolation_rotation[1] / 127.0f, keyLast.interpolation_rotation[3] / 127.0f);

				for (std::size_t j = 1; j <= (key.frame - keyLast.frame) * 20; j++)
				{
					auto qkeyLast = octoon::math::Quaternion(keyLast.quaternion.x, keyLast.quaternion.y, keyLast.quaternion.z, keyLast.quaternion.w);
					auto qkey = octoon::math::Quaternion(key.quaternion.x, key.quaternion.y, key.quaternion.z, key.quaternion.w);

					auto t = j / ((key.frame - keyLast.frame) * 20.0f);
					auto eulerDelta = octoon::math::eulerAngles(octoon::math::slerp(qkeyLast, qkey, interpolationRotation->interpolator(t)));
					auto frame = keyLast.frame + (key.frame - keyLast.frame) / ((key.frame - keyLast.frame) * 20.0f) * j;

					rotationX.emplace_back((float)frame / 30.0f, eulerDelta.x);
					rotationY.emplace_back((float)frame / 30.0f, eulerDelta.y);
					rotationZ.emplace_back((float)frame / 30.0f, eulerDelta.z);
				}

				translateX.emplace_back((float)key.frame / 30.0f, key.translation.x, interpolationX);
				translateY.emplace_back((float)key.frame / 30.0f, key.translation.y, interpolationY);
				translateZ.emplace_back((float)key.frame / 30.0f, key.translation.z, interpolationZ);

				next = key.next_index;
			}

			auto& clip = clips[i];
			clip.setName(it.bone_name[i]);
			clip.setCurve("LocalPosition.x", octoon::AnimationCurve(std::move(translateX)));
			clip.setCurve("LocalPosition.y", octoon::AnimationCurve(std::move(translateY)));
			clip.setCurve("LocalPosition.z", octoon::AnimationCurve(std::move(translateZ)));
			clip.setCurve("LocalEulerAnglesRaw.x", octoon::AnimationCurve(std::move(rotationX)));
			clip.setCurve("LocalEulerAnglesRaw.y", octoon::AnimationCurve(std::move(rotationY)));
			clip.setCurve("LocalEulerAnglesRaw.z", octoon::AnimationCurve(std::move(rotationZ)));
		}
	}

	void
	EntitiesComponent::setupMorphAnimation(const octoon::PmmModel& it, octoon::AnimationClip<float>& clip) noexcept
	{
		for (std::size_t i = 0, keyframeCount = 1; i < it.morph_init_frame.size(); i++)
		{
			auto index = it.morph_init_frame[i].next_index;
			while (index > 0)
			{
				keyframeCount++;
				index = it.morph_key_frame[index - it.morph_init_frame.size()].next_index;
			}

			octoon::Keyframes<float> keyframes;
			keyframes.reserve(keyframeCount);
			keyframes.emplace_back((float)it.morph_init_frame[i].frame / 30.0f, it.morph_init_frame[i].value);

			auto next = it.morph_init_frame[i].next_index;
			while (next > 0)
			{
				next -= it.morph_init_frame.size();
				auto& frame = it.morph_key_frame[next];
				keyframes.emplace_back((float)frame.frame / 30.0f, frame.value);
				next = frame.next_index;
			}

			clip.setCurve(it.morph_name[i], octoon::AnimationCurve(std::move(keyframes)));
		}
	}

	void
	EntitiesComponent::onInit() noexcept
	{
	}

	void
	EntitiesComponent::onEnable() noexcept
	{
		auto mainLight = octoon::GameObject::create("DirectionalLight");
		mainLight->addComponent<octoon::DirectionalLightComponent>();
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

		this->getContext()->profile->entitiesModule->camera = mainCamera;
		this->getContext()->profile->entitiesModule->mainLight = mainLight;
		this->getContext()->profile->entitiesModule->environmentLight = enviromentLight;
		
		auto planeGeometry = octoon::CubeMesh::create(1, 1, 1);

		auto material = std::make_shared<octoon::MeshStandardMaterial>();
		material->setCullMode(octoon::CullMode::Off);

		this->sendMessage("editor:camera:set", mainCamera);
	}

	void
	EntitiesComponent::onDisable() noexcept
	{
	}
}