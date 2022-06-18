#include "ass_loader.h"
#include <octoon/ass_loader.h>

namespace unreal
{
	AssLoader::AssLoader() noexcept
	{
	}

	AssLoader::~AssLoader() noexcept
	{
	}
	
	void
	AssLoader::load(UnrealProfile& profile, const std::filesystem::path& path) noexcept(false)
	{
		for (auto& it : octoon::ASSLoader::load(path))
		{
			if (it->getComponent<octoon::CameraComponent>())
				profile.cameraModule->camera = it;
			else
			{
				auto renderer = it->getComponent<octoon::MeshRendererComponent>();
				if (renderer)
					renderer->setGlobalIllumination(true);

				profile.entitiesModule->objects.getValue().push_back(it);
			}
		}
	}
}