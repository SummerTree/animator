#include <octoon/animator_component.h>
#include <octoon/transform_component.h>
#include <octoon/solver_component.h>
#include <octoon/timer_feature.h>
#include <octoon/rigidbody_component.h>
#include <iostream>
#include <codecvt>

namespace octoon
{
	OctoonImplementSubClass(AnimatorComponent, AnimationComponent, "Animator")

	AnimatorComponent::AnimatorComponent() noexcept
		: enableAnimation_(true)
		, enableAnimOnVisableOnly_(false)
	{
	}

	AnimatorComponent::AnimatorComponent(Animation<float>&& animation, GameObjects&& avatar) noexcept
		: AnimatorComponent()
	{
		this->setAvatar(std::move(avatar));
		this->setAnimation(std::move(animation));
	}

	AnimatorComponent::AnimatorComponent(Animation<float>&& animation, const GameObjects& avatar) noexcept
		: AnimatorComponent()
	{
		this->setAvatar(avatar);
		this->setAnimation(std::move(animation));
	}

	AnimatorComponent::AnimatorComponent(const Animation<float>& animation, GameObjects&& avatar) noexcept
		: AnimatorComponent()
	{
		this->setAvatar(std::move(avatar));
		this->setAnimation(animation);
	}

	AnimatorComponent::AnimatorComponent(const Animation<float>& animation, const GameObjects& avatar) noexcept
		: AnimatorComponent()
	{
		this->setAvatar(avatar);
		this->setAnimation(animation);
	}

	AnimatorComponent::AnimatorComponent(Animation<float>&& animation) noexcept
		: AnimatorComponent()
	{
		animation_ = std::move(animation);
	}

	AnimatorComponent::AnimatorComponent(const Animation<float>& animation) noexcept
		: AnimatorComponent()
	{
		animation_ = animation;
	}

	AnimatorComponent::AnimatorComponent(GameObjects&& avatar) noexcept
		: AnimatorComponent()
	{
		this->setAvatar(std::move(avatar));
	}

	AnimatorComponent::AnimatorComponent(const GameObjects& avatar) noexcept
		: AnimatorComponent()
	{
		this->setAvatar(avatar);
	}

	AnimatorComponent::~AnimatorComponent() noexcept
	{
	}

	bool
	AnimatorComponent::play(std::string_view status) noexcept
	{
		this->setName(status);
		this->addComponentDispatch(GameDispatchType::FixedUpdate);

		enableAnimation_ = true;
		return enableAnimation_;
	}

	void
	AnimatorComponent::pause() noexcept
	{
		enableAnimation_ = false;
		this->removeComponentDispatch(GameDispatchType::FixedUpdate);
	}

	void
	AnimatorComponent::reset() noexcept
	{
		this->setTime(0.0f);
		this->removeComponentDispatch(GameDispatchType::FixedUpdate);
	}

	void
	AnimatorComponent::setTime(float time) noexcept
	{
		animation_.setTime(time);
	}

	float
	AnimatorComponent::getTime() const noexcept
	{
		return animation_.getTime();
	}

	void
	AnimatorComponent::sample(float delta) noexcept
	{
		if (delta != 0.0f)
			animation_.evaluate(delta);

		if (!avatar_.empty())
		{
			this->updateAvatar();

			for (auto& bone : avatar_)
			{
				for (auto& child : bone->getChildren())
				{
					auto rigidbody = child->getComponent<RigidbodyComponent>();
					if (rigidbody)
					{
						auto transform = child->getComponent<TransformComponent>();
						rigidbody->setPositionAndRotation(transform->getTranslate(), transform->getQuaternion());
						rigidbody->setLinearVelocity(math::float3::Zero);
						rigidbody->setAngularVelocity(math::float3::Zero);
						rigidbody->setInterpolationLinearVelocity(math::float3::Zero);
						rigidbody->setInterpolationAngularVelocity(math::float3::Zero);
						rigidbody->clearForce();
					}
				}
			}
		}
		else
		{
			this->updateAnimation();
		}
	}

	void
	AnimatorComponent::evaluate(float delta) noexcept
	{
		if (delta != 0.0f)
			animation_.evaluate(delta);

		if (!avatar_.empty())
			this->updateAvatar();
		else
			this->updateAnimation();
	}

	void
	AnimatorComponent::setAnimation(Animation<float>&& animtion) noexcept
	{
		animation_ = std::move(animtion);
		this->updateBindmap();
	}

	void
	AnimatorComponent::setAnimation(const Animation<float>& clips) noexcept
	{
		animation_ = clips;
		this->updateBindmap();
	}

	const Animation<float>&
	AnimatorComponent::getAnimation() const noexcept
	{
		return animation_;
	}

	void
	AnimatorComponent::setAvatar(GameObjects&& avatar) noexcept
	{
		avatar_ = std::move(avatar);
		this->updateBindpose(avatar_);
		this->updateBindmap();
	}

	void
	AnimatorComponent::setAvatar(const GameObjects& avatar) noexcept
	{
		avatar_ = avatar;
		this->updateBindpose(avatar_);
		this->updateBindmap();
	}

	const GameObjects&
	AnimatorComponent::getAvatar() const noexcept
	{
		return avatar_;
	}

	const AnimatorStateInfo<float>&
	AnimatorComponent::getCurrentAnimatorStateInfo() const noexcept
	{
		return animation_.state;
	}

	GameComponentPtr
	AnimatorComponent::clone() const noexcept
	{
		auto instance = std::make_shared<AnimatorComponent>();
		instance->setName(this->getName());
		instance->setAvatar(this->getAvatar());
		instance->setAnimation(this->getAnimation());

		return instance;
	}

	void 
	AnimatorComponent::onActivate() except
	{
	}

	void
	AnimatorComponent::onDeactivate() noexcept
	{
		this->removeComponentDispatch(GameDispatchType::FixedUpdate);
	}

	void
	AnimatorComponent::onFixedUpdate() noexcept
	{
		if (enableAnimation_)
		{
			auto timeFeature = this->getFeature<TimerFeature>();
			if (timeFeature)
			{
				auto delta = timeFeature->delta();
				if (delta != 0.0f)
					animation_.evaluate(delta);

				if (!avatar_.empty())
					this->updateAvatar();
				else
					this->updateAnimation();
			}
		}
	}

	void
	AnimatorComponent::updateBindpose(const GameObjects& avatar) noexcept
	{
		bindpose_.resize(avatar.size());

		for (std::size_t i = 0; i < avatar.size(); i++)
			bindpose_[i] = avatar[i]->getComponent<TransformComponent>()->getLocalTranslate();
	}

	void
	AnimatorComponent::updateBindmap() noexcept
	{
		bindmap_.clear();

		if (!animation_.empty() && !avatar_.empty())
		{
			std::map<std::string, std::size_t> boneMap;

			for (std::size_t i = 0; i < avatar_.size(); i++)
				boneMap[avatar_[i]->getName()] = i;

			bindmap_.resize(animation_.clips.size());

			for (std::size_t i = 0; i < animation_.clips.size(); i++)
			{
				auto it = boneMap.find(animation_.clips[i].name);
				if (it != boneMap.end())
					bindmap_[i] = (*it).second;
				else
					bindmap_[i] = std::string::npos;
			}
		}
	}

	void
	AnimatorComponent::updateAvatar(float delta) noexcept
	{
		if (this->getCurrentAnimatorStateInfo().finish)
			return;

		for (std::size_t i = 0; i < animation_.clips.size(); i++)
		{
			if (bindmap_[i] == std::string::npos)
				continue;

			auto transform = avatar_[bindmap_[i]]->getComponent<TransformComponent>();
			auto scale = transform->getLocalScale();
			auto quat = transform->getLocalQuaternion();
			auto translate = transform->getLocalTranslate();
			auto euler = transform->getLocalEulerAngles();
			auto move = 0.0f;

			for (auto& curve : animation_.clips[i].curves)
			{
				if (curve.first == "LocalPosition")
					translate = curve.second.value.getFloat3() + bindpose_[bindmap_[i]];
				if (curve.first == "LocalPosition.x")
					translate.x = curve.second.value.getFloat() + bindpose_[bindmap_[i]].x;
				else if (curve.first == "LocalPosition.y")
					translate.y = curve.second.value.getFloat() + bindpose_[bindmap_[i]].y;
				else if (curve.first == "LocalPosition.z")
					translate.z = curve.second.value.getFloat() + bindpose_[bindmap_[i]].z;
				else if (curve.first == "LocalScale")
					scale = curve.second.value.getFloat3();
				else if (curve.first == "LocalScale.x")
					scale.x = curve.second.value.getFloat();
				else if (curve.first == "LocalScale.y")
					scale.y = curve.second.value.getFloat();
				else if (curve.first == "LocalScale.z")
					scale.z = curve.second.value.getFloat();
				else if (curve.first == "LocalRotation")
					quat = curve.second.value.getQuaternion();
				else if (curve.first == "LocalRotation.x")
					quat.x = curve.second.value.getFloat();
				else if (curve.first == "LocalRotation.y")
					quat.y = curve.second.value.getFloat();
				else if (curve.first == "LocalRotation.z")
					quat.z = curve.second.value.getFloat();
				else if (curve.first == "LocalRotation.w")
					quat.w = curve.second.value.getFloat();
				else if (curve.first == "LocalForward")
					move = curve.second.value.getFloat();
				else if (curve.first == "LocalEulerAnglesRaw")
				{
					euler = curve.second.value.getFloat3();
					quat = math::Quaternion(euler);
				}
				else if (curve.first == "LocalEulerAnglesRaw.x")
				{
					euler.x = curve.second.value.getFloat();
					quat = math::Quaternion(euler);
				}
				else if (curve.first == "LocalEulerAnglesRaw.y")
				{
					euler.y = curve.second.value.getFloat();
					quat = math::Quaternion(euler);
				}
				else if (curve.first == "LocalEulerAnglesRaw.z")
				{
					euler.z = curve.second.value.getFloat();
					quat = math::Quaternion(euler);
				}
				else
				{
					this->sendMessage(curve.first, curve.second.value);
				}
			}

			if (move != 0.0f)
			{
				transform->setLocalScale(scale);
				transform->setLocalTranslate(translate + math::rotate(quat, math::float3::Forward) * move);
				transform->setLocalQuaternion(quat);
			}
			else
			{
				transform->setLocalScale(scale);
				transform->setLocalTranslate(translate);
				transform->setLocalQuaternion(quat);
			}
		}

		this->sendMessage("octoon:animation:update");
	}

	void
	AnimatorComponent::updateAnimation(float delta) noexcept
	{
		for (auto& clip : animation_.clips)
		{
			if (clip.finish)
				continue;

			auto transform = this->getComponent<TransformComponent>();
			auto scale = transform->getLocalScale();
			auto quat = transform->getLocalQuaternion();
			auto translate = transform->getLocalTranslate();
			auto euler = transform->getLocalEulerAngles();
			auto move = 0.0f;

			for (auto& curve : clip.curves)
			{
				if (curve.first == "LocalPosition")
					translate = curve.second.value.getFloat3();
				if (curve.first == "LocalPosition.x")
					translate.x = curve.second.value.getFloat();
				else if (curve.first == "LocalPosition.y")
					translate.y = curve.second.value.getFloat();
				else if (curve.first == "LocalPosition.z")
					translate.z = curve.second.value.getFloat();
				else if (curve.first == "LocalScale")
					scale = curve.second.value.getFloat3();
				else if (curve.first == "LocalScale.x")
					scale.x = curve.second.value.getFloat();
				else if (curve.first == "LocalScale.y")
					scale.y = curve.second.value.getFloat();
				else if (curve.first == "LocalScale.z")
					scale.z = curve.second.value.getFloat();
				else if (curve.first == "LocalRotation")
					quat = curve.second.value.getQuaternion();
				else if (curve.first == "LocalRotation.x")
					quat.x = curve.second.value.getFloat();
				else if (curve.first == "LocalRotation.y")
					quat.y = curve.second.value.getFloat();
				else if (curve.first == "LocalRotation.z")
					quat.z = curve.second.value.getFloat();
				else if (curve.first == "LocalRotation.w")
					quat.w = curve.second.value.getFloat();
				else if (curve.first == "LocalForward")
					move = curve.second.value.getFloat();
				else if (curve.first == "LocalEulerAnglesRaw")
				{
					euler = curve.second.value.getFloat3();
					quat = math::Quaternion(euler);
				}
				else if (curve.first == "LocalEulerAnglesRaw.x")
				{
					euler.x = curve.second.value.getFloat();
					quat = math::Quaternion(euler);
				}
				else if (curve.first == "LocalEulerAnglesRaw.y")
				{
					euler.y = curve.second.value.getFloat();
					quat = math::Quaternion(euler);
				}
				else if (curve.first == "LocalEulerAnglesRaw.z")
				{
					euler.z = curve.second.value.getFloat();
					quat = math::Quaternion(euler);
				}
				else
					this->sendMessage(curve.first, curve.second.value);
			}

			if (move != 0.0f)
			{
				transform->setLocalScale(scale);
				transform->setLocalTranslate(translate + math::rotate(quat, math::float3::Forward) * move);
				transform->setLocalQuaternion(quat);
			}
			else
			{
				transform->setLocalScale(scale);
				transform->setLocalTranslate(translate);
				transform->setLocalQuaternion(quat);
			}
		}

		this->sendMessage("octoon:animation:update");
	}
}