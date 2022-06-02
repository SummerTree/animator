#ifndef OCTOON_ANIMATION_H_
#define OCTOON_ANIMATION_H_

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
	class Animation final
	{
	public:			
		std::string name;
		AnimatorStateInfo<_Time> state;
		AnimationClips<_Time> clips;

		Animation() noexcept
			: name("Default")
		{
			state.finish = false;
			state.time = 0;
			state.timeLength = 0;
		}

		Animation(AnimationClip<_Time>&& _clip) noexcept
			: Animation()
		{
			this->setClip(std::move(_clip));
		}

		Animation(AnimationClips<_Time>&& _clips) noexcept
			: Animation()
		{
			this->setClips(std::move(_clips));
		}

		Animation(const AnimationClip<_Time>& _clip) noexcept
			: Animation()
		{
			this->setClip(_clip);
		}

		Animation(const AnimationClips<_Time>& _clips) noexcept
			: Animation()
		{
			this->setClips(_clips);
		}

		Animation(std::string&& _name, AnimationClips<_Time>&& _clips) noexcept
			: name(std::move(_name))
			, clips(std::move(_clips))
		{
			state.finish = false;
			state.time = 0;

			for (auto& clip : clips)
				state.timeLength = std::max(clip.timeLength, state.timeLength);
		}

		Animation(std::string_view _name, const AnimationClips<_Time>& _clips) noexcept
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

		void addClip(AnimationClip<_Time>&& clip) noexcept
		{
			this->clips.push_back(std::move(clip));
			this->clips.back().setTime(state.time);

			state.timeLength = std::max(this->clips.back().timeLength, state.timeLength);
		}

		void addClip(const AnimationClip<_Time>& clip) noexcept
		{
			this->clips.push_back(clip);
			this->clips.back().setTime(state.time);

			state.timeLength = std::max(clip.timeLength, state.timeLength);
		}

		void setClip(AnimationClip<_Time>&& clip) noexcept
		{
			state.time = 0;
			state.timeLength = 0;
			state.finish = true;

			this->clips.clear();
			this->clips.push_back(std::move(clip));

			for (auto& it : clips)
			{
				it.evaluate(0.0f);

				state.finish &= it.finish;
				state.timeLength = std::max(it.timeLength, state.timeLength);
			}
		}

		void setClip(const AnimationClip<_Time>& clip) noexcept
		{
			state.time = 0;
			state.timeLength = 0;
			state.finish = true;

			this->clips.clear();
			this->clips.push_back(clip);

			for (auto& it : clips)
			{
				it.evaluate(0.0f);

				state.finish &= it.finish;
				state.timeLength = std::max(it.timeLength, state.timeLength);
			}
		}

		void setClips(const std::vector<AnimationClip<_Time>>& clip) noexcept
		{
			state.time = 0;
			state.timeLength = 0;
			state.finish = true;

			this->clips = clip;

			for (auto& it : clips)
			{
				it.evaluate(0.0f);

				state.finish &= it.finish;
				state.timeLength = std::max(it.timeLength, state.timeLength);
			}
		}

		void setClips(std::vector<AnimationClip<_Time>>&& clip) noexcept
		{
			state.time = 0;
			state.timeLength = 0;
			state.finish = true;

			this->clips = std::move(clip);

			for (auto& it : clips)
			{
				it.evaluate(0.0f);

				state.finish &= it.finish;
				state.timeLength = std::max(it.timeLength, state.timeLength);
			}
		}

		float timeLength() const noexcept
		{
			return state.timeLength;
		}

		void setTime(const _Time& time) noexcept
		{
			if (this->state.time != time)
			{
				for (auto& it : this->clips)
					it.setTime(time);

				this->state.time = time;
				this->state.finish = true;

				for (auto& it : this->clips)
					this->state.finish &= it.finish;
			}
		}

		_Time getTime() const noexcept
		{
			return this->state.time;
		}

		void evaluate(const _Time& delta) noexcept
		{
			this->state.time += delta;
			this->state.finish = true;

			for (auto& it : this->clips)
			{
				it.evaluate(delta);
				this->state.finish &= it.finish;
			}
		}

		bool empty() const noexcept
		{
			return this->clips.empty();
		}

		std::size_t size() const noexcept
		{
			return this->clips.size();
		}
	};
}

#endif