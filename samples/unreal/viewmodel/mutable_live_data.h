#ifndef UNREAL_MUTABLE_LIVE_DATA_H_
#define UNREAL_MUTABLE_LIVE_DATA_H_

#include <octoon/runtime/sigslot.h>

namespace unreal
{
	template <typename T>
	class MutableLiveData
	{
	public:
		MutableLiveData()
		{
		}

		MutableLiveData(const T& value)
			: value_(value)
		{
		}

		void
		connect(std::function<void(const T&)> listener) noexcept
		{
			dispatchEvents_.connect(listener);
		}

		void
		disconnect() noexcept
		{
			dispatchEvents_ = octoon::runtime::signal<void(const T&)>();
		}

		void
		postValue()
		{
			dispatchEvents_.call_all_slots(value_);
		}

		void
		setValue(T value)
		{
			if (value_ != value)
			{
				value_ = value;
				this->postValue();
			}
		}

		const T&
		getValue() const noexcept
		{
			return value_;
		}

		operator const T&() const noexcept
		{
			return value_;
		}

		void
		operator=(const T& value) noexcept
		{
			this->setValue(value);
		}

		void
		operator+=(std::function<void(const T&)> listener) noexcept
		{
			dispatchEvents_.connect(listener);
		}

	private:
		T value_;
		octoon::runtime::signal<void(const T&)> dispatchEvents_;
	};
}

#endif