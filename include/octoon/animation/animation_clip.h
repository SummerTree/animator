#ifndef OCTOON_ANIMATION_CLIP_H_
#define OCTOON_ANIMATION_CLIP_H_

#include <unordered_map>
#include <octoon/runtime/rtti_object.h>
#include <octoon/animation/animation_curve.h>

namespace octoon
{
	template<typename _Time = float>
	class AnimationClip final : public RttiObject
	{
	public:
		std::string name;
		std::unordered_map<std::string, std::unordered_map<std::string, AnimationCurve<_Time>>> bindings;
		bool finish;
		_Time timeLength;

		AnimationClip() noexcept
			: finish(false)
			, timeLength(0)
		{
		}

		explicit AnimationClip(std::string_view _name) noexcept
			: name(_name)
			, finish(false)
			, timeLength(0)
		{				
		}

		void setName(std::string_view _name) noexcept
		{
			this->name = _name;
		}

		void setCurve(std::string_view relativePath, std::string_view propertyName, AnimationCurve<_Time>&& curve) noexcept
		{
			this->bindings[std::string(relativePath)][std::string(propertyName)] = std::move(curve);

			this->timeLength = 0;

			for (auto& binding : this->bindings)
			{
				for (auto& it : binding.second)
					timeLength = std::max(it.second.timeLength, timeLength);
			}
		}

		void setCurve(std::string_view relativePath, std::string_view propertyName, const AnimationCurve<_Time>& curve) noexcept
		{
			this->bindings[std::string(relativePath)][std::string(propertyName)] = curve;

			this->timeLength = 0;

			for (auto& binding : this->bindings)
			{
				for (auto& it : binding.second)
					timeLength = std::max(it.second.timeLength, timeLength);
			}
		}

		bool hasCurve(std::string_view relativePath) const noexcept
		{
			return this->bindings.find(relativePath) != this->bindings.end();
		}

		AnimationCurve<_Time>& getCurve(std::string_view relativePath, std::string_view propertyName) noexcept
		{
			return this->bindings.at(relativePath).at(propertyName);
		}

		const AnimationCurve<_Time>& getCurve(std::string_view relativePath, std::string_view propertyName) const noexcept
		{
			return this->bindings.at(relativePath).at(propertyName);
		}

		void clearCurve() noexcept
		{
			this->timeLength = 0;
			this->bindings.clear();
		}

		bool empty() const noexcept
		{
			return this->bindings.empty();
		}

		std::size_t size() const noexcept
		{
			return this->bindings.size();
		}

		void evaluate(const _Time& delta) noexcept
		{
			this->finish = true;

			for (auto& binding : this->bindings)
			{
				for (auto& curve : binding.second)
				{
					curve.second.evaluate(delta);
					this->finish &= curve.second.finish;
				}
			}
		}

		void setTime(const _Time& time) noexcept
		{
			for (auto& binding : this->bindings)
			{
				for (auto& curve : binding.second)
					curve.second.setTime(time);
			}				

			this->finish = true;

			for (auto& binding : this->bindings)
			{
				for (auto& curve : binding.second)
					this->finish &= curve.second.finish;
			}
		}
	};

	template<typename _Time = float>
	using AnimationClips = std::vector<AnimationClip<_Time>>;
}

#endif