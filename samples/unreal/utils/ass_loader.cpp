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
	AssLoader::load(UnrealProfile& profile, std::string_view path) noexcept(false)
	{
		octoon::GameObjects objects = octoon::ASSLoader::load(path);

		for (auto& it : objects)
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