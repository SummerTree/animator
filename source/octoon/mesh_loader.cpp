#include <octoon/mesh_loader.h>
#include <octoon/runtime/string.h>

#include <octoon/transform_component.h>
#include <octoon/solver_component.h>
#include <octoon/animator_component.h>
#include <octoon/skinned_morph_component.h>
#include <octoon/skinned_mesh_renderer_component.h>
#include <octoon/skinned_joint_renderer_component.h>
#include <octoon/rotation_limit_component.h>
#include <octoon/rigidbody_component.h>
#include <octoon/sphere_collider_component.h>
#include <octoon/box_collider_component.h>
#include <octoon/capsule_collider_component.h>
#include <octoon/configurable_joint_component.h>
#include <octoon/rotation_link_component.h>
#include <octoon/rotation_link_limit_component.h>
#include <octoon/cloth_component.h>

#include <octoon/io/fstream.h>
#include <octoon/pmx_loader.h>

namespace octoon
{
	void createBones(const Model& model, GameObjects& bones) noexcept(false)
	{
		bones.reserve(model.bones.size());

		for (auto& it : model.bones)
			bones.emplace_back(GameObject::create(it->getName()));

		for (std::size_t i = 0; i < model.bones.size(); i++)
		{
			auto it = model.bones[i];

			auto parent = it->getParent();
			if (parent >= 0 && parent < bones.size())
				bones[i]->setParent(bones[parent]);

			auto transform = bones[i]->getComponent<TransformComponent>();
			transform->setTranslate(it->getPosition());
			transform->setQuaternion(it->getRotation());
			
			auto additiveParent = it->getAdditiveParent();
			if (additiveParent >= 0 && additiveParent < bones.size())
			{
				auto limit = bones[i]->addComponent<RotationLinkLimitComponent>();
				limit->setTranslate(transform->getTranslate());
				limit->setQuaternion(transform->getQuaternion());
				limit->setLocalTranslate(transform->getLocalTranslate());
				limit->setLocalQuaternion(transform->getLocalQuaternion());
				limit->setAdditiveMoveRatio(it->getAdditiveMoveRatio());
				limit->setAdditiveRotationRatio(it->getAdditiveRotationRatio());
				limit->setAdditiveUseLocal(it->getAdditiveUseLocal());

				auto parentController = bones[additiveParent]->getComponent<RotationLinkComponent>();
				if (parentController)
					parentController->addBone(bones[i]);
				else
				{
					auto additiveTransform = bones[additiveParent]->getComponent<TransformComponent>();
					auto rotationLink = bones[additiveParent]->addComponent<RotationLinkComponent>(bones[i]);
					rotationLink->setTranslate(additiveTransform->getTranslate());
					rotationLink->setQuaternion(additiveTransform->getQuaternion());
					rotationLink->setLocalTranslate(additiveTransform->getLocalTranslate());
					rotationLink->setLocalQuaternion(additiveTransform->getLocalQuaternion());
				}
			}
		}
	}

	void createClothes(const Model& model, GameObjectPtr& meshes, const GameObjects& bones) noexcept(false)
	{
		for (auto& it : model.softbodies)
		{
			GameObjects collider;

			for (auto& body : bones)
			{
				auto rigidbody = body->getComponent<RigidbodyComponent>();
				if ((1 << rigidbody->getGroup()) & ~it->groupMask)
					continue;

				collider.push_back(body);
			}

			auto cloth = ClothComponent::create();
			cloth->setColliders(collider);
			cloth->setTotalMass(it->totalMass);
			cloth->setPinVertexIndices(it->pinVertexIndices);
			cloth->setSolverFrequency(300.0f);
			cloth->setEnableCCD(true);
			cloth->setMaterialId(it->materialIndex);

			if (!it->anchorRigidbodies.empty())
			{
				auto rigidbody = bones[it->anchorRigidbodies.front()];
				if (rigidbody)
				{
					if (rigidbody->getParent())
						cloth->setTarget(rigidbody->getParent()->downcast_pointer<GameObject>());
				}
			}
			
			meshes->addComponent(cloth);
		}
	}

	void createColliders(const Model& model, GameObjects& bones) noexcept(false)
	{
		for (auto& it : model.rigidbodies)
		{
			if (it->bone >= bones.size())
				continue;

			auto bone = bones[it->bone];
			auto baseTransformInverse = bone->getComponent<TransformComponent>()->getTransformInverse();
			auto localTransform = math::transformMultiply(baseTransformInverse, math::makeRotation(math::Quaternion(it->rotation), it->position));

			math::float3 translate;
			math::float3 scale;
			math::Quaternion rotation;
			localTransform.getTransform(translate, rotation, scale);

			if (it->shape == ShapeType::ShapeTypeSphere)
			{
				auto collider = bone->addComponent<SphereColliderComponent>(it->scale.x > 0.0f ? it->scale.x : math::EPSILON_E3);
				collider->setCenter(translate);
				collider->setQuaternion(rotation);
			}
			else if (it->shape == ShapeType::ShapeTypeSquare)
			{
				auto collider = bone->addComponent<BoxColliderComponent>(math::max(math::float3(0.001, 0.001, 0.001), it->scale * 2.0f));
				collider->setCenter(translate);
				collider->setQuaternion(rotation);
			}
			else if (it->shape == ShapeType::ShapeTypeCapsule)
			{
				auto collider = bone->addComponent<CapsuleColliderComponent>(it->scale.x > 0.0f ? it->scale.x : math::EPSILON_E3, it->scale.y);
				collider->setCenter(translate);
				collider->setQuaternion(rotation);
			}
		}
	}

	void createRigidbodies(const Model& model, GameObjects& bones) noexcept(false)
	{
		for (auto& it : model.rigidbodies)
		{
			if (it->bone >= bones.size())
				continue;

			auto bone = bones[it->bone];

			if (!bone->getComponent<RigidbodyComponent>())
			{
				auto component = bone->addComponent<RigidbodyComponent>();
				component->setName(it->name);
				component->setMass(it->mass);
				component->setGroup(it->group);
				component->setGroupMask(it->groupMask);
				component->setRestitution(it->elasticity);
				component->setStaticFriction(it->friction);
				component->setDynamicFriction(it->friction);
				component->setLinearDamping(it->movementDecay);
				component->setAngularDamping(it->rotationDecay);

				if (it->physicsOperation == 0)
					component->setIsKinematic(it->physicsOperation == 0);
				else
					component->setSleepThreshold(0.0f);

				component->wakeUp();
			}
		}
	}

	void createJoints(const Model& model, GameObjects& bones) noexcept(false)
	{
		for (auto& it : model.joints)
		{
			if (it->bodyIndexA  >= model.rigidbodies.size()|| it->bodyIndexB >= model.rigidbodies.size())
				continue;

			auto boneA = model.rigidbodies[it->bodyIndexA]->bone;
			auto boneB = model.rigidbodies[it->bodyIndexB]->bone;

			if (boneA >= model.bones.size() || boneB >= model.bones.size())
				continue;

			auto& bodyA = bones[boneA];
			auto& bodyB = bones[boneB];

			if (bodyA != bodyB && bodyA && bodyB)
			{
				auto joint = bodyA->addComponent<ConfigurableJointComponent>();
				joint->setTargetPosition(it->position);
				joint->setTargetRotation(math::Quaternion(it->rotation));
				joint->setTarget(bodyB->getComponent<RigidbodyComponent>());
				joint->enablePreprocessing(false);

				if (it->movementLowerLimit.x == 0.0f && it->movementUpperLimit.x == 0.0f)
					joint->setXMotion(ConfigurableJointMotion::Locked);
				else if (it->movementLowerLimit.x > it->movementUpperLimit.x)
					joint->setXMotion(ConfigurableJointMotion::Free);
				else
				{
					joint->setHighXLimit(it->movementUpperLimit.x);
					joint->setLowXLimit(it->movementLowerLimit.x);
					joint->setXMotion(ConfigurableJointMotion::Limited);
				}

				if (it->movementLowerLimit.y == 0.0f && it->movementUpperLimit.y == 0.0f)
					joint->setYMotion(ConfigurableJointMotion::Locked);
				else if (it->movementLowerLimit.y > it->movementUpperLimit.y)
					joint->setYMotion(ConfigurableJointMotion::Free);
				else
				{
					joint->setHighYLimit(it->movementUpperLimit.y);
					joint->setLowYLimit(it->movementLowerLimit.y);
					joint->setYMotion(ConfigurableJointMotion::Limited);
				}

				if (it->movementLowerLimit.z == 0.0f && it->movementUpperLimit.z == 0.0f)
					joint->setZMotion(ConfigurableJointMotion::Locked);
				else if (it->movementLowerLimit.z > it->movementUpperLimit.z)
					joint->setZMotion(ConfigurableJointMotion::Free);
				else
				{
					joint->setHighZLimit(it->movementUpperLimit.z);
					joint->setLowZLimit(it->movementLowerLimit.z);
					joint->setZMotion(ConfigurableJointMotion::Limited);
				}

				if (it->rotationLowerLimit.x == 0.0f && it->rotationUpperLimit.x == 0.0f)
					joint->setAngularXMotion(ConfigurableJointMotion::Locked);
				else if (it->rotationLowerLimit.x > it->rotationUpperLimit.x)
					joint->setAngularXMotion(ConfigurableJointMotion::Free);
				else
					joint->setAngularXMotion(ConfigurableJointMotion::Limited);

				if (it->rotationLowerLimit.y == 0.0f && it->rotationUpperLimit.y == 0.0f)
					joint->setAngularYMotion(ConfigurableJointMotion::Locked);
				else if (it->rotationLowerLimit.y > it->rotationUpperLimit.y)
					joint->setAngularYMotion(ConfigurableJointMotion::Free);
				else
					joint->setAngularYMotion(ConfigurableJointMotion::Limited);

				if (it->rotationLowerLimit.z == 0.0f && it->rotationUpperLimit.z == 0.0f)
					joint->setAngularZMotion(ConfigurableJointMotion::Locked);
				else if (it->rotationLowerLimit.z > it->rotationUpperLimit.z)
					joint->setAngularZMotion(ConfigurableJointMotion::Free);
				else
					joint->setAngularZMotion(ConfigurableJointMotion::Limited);

				joint->setAngularLimit(it->rotationLowerLimit.x, it->rotationUpperLimit.x, it->rotationLowerLimit.y, it->rotationUpperLimit.y, it->rotationLowerLimit.z, it->rotationUpperLimit.z);
				
				joint->setDriveMotionX(it->springMovementConstant.x);
				joint->setDriveMotionY(it->springMovementConstant.y);
				joint->setDriveMotionZ(it->springMovementConstant.z);

				joint->setDriveAngularX(it->springRotationConstant.x);
				joint->setDriveAngularY(it->springRotationConstant.y);
				joint->setDriveAngularZ(it->springRotationConstant.z);
			}
		}
	}

	void createSolver(const Model& model, GameObjects& bones) noexcept(false)
	{
		for (auto& it : model.iks)
		{
			auto iksolver = std::make_shared<CCDSolverComponent>();
			iksolver->setTarget(bones[it->targetBoneIndex]);
			iksolver->setIterations(it->iterations);
			iksolver->setAutomaticUpdate(false);

			for (auto& child : it->child)
			{
				if (child.rotateLimited)
					bones[child.boneIndex]->addComponent<RotationLimitComponent>(-child.angleRadian, child.angleRadian, child.minimumRadian, child.maximumRadian);
				else
					bones[child.boneIndex]->addComponent<RotationLimitComponent>(-child.angleRadian, child.angleRadian, math::float3::Zero, math::float3::Zero);

				iksolver->addBone(bones[child.boneIndex]);
			}

			bones[it->boneIndex]->addComponent(std::move(iksolver));
		}
	}

	void createMorph(const Model& model, GameObjectPtr& mesh) noexcept(false)
	{
		for (auto& it : model.morphs)
		{
			math::float3s offsets;
			math::uint1s indices;

			for (auto& v : it->vertices)
			{
				offsets.push_back(v.offset);
				indices.push_back(v.index);
			}

			auto animation = mesh->addComponent<SkinnedMorphComponent>();
			animation->setName(it->name);
			animation->setOffsets(offsets);
			animation->setIndices(indices);
		}
	}

	void createMaterials(const Model& model, Materials& materials) noexcept(false)
	{
		materials.reserve(model.materials.size());

		for (auto& it : model.materials)
			materials.push_back(it);
	}

	void createMeshes(const Model& model, GameObjectPtr& object, const GameObjects& bones) noexcept(false)
	{
		Materials materials;
		createMaterials(model, materials);

		object->addComponent<MeshFilterComponent>(model.meshes[0]);

		if (bones.empty())
		{
			auto meshRender = object->addComponent<MeshRendererComponent>(materials);
			meshRender->setGlobalIllumination(true);
		}
		else
		{
			auto smr = SkinnedMeshRendererComponent::create();
			smr->setMaterials(materials);
			smr->setTransforms(bones);
			smr->setMorphBlendEnable(true);
			smr->setTextureBlendEnable(true);
			smr->setGlobalIllumination(true);

			object->addComponent(smr);
		}
	}

	std::shared_ptr<Geometry>
	MeshLoader::load(const Model& model) noexcept(false)
	{
		Materials materials;
		createMaterials(model, materials);

		auto geometry = std::make_shared<Geometry>();
		geometry->setMesh(model.meshes[0]);
		geometry->setMaterials(materials);

		return geometry;
	}

	GameObjectPtr
	MeshLoader::load(std::string_view filepath, bool cache) noexcept(false)
	{
		Model model;

		PmxLoader load;
		load.load(filepath, model);

		if (!model.meshes.empty())
		{
			GameObjectPtr actor = GameObject::create();

			GameObjects bones;

			createBones(model, bones);
			createSolver(model, bones);
			createColliders(model, bones);
			createRigidbodies(model, bones);
			createJoints(model, bones);
			
			createMeshes(model, actor, bones);
			createMorph(model, actor);
			createClothes(model, actor, bones);

			return actor;
		}

		return nullptr;
	}
}