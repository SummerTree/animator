#include "ass_loader.h"
#include <octoon/ass_importer.h>

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
		auto importer = std::make_shared<octoon::ASSImporter>(path);
		auto object = importer->importer();
		
		if (object)
		{
			for (auto& it : object->downcast<octoon::GameObject>()->getChildren())
			{
				if (it->getComponent<octoon::CameraComponent>())
					profile.cameraModule->camera = it;
				else
					profile.entitiesModule->objects.getValue().push_back(it);
			}
		}
	}
}