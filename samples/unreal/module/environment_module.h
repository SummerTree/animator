#ifndef UNREAL_ENVIRONMENT_MODULE_H_
#define UNREAL_ENVIRONMENT_MODULE_H_

#include <unreal_model.h>
#include <octoon/math/vector2.h>
#include <octoon/math/vector3.h>
#include <octoon/game_object.h>
#include <octoon/hal/graphics_texture.h>

namespace unreal
{
	class EnvironmentModule final : public UnrealModule
	{
	public:
		EnvironmentModule() noexcept;
		virtual ~EnvironmentModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(octoon::runtime::json& reader, std::string_view path) noexcept override;
		virtual void save(octoon::runtime::json& writer, std::string_view path) noexcept override;

		virtual void disconnect() noexcept;

	private:
		EnvironmentModule(const EnvironmentModule&) = delete;
		EnvironmentModule& operator=(const EnvironmentModule&) = delete;

	public:
		MutableLiveData<bool> useTexture;
		MutableLiveData<bool> showBackground;
		MutableLiveData<float> intensity;
		MutableLiveData<octoon::math::float2> offset;
		MutableLiveData<octoon::math::float3> color;
		MutableLiveData<std::shared_ptr<octoon::GraphicsTexture>> texture;
		MutableLiveData<octoon::GameObjectPtr> environmentLight;
	};
}

#endif