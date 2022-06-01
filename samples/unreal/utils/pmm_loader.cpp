#include "pmm_loader.h"
#include "../unreal_profile.h"
#include "../unreal_behaviour.h"
#include <quuid.h>

namespace unreal
{
	PMMLoader::PMMLoader() noexcept
	{
	}

	PMMLoader::~PMMLoader() noexcept
	{
	}
	
	void
	PMMLoader::load(UnrealProfile& profile, std::string_view path) noexcept(false)
	{
		octoon::GameObjects objects;

		auto stream = octoon::io::ifstream(std::string(path));
		auto pmm = octoon::PMMFile::load(stream).value();

		for (auto& it : pmm.model)
		{
			auto object = octoon::PMXLoader::load(it.path);
			if (object)
			{
				octoon::AnimationClips<float> boneClips;
				setupBoneAnimation(it, boneClips);

				octoon::AnimationClip<float> morphClip;
				setupMorphAnimation(it, morphClip);

				auto id = QUuid::createUuid().toString();
				auto uuid = id.toStdString().substr(1, id.length() - 2);

				object->setName(uuid);
				object->addComponent<octoon::AnimatorComponent>(octoon::Animation(std::move(boneClips)), object->getComponent<octoon::SkinnedMeshRendererComponent>()->getTransforms());
				object->addComponent<octoon::AnimatorComponent>(octoon::Animation(std::move(morphClip)));
				object->getComponent<octoon::SkinnedMeshRendererComponent>()->setAutomaticUpdate(!profile.offlineModule->getEnable());

				objects.emplace_back(std::move(object));
			}
			else
			{
				throw std::runtime_error("Failed to find the file: " + it.path);
			}
		}

		profile.physicsModule->gravity.setValue(octoon::math::float3(pmm.gravity_current_data.direction.x, pmm.gravity_current_data.direction.y * pmm.gravity_current_data.acceleration, pmm.gravity_current_data.direction.z));
		profile.soundModule->enable = pmm.is_wave_enabled;
		profile.soundModule->filepath = pmm.wave_path;
		profile.mainLightModule->intensity = 1.0f;
		profile.mainLightModule->color = octoon::math::float3(pmm.main_light.rgb.x, pmm.main_light.rgb.y, pmm.main_light.rgb.z);
		profile.mainLightModule->rotation = octoon::math::degrees(octoon::math::eulerAngles(octoon::math::Quaternion(octoon::math::float3(-0.1, octoon::math::PI + 0.5f, 0.0f))));
		profile.cameraModule->camera = createCamera(profile, pmm);
		profile.entitiesModule->objects = objects;
	}

	octoon::GameObjectPtr
	PMMLoader::createCamera(UnrealProfile& profile, const octoon::PMMFile& pmm) noexcept
	{
		octoon::Animation<float> animtion;
		setupCameraAnimation(pmm.camera_keyframes, animtion);

		auto eye = octoon::math::float3(pmm.camera.eye.x, pmm.camera.eye.y, pmm.camera.eye.z);
		auto target = octoon::math::float3(pmm.camera.target.x, pmm.camera.target.y, pmm.camera.target.z);
		auto quat = octoon::math::Quaternion(-octoon::math::float3(pmm.camera.rotation.x, pmm.camera.rotation.y, pmm.camera.rotation.z));

		auto mainCamera = octoon::GameObject::create("MainCamera");
		mainCamera->addComponent<octoon::FirstPersonCameraComponent>();
		mainCamera->addComponent<octoon::AnimatorComponent>(animtion);

		auto camera = mainCamera->addComponent<octoon::FilmCameraComponent>();
		camera->setFov((float)pmm.camera_keyframes[0].fov);
		camera->setCameraType(octoon::CameraType::Main);
		camera->setClearFlags(octoon::ClearFlagBits::AllBit);
		camera->setClearColor(octoon::math::float4(0.0f, 0.0f, 0.0f, 1.0f));
		camera->setupFramebuffers(profile.recordModule->width, profile.recordModule->height, 0, octoon::GraphicsFormat::R32G32B32SFloat);

		auto transform = mainCamera->getComponent<octoon::TransformComponent>();
		transform->setQuaternion(quat);
		transform->setTranslate(eye + octoon::math::rotate(quat, octoon::math::float3::Forward) * octoon::math::distance(eye, target));

		return mainCamera;
	}

	void
	PMMLoader::setupCameraAnimation(const std::vector<octoon::PmmKeyframeCamera>& keyframes, octoon::Animation<float>& animation) noexcept
	{
		octoon::Keyframes<float> distance;
		octoon::Keyframes<float> eyeX;
		octoon::Keyframes<float> eyeY;
		octoon::Keyframes<float> eyeZ;
		octoon::Keyframes<float> rotationX;
		octoon::Keyframes<float> rotationY;
		octoon::Keyframes<float> rotationZ;
		octoon::Keyframes<float> fov;

		distance.reserve(keyframes.size());
		eyeX.reserve(keyframes.size());
		eyeY.reserve(keyframes.size());
		eyeZ.reserve(keyframes.size());
		rotationX.reserve(keyframes.size());
		rotationY.reserve(keyframes.size());
		rotationZ.reserve(keyframes.size());
		fov.reserve(keyframes.size());

		for (auto& it : keyframes)
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
	PMMLoader::setupBoneAnimation(const octoon::PmmModel& it, octoon::AnimationClips<float>& clips) noexcept
	{
		std::size_t numBone = it.bone_init_frame.size();

		clips.resize(numBone);

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
	PMMLoader::setupMorphAnimation(const octoon::PmmModel& it, octoon::AnimationClip<float>& clip) noexcept
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
}