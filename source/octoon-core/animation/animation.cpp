#include <octoon/animation/animation.h>

namespace octoon
{
	OctoonImplementSubClass(Animation, Object, "Animation")

	Animation::Animation() noexcept
		: name("Default")
	{
		state.finish = false;
		state.time = 0;
		state.timeLength = 0;
	}

	Animation::Animation(std::shared_ptr<AnimationClip>&& _clip, std::string_view _name) noexcept
		: Animation()
	{
		this->setClip(std::move(_clip), _name);
	}

	Animation::Animation(const std::shared_ptr<AnimationClip>& _clip, std::string_view _name) noexcept
		: Animation()
	{
		this->setClip(_clip, _name);
	}

	Animation::Animation(std::unordered_map<std::string, std::shared_ptr<AnimationClip>>&& _clips) noexcept
		: Animation()
	{
		this->setClips(std::move(_clips));
	}

	Animation::Animation(const std::unordered_map<std::string, std::shared_ptr<AnimationClip>>& _clips) noexcept
		: Animation()
	{
		this->setClips(_clips);
	}

	Animation::Animation(std::string&& _name, std::unordered_map<std::string, std::shared_ptr<AnimationClip>>&& _clips) noexcept
		: name(std::move(_name))
		, clips(std::move(_clips))
	{
		state.finish = false;
		state.time = 0;

		for (auto& it : clips)
			state.timeLength = std::max(it.second->timeLength, state.timeLength);
	}

	Animation::Animation(std::string_view _name, const std::unordered_map<std::string, std::shared_ptr<AnimationClip>>& _clips) noexcept
		: name(_name)
		, clips(_clips)
	{
		state.finish = false;
		state.time = 0;

		for (auto& it : clips)
			state.timeLength = std::max(it.second->timeLength, state.timeLength);
	}

	Animation::Animation(std::string_view _name) noexcept
		: name(_name)
	{
		state.finish = false;
	}

	void
	Animation::setName(std::string_view _name) noexcept
	{
		this->name = _name;
	}

	const std::string&
	Animation::getName() const noexcept
	{
		return name;
	}

	void
	Animation::addClip(std::shared_ptr<AnimationClip>&& clip_, std::string_view key) noexcept
	{
		if (!clip) this->clip = clip_;
		clip_->setTime(state.time);
		state.timeLength = std::max(clip_->timeLength, state.timeLength);
		this->clips[std::string(key)] = std::move(clip_);
	}

	void
	Animation::addClip(const std::shared_ptr<AnimationClip>& clip_, std::string_view key) noexcept
	{
		if (!clip) this->clip = clip_;
		clip_->setTime(state.time);
		state.timeLength = std::max(clip_->timeLength, state.timeLength);
		this->clips[std::string(key)] = clip_;
	}

	void
	Animation::setClip(std::shared_ptr<AnimationClip>&& clip_, std::string_view key) noexcept
	{
		state.time = 0;
		state.timeLength = 0;
		state.finish = true;

		this->clip = clip_;
		this->clips.clear();
		this->clips[std::string(key)] = std::move(clip_);

		for (auto& it : this->clips)
		{
			it.second->evaluate(0.0f);

			state.finish &= it.second->finish;
			state.timeLength = std::max(it.second->timeLength, state.timeLength);
		}
	}

	void
	Animation::setClip(const std::shared_ptr<AnimationClip>& clip_, std::string_view key) noexcept
	{
		state.time = 0;
		state.timeLength = 0;
		state.finish = true;

		this->clip = clip_;
		this->clips.clear();
		this->clips[std::string(key)] = clip_;

		for (auto& it : this->clips)
		{
			it.second->evaluate(0.0f);

			state.finish &= it.second->finish;
			state.timeLength = std::max(it.second->timeLength, state.timeLength);
		}
	}

	void
	Animation::setClips(const std::unordered_map<std::string, std::shared_ptr<AnimationClip>>& clips_) noexcept
	{
		state.time = 0;
		state.timeLength = 0;
		state.finish = true;

		this->clip = (*clips_.begin()).second;
		this->clips = clips_;

		for (auto& it : this->clips)
		{
			it.second->evaluate(0.0f);

			state.finish &= it.second->finish;
			state.timeLength = std::max(it.second->timeLength, state.timeLength);
		}
	}

	void
	Animation::setClips(std::unordered_map<std::string, std::shared_ptr<AnimationClip>>&& clips_) noexcept
	{
		state.time = 0;
		state.timeLength = 0;
		state.finish = true;

		this->clip = (*clips_.begin()).second;
		this->clips = std::move(clips_);

		for (auto& it : this->clips)
		{
			it.second->evaluate(0.0f);

			state.finish &= it.second->finish;
			state.timeLength = std::max(it.second->timeLength, state.timeLength);
		}
	}

	std::shared_ptr<AnimationClip>
	Animation::getClip(const std::string& key)
	{
		if (this->clips.find(key) != this->clips.end())
			return this->clips.at(key);
		return nullptr;
	}

	const std::shared_ptr<AnimationClip>&
	Animation::getClip(const std::string& key) const
	{
		return this->clips.at(key);
	}

	bool
	Animation::hasClip(std::string_view key) const
	{
		return this->clips.find((std::string)key) != this->clips.end();
	}

	float
	Animation::timeLength() const noexcept
	{
		return state.timeLength;
	}

	bool
	Animation::setDefaultClip(std::string_view key)
	{
		this->clip = this->getClip((std::string)key);
		return this->clip ? true : false;
	}

	void
	Animation::setTime(const float& time) noexcept
	{
		if (this->state.time != time)
		{
			for (auto& it : this->clips)
				it.second->setTime(time);

			this->state.time = time;
			this->state.finish = true;

			for (auto& it : this->clips)
				this->state.finish &= it.second->finish;
		}
	}

	float
	Animation::getTime() const noexcept
	{
		return this->state.time;
	}

	void
	Animation::evaluate(const float& delta) noexcept
	{
		this->state.time += delta;
		this->clip->evaluate(delta);
		this->state.finish = this->clip->finish;
	}

	bool
	Animation::empty() const noexcept
	{
		return this->clips.empty();
	}

	std::size_t
	Animation::size() const noexcept
	{
		return this->clips.size();
	}
}