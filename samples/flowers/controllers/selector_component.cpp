#include "selector_component.h"
#include "flower_behaviour.h"
#include <octoon/mesh/cube_wireframe_mesh.h>
#include <octoon/material/mesh_color_material.h>

namespace flower
{
	SelectorComponent::SelectorComponent() noexcept
	{
	}

	SelectorComponent::~SelectorComponent() noexcept
	{
	}

	std::optional<octoon::RaycastHit>
	SelectorComponent::intersectObjects(float x, float y) noexcept
	{
		auto preofile = this->getContext()->profile;
		if (preofile->entitiesModule->camera)
		{
			auto cameraComponent = preofile->entitiesModule->camera->getComponent<octoon::CameraComponent>();
			if (cameraComponent)
			{
				octoon::Raycaster raycaster(cameraComponent->screenToRay(octoon::math::float2(x, y)));
				auto& intersects = raycaster.intersectObjects(preofile->entitiesModule->objects);
				if (!intersects.empty())
					return intersects[0];
			}
		}

		return std::nullopt;
	}

	void
	SelectorComponent::handleMouseDown(const octoon::input::InputEvent& event) noexcept
	{
		auto& model = this->getModel();
		auto& profile = this->getContext()->profile;

		auto selected = this->intersectObjects(event.button.x, event.button.y);
		if (model->selectedItem_ != selected)
		{
			model->selectedItem_ = selected;

			if (selected)
			{
				this->captureEvent();
				this->sendMessage("editor:selected", selected.value());
			}
			else
			{
				this->sendMessage("editor:selected");
			}
		}
	}

	void
	SelectorComponent::handleMouseMove(const octoon::input::InputEvent& event) noexcept
	{
	}

	void
	SelectorComponent::handleMouseHover(const octoon::input::InputEvent& event) noexcept
	{
		auto& model = this->getModel();
		auto& profile = this->getContext()->profile;

		auto hover = this->intersectObjects(event.motion.x, event.motion.y);
		if (model->selectedItemHover_ != hover)
		{
			model->selectedItemHover_ = hover;

			if (hover)
				this->sendMessage("editor:hover", hover.value());
			else
				this->sendMessage("editor:hover");
		}
	}

	void
	SelectorComponent::onEnable() noexcept
	{
		this->gizmoHoverMtl_ = std::make_shared<octoon::MeshColorMaterial>(octoon::math::float3(1.0f, 0.3f, 0.5f));
		this->gizmoSelectedMtl_ = std::make_shared<octoon::MeshColorMaterial>(octoon::math::float3(0.85f, 0.15f, 0.30f));

		gizmoHover_ = octoon::GameObject::create("GizmoHover");
		gizmoHover_->addComponent<octoon::MeshFilterComponent>(octoon::CubeWireframeMesh::create(1.0f, 1.0f, 1.0f));
		auto meshRenderHover = gizmoHover_->addComponent<octoon::MeshRendererComponent>(this->gizmoHoverMtl_);
		meshRenderHover->setVisible(false);
		meshRenderHover->setRenderOrder(1);

		gizmoSelected_ = octoon::GameObject::create("GizmoSelect");
		gizmoSelected_->addComponent<octoon::MeshFilterComponent>(octoon::CubeWireframeMesh::create(1.0f, 1.0f, 1.0f));
		auto meshRenderSelected = gizmoSelected_->addComponent<octoon::MeshRendererComponent>(this->gizmoSelectedMtl_);
		meshRenderSelected->setVisible(false);
		meshRenderSelected->setRenderOrder(1);
	}

	void
	SelectorComponent::onDisable() noexcept
	{
		this->gizmoHover_ = nullptr;
		this->gizmoSelected_ = nullptr;
	}

	void
	SelectorComponent::onMouseDown(const octoon::input::InputEvent& event) noexcept
	{
		if (event.button.button == octoon::input::InputButton::Left)
			this->handleMouseDown(event);
	}

	void
	SelectorComponent::onMouseUp(const octoon::input::InputEvent& event) noexcept
	{
		this->releaseEvent();
	}

	void
	SelectorComponent::onMouseMotion(const octoon::input::InputEvent& event) noexcept
	{
		auto inputFeature = this->getFeature<octoon::InputFeature>();
		if (inputFeature)
		{
			auto input = inputFeature->getInput();
			if (input)
			{
				if (this->isCapture())
					this->handleMouseMove(event);
				else if (!input->isButtonPressed(octoon::input::InputButton::Left))
					this->handleMouseHover(event);
			}
		}
	}

	void
	SelectorComponent::onUpdate() noexcept
	{
		auto& model = this->getModel();
		auto& profile = this->getContext()->profile;

		if (model->selectedItem_ && !profile->playerModule->isPlaying)
		{
			auto hit = model->selectedItem_.value();
			auto hitObject = hit.object.lock();
			if (hitObject)
			{
				octoon::MeshPtr mesh;
				auto skinnedMesh = hitObject->getComponent<octoon::SkinnedMeshRendererComponent>();
				if (skinnedMesh)
					mesh = skinnedMesh->getSkinnedMesh();
				else
				{
					auto meshFilter = hitObject->getComponent<octoon::MeshFilterComponent>();
					if (meshFilter)
						mesh = meshFilter->getMesh();
				}

				if (mesh)
				{
					auto gizmoTransform = this->gizmoSelected_->getComponent<octoon::TransformComponent>();
					gizmoTransform->setTransform(hitObject->getComponent<octoon::TransformComponent>()->getTransform());
					gizmoTransform->getComponent<octoon::MeshFilterComponent>()->setMesh(mesh);

					auto meshRenderer = this->gizmoSelected_->getComponent<octoon::MeshRendererComponent>();
					meshRenderer->setVisible(true);
					meshRenderer->clearMaterials();
					meshRenderer->setMaterial(this->gizmoSelectedMtl_, hit.mesh);
				}
			}
			else
			{
				model->selectedItem_.emplace();
				this->gizmoSelected_->getComponent<octoon::MeshRendererComponent>()->setVisible(false);
			}
		}
		else
		{
			gizmoSelected_->getComponent<octoon::MeshRendererComponent>()->setVisible(false);
		}

		if (model->selectedItemHover_ && model->selectedItem_ != model->selectedItemHover_ && !profile->playerModule->isPlaying)
		{
			if (model->selectedItemHover_)
			{
				auto hit = model->selectedItemHover_.value();
				auto hitObject = hit.object.lock();

				octoon::MeshPtr mesh;
				auto skinnedMesh = hit.object.lock()->getComponent<octoon::SkinnedMeshRendererComponent>();
				if (skinnedMesh)
					mesh = skinnedMesh->getSkinnedMesh();
				else
				{
					auto meshFilter = hit.object.lock()->getComponent<octoon::MeshFilterComponent>();
					if (meshFilter)
						mesh = meshFilter->getMesh();
				}

				if (mesh)
				{
					auto gizmoTransform = this->gizmoHover_->getComponent<octoon::TransformComponent>();
					gizmoTransform->setTransform(hitObject->getComponent<octoon::TransformComponent>()->getTransform());
					gizmoTransform->getComponent<octoon::MeshFilterComponent>()->setMesh(mesh);

					auto meshRenderer = this->gizmoHover_->getComponent<octoon::MeshRendererComponent>();
					meshRenderer->setVisible(true);
					meshRenderer->clearMaterials();
					meshRenderer->setMaterial(this->gizmoHoverMtl_, hit.mesh);
				}
			}
			else
			{
				model->selectedItemHover_.emplace();
				this->gizmoHover_->getComponent<octoon::MeshRendererComponent>()->setVisible(false);
			}
		}
		else
		{
			gizmoHover_->getComponent<octoon::MeshRendererComponent>()->setVisible(false);
		}
	}
}