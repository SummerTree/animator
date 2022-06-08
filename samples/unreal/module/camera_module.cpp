#include "camera_module.h"
#include "../importer/motion_importer.h"
#include <octoon/vmd_loader.h>
#include <octoon/io/fstream.h>

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
		this->framebufferSize = octoon::math::uint2(1280, 720);
		this->translate = octoon::math::float3(0, 10, -10);
		this->rotation = octoon::math::float3::Zero;
		this->animation = nullptr;
	}

	void 
	CameraModule::load(octoon::runtime::json& reader, std::string_view path) noexcept
	{
		if (reader.contains("useDepthOfFiled"))
			this->useDepthOfFiled = reader["useDepthOfFiled"].get<nlohmann::json::boolean_t>();
		if (reader["fov"].is_number_float())
			this->fov = reader["fov"].get<nlohmann::json::number_float_t>();
		if (reader["aperture"].is_number_float())
			this->aperture = reader["aperture"].get<nlohmann::json::number_float_t>();
		if (reader["focusDistance"].is_number_float())
			this->focusDistance = reader["focusDistance"].get<nlohmann::json::number_float_t>();
		if (reader["size"].is_array())
			this->framebufferSize = octoon::math::uint2(reader["size"].get<std::array<std::uint32_t, 2>>());
		if (reader["translate"].is_array())
			this->translate = octoon::math::float3(reader["translate"].get<std::array<float, 3>>());
		if (reader["rotation"].is_array())
			this->rotation = octoon::math::float3(reader["rotation"].get<std::array<float, 3>>());
		if (reader["animation"].is_object())
		{
			auto& animationJson = reader["animation"];
			if (animationJson.find("path") != animationJson.end())
			{
				auto filepath = animationJson["path"].get<nlohmann::json::string_t>();
				this->animation = octoon::VMDLoader::loadCameraMotion((char8_t*)filepath.c_str());
			}
		}			
	}

	void 
	CameraModule::save(octoon::runtime::json& writer, std::string_view path) noexcept
	{
		writer["useDepthOfFiled"] = this->useDepthOfFiled.getValue();
		writer["fov"] = this->fov.getValue();
		writer["aperture"] = this->aperture.getValue();
		writer["focusDistance"] = this->focusDistance.getValue();
		writer["size"] = this->framebufferSize.getValue().to_array();
		writer["translate"] = this->translate.getValue().to_array();
		writer["rotation"] = this->rotation.getValue().to_array();

		if (this->animation.getValue() && !this->animation.getValue()->clips.empty())
		{
			nlohmann::json animationJson = MotionImporter::instance()->createMetadata(this->animation.getValue());
			if (!animationJson.is_object())
			{
				auto root = std::string(path).substr(0, path.find_last_of('/')) + "/Assets/" + this->animation.getValue()->name + ".vmd";
				octoon::VMDLoader::saveCameraMotion(root, *this->animation.getValue());
				animationJson["path"] = root;
			}

			writer["animation"] = std::move(animationJson);
		}
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
		this->animation.disconnect();
	}
}