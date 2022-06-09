#ifndef OCTOON_ANIMATION_H_
#define OCTOON_ANIMATION_H_

#include <unordered_map>
#include <octoon/animation/animation_clip.h>

namespace octoon
{
	template<typename _Time = float>
	struct AnimatorStateInfo
	{
		bool finish;
		_Time time;
		_Time timeLength;
	};

	template<typename _Time = float>
	class Animation final : public RttiObject
	{
	public:			
		std::string name;
		AnimatorStateInfo<_Time> state;
		std::shared_ptr<AnimationClip<_Time>> clip;
		std::unordered_map<std::string, std::shared_ptr<AnimationClip<_Time>>> clips;

		Animation() noexcept
			: name("Default")
		{
			state.finish = false;
			state.time = 0;
			state.timeLength = 0;
		}

		Animation(std::shared_ptr<AnimationClip<_Time>>&& _clip, std::string_view _name) noexcept
			: Animation()
		{
			this->setClip(std::move(_clip), _name);
		}

		Animation(const std::shared_ptr<AnimationClip<_Time>>& _clip, std::string_view _name) noexcept
			: Animation()
		{
			this->setClip(_clip, _name);
		}

		Animation(std::unordered_map<std::string, std::shared_ptr<AnimationClip<_Time>>>&& _clips) noexcept
			: Animation()
		{
			this->setClips(std::move(_clips));
		}

		Animation(const std::unordered_map<std::string, std::shared_ptr<AnimationClip<_Time>>>& _clips) noexcept
			: Animation()
		{
			this->setClips(_clips);
		}

		Animation(std::string&& _name, std::unordered_map<std::string, std::shared_ptr<AnimationClip<_Time>>>&& _clips) noexcept
			: name(std::move(_name))
			, clips(std::move(_clips))
		{
			state.finish = false;
			state.time = 0;

			for (auto& clip : clips)
				state.timeLength = std::max(clip.timeLength, state.timeLength);
		}

		Animation(std::string_view _name, const std::unordered_map<std::string, std::shared_ptr<AnimationClip<_Time>>>& _clips) noexcept
			: name(_name)
			, clips(_clips)
		{
			state.finish = false;
			state.time = 0;

			for (auto& clip : clips)
				state.timeLength = std::max(clip.timeLength, state.timeLength);
		}

		explicit Animation(std::string_view _name) noexcept
			: name(_name)
		{
			state.finish = false;
		}

		void setName(std::string_view _name) noexcept
		{
			this->name = _name;
		}

		void addClip(std::shared_ptr<AnimationClip<_Time>>&& clip_, std::string_view key) noexcept
		{
			if (!clip) this->clip = clip_;
			clip_->setTime(state.time);
			state.timeLength = std::max(clip_->timeLength, state.timeLength);
			this->clips[std::string(key)] = std::move(clip_);
		}

		void addClip(const std::shared_ptr<AnimationClip<_Time>>& clip_, std::string_view key) noexcept
		{
			if (!clip) this->clip = clip_;
			clip_->setTime(state.time);
			state.timeLength = std::max(clip_->timeLength, state.timeLength);
			this->clips[std::string(key)] = clip_;
		}

		void setClip(std::shared_ptr<AnimationClip<_Time>>&& clip_, std::string_view key) noexcept
		{
			state.time = 0;
			state.timeLength = 0;
			state.finish = true;

			this->clip = clip_;
			this->clips.clear();
			this->clips[std::string(key)] = std::move(clip_);

			for (auto& it : clips)
			{
				it.second->evaluate(0.0f);

				state.finish &= it.second->finish;
				state.timeLength = std::max(it.second->timeLength, state.timeLength);
			}
		}

		void setClip(const std::shared_ptr<AnimationClip<_Time>>& clip_, std::string_view key) noexcept
		{
			state.time = 0;
			state.timeLength = 0;
			state.finish = true;

			this->clip = clip_;
			this->clips.clear();
			this->clips[std::string(key)] = clip_;

			for (auto& it : clips)
			{
				it.second->evaluate(0.0f);

				state.finish &= it.second->finish;
				state.timeLength = std::max(it.second->timeLength, state.timeLength);
			}
		}

		void setClips(const std::unordered_map<std::string, std::shared_ptr<AnimationClip<_Time>>>& clips) noexcept
		{
			state.time = 0;
			state.timeLength = 0;
			state.finish = true;

			this->clip = *clips.begin();
			this->clips = clips;

			for (auto& it : clips)
			{
				it.second->evaluate(0.0f);

				state.finish &= it.second->finish;
				state.timeLength = std::max(it.second->timeLength, state.timeLength);
			}
		}

		void setClips(std::unordered_map<std::string, std::shared_ptr<AnimationClip<_Time>>>&& clips) noexcept
		{
			state.time = 0;
			state.timeLength = 0;
			state.finish = true;

			this->clip = *clips.begin();
			this->clips = std::move(clips);

			for (auto& it : clips)
			{
				it.second->evaluate(0.0f);

				state.finish &= it.second.finish;
				state.timeLength = std::max(it.second->timeLength, state.timeLength);
			}
		}

		std::shared_ptr<AnimationClip<_Time>> getClip(const std::string& key)
		{
			if (this->clips.find(key) != this->clips.end())
				return this->clips.at(key);
			return nullptr;
		}

		const std::shared_ptr<AnimationClip<_Time>>& getClip(const std::string& key) const
		{
			return this->clips.at(key);
		}

		bool hasClip(std::string_view key) const
		{
			return this->clips.find((std::string)key) != this->clips.end();
		}

		float timeLength() const noexcept
		{
			return state.timeLength;
		}

		bool setDefaultClip(std::string_view key)
		{
			this->clip = this->getClip((std::string)key);
			return this->clip ? true : false;
		}

		void setTime(const _Time& time) noexcept
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

		_Time getTime() const noexcept
		{
			return this->state.time;
		}

		void evaluate(const _Time& delta) noexcept
		{
			this->state.time += delta;
			this->clip->evaluate(delta);
			this->state.finish = this->clip->finish;
		}

		bool empty() const noexcept
		{
			return this->clips.empty();
		}

		std::size_t size() const noexcept
		{
			return this->clips.size();
		}

		const std::shared_ptr<AnimationClip<_Time>>& operator[](const char* key) const
		{
			return this->clips.at(key);
		}

		const std::shared_ptr<AnimationClip<_Time>>& operator[](const std::string& key) const
		{
			return this->clips.at(key);
		}
	};
}

#endif