#ifndef UNREAL_SUN_MODULE_H_
#define UNREAL_SUN_MODULE_H_

#include <unreal_model.h>
#include <octoon/math/vector3.h>

namespace unreal
{
	class SunModule final : public UnrealModule
	{
	public:
		SunModule() noexcept;
		virtual ~SunModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(octoon::runtime::json& reader) noexcept override;
		virtual void save(octoon::runtime::json& reader) noexcept override;

	private:
		SunModule(const SunModule&) = delete;
		SunModule& operator=(const SunModule&) = delete;

	public:
		MutableLiveData<float> intensity;
		MutableLiveData<float> size;
		MutableLiveData<octoon::math::float3> color;
		MutableLiveData<octoon::math::float3> rotation;
	};
}

#endif