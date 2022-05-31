#ifndef UNREAL_CAMERA_MODULE_H_
#define UNREAL_CAMERA_MODULE_H_

#include <unreal_model.h>
#include <octoon/game_object.h>
#include <octoon/math/vector2.h>
#include <octoon/math/vector3.h>
#include <octoon/hal/graphics_texture.h>

namespace unreal
{
	class CameraModule final : public UnrealModule
	{
	public:
		CameraModule() noexcept;
		virtual ~CameraModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(octoon::runtime::json& reader, std::string_view path) noexcept override;
		virtual void save(octoon::runtime::json& reader, std::string_view path) noexcept override;

		virtual void disconnect() noexcept;

	private:
		CameraModule(const CameraModule&) = delete;
		CameraModule& operator=(const CameraModule&) = delete;

	public:
		MutableLiveData<bool> useDepthOfFiled;
		MutableLiveData<float> fov;
		MutableLiveData<float> focalLength;
		MutableLiveData<float> aperture;
		MutableLiveData<float> focusDistance;
		MutableLiveData<octoon::math::float3> translate;
		MutableLiveData<octoon::math::float3> rotation;
		MutableLiveData<std::string> animation;
		MutableLiveData<octoon::GameObjectPtr> camera;
	};
}

#endif