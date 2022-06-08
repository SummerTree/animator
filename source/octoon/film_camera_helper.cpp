#include <octoon/film_camera_helper.h>
#include <octoon/film_camera_component.h>

namespace octoon
{
	GameObjectPtr
	FilmCameraHelper::create(float canvasWidth, float zoom)
	{
		auto object = GameObject::create(std::string_view("MainCamera"));

		auto camera = object->addComponent<FilmCameraComponent>();
		camera->setCameraType(CameraType::Main);
		camera->setCanvasWidth(canvasWidth);
		camera->setZoom(zoom);

		return object;
	}
}