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
		if (reader["useDepthOfFiled"].is_boolean())
			this->useDepthOfFiled = reader["useDepthOfFiled"].get<nlohmann::json::boolean_t>();
		if (reader["fov"].is_number_float())
			this->fov = reader["fov"].get<nlohmann::json::number_float_t>();
		if (reader["aperture"].is_number_float())
			this->aperture = reader["aperture"].get<nlohmann::json::number_float_t>();
		if (reader["focusDistance"].is_number_float())
			this->focusDistance = reader["focusDistance"].get<nlohmann::json::number_float_t>();
		if (reader["translate"].is_array())
		{
			this->translate = octoon::math::float3(reader["translate"][0], reader["translate"][1], reader["translate"][2]);
			this->translate.submit();
		}
		if (reader["rotation"].is_array())
		{
			this->rotation = octoon::math::float3(reader["rotation"][0], reader["rotation"][1], reader["rotation"][2]);
			this->rotation.submit();
		}
	}

	void 
	CameraModule::save(octoon::runtime::json& writer) noexcept
	{
		writer["useDepthOfFiled"] = this->useDepthOfFiled.getValue();
		writer["fov"] = this->fov.getValue();
		writer["aperture"] = this->aperture.getValue();
		writer["focusDistance"] = this->focusDistance.getValue();
		writer["translate"].push_back(this->translate.getValue().x);
		writer["translate"].push_back(this->translate.getValue().y);
		writer["translate"].push_back(this->translate.getValue().z);
		writer["rotation"].push_back(this->rotation.getValue().x);
		writer["rotation"].push_back(this->rotation.getValue().y);
		writer["rotation"].push_back(this->rotation.getValue().z);
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