#include "camera_module.h"

namespace unreal
{
	CameraModule::CameraModule() noexcept
	{
		this->reset();
	}

	CameraModule::~CameraModule() noexcept
	{
	}

	void
	CameraModule::reset() noexcept
	{
		this->useDepthOfFiled = false;
		this->fov = 60.0f;
		this->focalLength = 31.18f;
		this->aperture = 5.6f;
		this->focusDistance = 10.0f;
		this->translate = octoon::math::float3(0, 10, -10);
		this->rotation = octoon::math::float3::Zero;
	}

	void 
	CameraModule::load(octoon::runtime::json& reader) noexcept
	{
	}

	void 
	CameraModule::save(octoon::runtime::json& writer) noexcept
	{
	}

	void
	CameraModule::disconnect() noexcept
	{
		this->enable.disconnect();
		this->useDepthOfFiled.disconnect();
		this->fov.disconnect();
		this->focalLength.disconnect();
		this->aperture.disconnect();
		this->focusDistance.disconnect();
		this->translate.disconnect();
		this->rotation.disconnect();	
	}
}