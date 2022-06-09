#include "grid_component.h"
#include "unreal_behaviour.h"
#include <octoon/material/material.h>
#include <octoon/mesh/cube_wireframe_mesh.h>

namespace unreal
{
	GridComponent::GridComponent() noexcept
	{
	}

	GridComponent::~GridComponent() noexcept
	{
	}

	void
	GridComponent::onEnable() noexcept
	{
		auto material = std::make_shared<octoon::LineBasicMaterial>(octoon::math::float3(0.5f, 0.5f, 0.5f));
		material->setDepthEnable(false);
		material->setDepthWriteEnable(false);

		this->gizmo_ = std::make_shared<octoon::GameObject>(std::string_view("CoordinateSystem"));
		this->gizmo_->setRaycastEnable(false);
		this->gizmo_->addComponent<octoon::MeshFilterComponent>(std::make_shared<octoon::PlaneMesh>(100.0f, 100.0f, 20, 20, true));
		this->gizmo_->addComponent<octoon::MeshRendererComponent>(material)->setRenderOrder(-1);

		auto transform = this->gizmo_->getComponent<octoon::TransformComponent>();
		transform->setQuaternion(octoon::math::Quaternion(octoon::math::float3(octoon::math::PI * 0.5f, 0, 0)));
		transform->setTranslate(octoon::math::float3::Zero);
	}

	void
	GridComponent::onDisable() noexcept
	{
		this->gizmo_.reset();
	}
}