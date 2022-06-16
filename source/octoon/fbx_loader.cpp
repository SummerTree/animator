#include <octoon/fbx_loader.h>
#include <octoon/asset_database.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/transform_component.h>
#include <octoon/point_light_component.h>
#include <octoon/film_camera_component.h>
#include <octoon/mesh_filter_component.h>
#include <octoon/mesh_renderer_component.h>
#include <ofbx.h>
#include <fstream>

namespace octoon
{
	FBXLoader::FBXLoader() noexcept
	{
	}

	FBXLoader::~FBXLoader() noexcept
	{
	}

	bool
	FBXLoader::doCanRead(io::istream& stream) noexcept
	{
		return false;
	}

	bool
	FBXLoader::doCanRead(const char* type) noexcept
	{
		return std::strncmp(type, "fbx", 3) == 0;
	}

	std::shared_ptr<GameObject>
	FBXLoader::load(std::istream& stream) noexcept(false)
	{
		return nullptr;
	}

	std::shared_ptr<GameObject>
	FBXLoader::load(const std::filesystem::path& filepath) noexcept(false)
	{
		std::ifstream stream(filepath);
		if (stream)
			return load(stream);

		return nullptr;
	}
}