#include <octoon/pmx_loader.h>
#include <octoon/pmx.h>

#include <octoon/runtime/string.h>
#include <octoon/material/mesh_standard_material.h>

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
#include <octoon/texture_loader.h>

#include <set>
#include <codecvt>

namespace octoon
{
	void createBones(const PMX& pmx, GameObjects& bones) noexcept(false)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

		bones.reserve(pmx.bones.size());

		for (auto& it : pmx.bones)
			bones.emplace_back(GameObject::create(cv.to_bytes(it.name.name)));

		for (std::size_t i = 0; i < pmx.bones.size(); i++)
		{
			auto& it = pmx.bones[i];

			auto parent = it.Parent;
			if (parent >= 0 && parent < bones.size())
				bones[i]->setParent(bones[parent]);

			auto transform = bones[i]->getComponent<TransformComponent>();
			transform->setTranslate(math::float3(it.position.x, it.position.y, it.position.z));

			auto additiveParent = it.ProvidedParentBoneIndex;
			if (additiveParent >= 0 && additiveParent < bones.size())
			{
				auto limit = bones[i]->addComponent<RotationLinkLimitComponent>();
				limit->setTranslate(transform->getTranslate());
				limit->setQuaternion(transform->getQuaternion());
				limit->setLocalTranslate(transform->getLocalTranslate());
				limit->setLocalQuaternion(transform->getLocalQuaternion());
				limit->setAdditiveUseLocal(!(it.Flag & PMX_BONE_ADD_LOCAL));

				if (it.Flag & PMX_BONE_ADD_MOVE)
					limit->setAdditiveMoveRatio(it.ProvidedRatio);
				if (it.Flag & PMX_BONE_ADD_ROTATION)
					limit->setAdditiveRotationRatio(it.ProvidedRatio);

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

			if (it.Flag & PMX_BONE_IK)
			{
				auto solver = std::make_shared<CCDSolverComponent>();
				solver->setTarget(bones[it.IKTargetBoneIndex]);
				solver->setIterations(it.IKLoopCount);
				solver->setAutomaticUpdate(false);

				for (auto& child : it.IKList)
				{
					if (child.rotateLimited)
					{
						math::float3 minimumRadian = math::float3(child.minimumRadian.x, child.minimumRadian.y, child.minimumRadian.z);
						math::float3 maximumRadian = math::float3(child.maximumRadian.x, child.maximumRadian.y, child.maximumRadian.z);
						bones[child.BoneIndex]->addComponent<RotationLimitComponent>(-it.IKLimitedRadian, it.IKLimitedRadian, minimumRadian, maximumRadian);
					}
					else
					{
						bones[child.BoneIndex]->addComponent<RotationLimitComponent>(-it.IKLimitedRadian, it.IKLimitedRadian, math::float3::Zero, math::float3::Zero);
					}

					solver->addBone(bones[child.BoneIndex]);
				}

				bones[i]->addComponent(std::move(solver));
			}
		}
	}

	void createClothes(const PMX& pmx, GameObjectPtr& meshes, const GameObjects& bones) noexcept(false)
	{
		for (auto& it : pmx.softbodies)
		{
			GameObjects collider;

			for (auto& body : bones)
			{
				auto rigidbody = body->getComponent<RigidbodyComponent>();
				if ((1 << rigidbody->getGroup()) & ~it.groupMask)
					continue;

				collider.push_back(body);
			}

			math::uint1s pinVertexIndices;

			for (std::size_t i = 0; i < it.numIndices; i++)
			{
				std::uint32_t index = 0;
				if (pmx.header.sizeOfIndices == 1)
					index = *((std::uint8_t*)it.pinVertexIndices.data() + i);
				else if (pmx.header.sizeOfIndices == 2)
					index = *((std::uint16_t*)it.pinVertexIndices.data() + i);
				else if (pmx.header.sizeOfIndices == 4)
					index = *((std::uint32_t*)it.pinVertexIndices.data() + i);

				pinVertexIndices.push_back(index);
			}

			auto cloth = ClothComponent::create();
			cloth->setColliders(collider);
			cloth->setTotalMass(it.totalMass);
			cloth->setPinVertexIndices(pinVertexIndices);
			cloth->setSolverFrequency(300.0f);
			cloth->setEnableCCD(true);
			cloth->setMaterialId(it.materialIndex);

			if (!it.anchorRigidbodies.empty())
			{
				auto rigidbody = bones[it.anchorRigidbodies[0].rigidBodyIndex];
				if (rigidbody)
				{
					if (rigidbody->getParent())
						cloth->setTarget(rigidbody->getParent()->downcast_pointer<GameObject>());
				}
			}
			
			meshes->addComponent(cloth);
		}
	}

	void createColliders(const PMX& pmx, GameObjects& bones) noexcept(false)
	{
		for (auto& it : pmx.rigidbodies)
		{
			if (it.bone >= bones.size())
				continue;

			auto bone = bones[it.bone];
			auto baseTransformInverse = bone->getComponent<TransformComponent>()->getTransformInverse();
			auto localTransform = math::transformMultiply(baseTransformInverse, math::makeRotation(math::Quaternion(octoon::math::float3(it.rotate.x, it.rotate.y, it.rotate.z)), octoon::math::float3(it.position.x, it.position.y, it.position.z)));

			math::float3 translate;
			math::float3 scale;
			math::Quaternion rotation;
			localTransform.getTransform(translate, rotation, scale);

			if (it.shape == PmxShapeType::ShapeTypeSphere)
			{
				auto collider = bone->addComponent<SphereColliderComponent>(it.scale.x > 0.0f ? it.scale.x : math::EPSILON_E3);
				collider->setCenter(translate);
				collider->setQuaternion(rotation);
			}
			else if (it.shape == PmxShapeType::ShapeTypeSquare)
			{
				auto collider = bone->addComponent<BoxColliderComponent>(math::max(math::float3(0.001, 0.001, 0.001), math::float3(it.scale.x, it.scale.y, it.scale.z) * 2.0f));
				collider->setCenter(translate);
				collider->setQuaternion(rotation);
			}
			else if (it.shape == PmxShapeType::ShapeTypeCapsule)
			{
				auto collider = bone->addComponent<CapsuleColliderComponent>(it.scale.x > 0.0f ? it.scale.x : math::EPSILON_E3, it.scale.y);
				collider->setCenter(translate);
				collider->setQuaternion(rotation);
			}
		}
	}

	void createRigidbodies(const PMX& pmx, GameObjects& bones) noexcept(false)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

		for (auto& it : pmx.rigidbodies)
		{
			if (it.bone >= bones.size())
				continue;

			auto bone = bones[it.bone];

			if (!bone->getComponent<RigidbodyComponent>())
			{
				auto component = bone->addComponent<RigidbodyComponent>();
				component->setName(cv.to_bytes(it.name.name));
				component->setMass(it.mass);
				component->setGroup(it.group);
				component->setGroupMask(it.groupMask);
				component->setRestitution(it.elasticity);
				component->setStaticFriction(it.friction);
				component->setDynamicFriction(it.friction);
				component->setLinearDamping(it.movementDecay);
				component->setAngularDamping(it.rotationDecay);

				if (it.physicsOperation == 0)
					component->setIsKinematic(it.physicsOperation == 0);
				else
					component->setSleepThreshold(0.0f);

				component->wakeUp();
			}
		}
	}

	void createJoints(const PMX& pmx, GameObjects& bones) noexcept(false)
	{
		for (auto& it : pmx.joints)
		{
			if (it.relatedRigidBodyIndexA >= pmx.rigidbodies.size()|| it.relatedRigidBodyIndexB >= pmx.rigidbodies.size())
				continue;

			auto boneA = pmx.rigidbodies[it.relatedRigidBodyIndexA].bone;
			auto boneB = pmx.rigidbodies[it.relatedRigidBodyIndexB].bone;

			if (boneA >= pmx.bones.size() || boneB >= pmx.bones.size())
				continue;

			auto& bodyA = bones[boneA];
			auto& bodyB = bones[boneB];

			if (bodyA != bodyB && bodyA && bodyB)
			{
				auto joint = bodyA->addComponent<ConfigurableJointComponent>();
				joint->setTargetPosition(math::float3(it.position.x, it.position.y, it.position.z));
				joint->setTargetRotation(math::Quaternion(math::float3(it.rotation.x, it.rotation.y, it.rotation.z)));
				joint->setTarget(bodyB->getComponent<RigidbodyComponent>());
				joint->enablePreprocessing(false);

				if (it.movementLowerLimit.x == 0.0f && it.movementUpperLimit.x == 0.0f)
					joint->setXMotion(ConfigurableJointMotion::Locked);
				else if (it.movementLowerLimit.x > it.movementUpperLimit.x)
					joint->setXMotion(ConfigurableJointMotion::Free);
				else
				{
					joint->setHighXLimit(it.movementUpperLimit.x);
					joint->setLowXLimit(it.movementLowerLimit.x);
					joint->setXMotion(ConfigurableJointMotion::Limited);
				}

				if (it.movementLowerLimit.y == 0.0f && it.movementUpperLimit.y == 0.0f)
					joint->setYMotion(ConfigurableJointMotion::Locked);
				else if (it.movementLowerLimit.y > it.movementUpperLimit.y)
					joint->setYMotion(ConfigurableJointMotion::Free);
				else
				{
					joint->setHighYLimit(it.movementUpperLimit.y);
					joint->setLowYLimit(it.movementLowerLimit.y);
					joint->setYMotion(ConfigurableJointMotion::Limited);
				}

				if (it.movementLowerLimit.z == 0.0f && it.movementUpperLimit.z == 0.0f)
					joint->setZMotion(ConfigurableJointMotion::Locked);
				else if (it.movementLowerLimit.z > it.movementUpperLimit.z)
					joint->setZMotion(ConfigurableJointMotion::Free);
				else
				{
					joint->setHighZLimit(it.movementUpperLimit.z);
					joint->setLowZLimit(it.movementLowerLimit.z);
					joint->setZMotion(ConfigurableJointMotion::Limited);
				}

				if (it.rotationLowerLimit.x == 0.0f && it.rotationUpperLimit.x == 0.0f)
					joint->setAngularXMotion(ConfigurableJointMotion::Locked);
				else if (it.rotationLowerLimit.x > it.rotationUpperLimit.x)
					joint->setAngularXMotion(ConfigurableJointMotion::Free);
				else
					joint->setAngularXMotion(ConfigurableJointMotion::Limited);

				if (it.rotationLowerLimit.y == 0.0f && it.rotationUpperLimit.y == 0.0f)
					joint->setAngularYMotion(ConfigurableJointMotion::Locked);
				else if (it.rotationLowerLimit.y > it.rotationUpperLimit.y)
					joint->setAngularYMotion(ConfigurableJointMotion::Free);
				else
					joint->setAngularYMotion(ConfigurableJointMotion::Limited);

				if (it.rotationLowerLimit.z == 0.0f && it.rotationUpperLimit.z == 0.0f)
					joint->setAngularZMotion(ConfigurableJointMotion::Locked);
				else if (it.rotationLowerLimit.z > it.rotationUpperLimit.z)
					joint->setAngularZMotion(ConfigurableJointMotion::Free);
				else
					joint->setAngularZMotion(ConfigurableJointMotion::Limited);

				joint->setAngularLimit(it.rotationLowerLimit.x, it.rotationUpperLimit.x, it.rotationLowerLimit.y, it.rotationUpperLimit.y, it.rotationLowerLimit.z, it.rotationUpperLimit.z);
				
				joint->setDriveMotionX(it.springMovementConstant.x);
				joint->setDriveMotionY(it.springMovementConstant.y);
				joint->setDriveMotionZ(it.springMovementConstant.z);

				joint->setDriveAngularX(it.springRotationConstant.x);
				joint->setDriveAngularY(it.springRotationConstant.y);
				joint->setDriveAngularZ(it.springRotationConstant.z);
			}
		}
	}

	void createMorph(const PMX& pmx, GameObjectPtr& mesh) noexcept(false)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

		for (auto& it : pmx.morphs)
		{
			switch (it.morphType)
			{
			case PmxMorphType::PMX_MorphTypeVertex:
			{
				math::float3s offsets;
				math::uint1s indices;

				for (auto& v : it.vertices)
				{
					offsets.push_back(math::float3(v.offset.x, v.offset.y, v.offset.z));
					indices.push_back(v.index);
				}

				auto animation = mesh->addComponent<SkinnedMorphComponent>();
				animation->setName(cv.to_bytes(it.name.name));
				animation->setOffsets(std::move(offsets));
				animation->setIndices(std::move(indices));
			}
			break;
			}
		}
	}

	void createMaterials(const PMX& pmx, Materials& materials) noexcept(false)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

		materials.reserve(pmx.materials.size());

		for (auto& it : pmx.materials)
		{
			auto material = std::make_shared<MeshStandardMaterial>();
			material->setName(it.name.length > 0 ? cv.to_bytes(it.name.name) : "Untitled");
			material->setColor(math::srgb2linear(math::float3(it.Diffuse.x, it.Diffuse.y, it.Diffuse.z)));
			material->setOpacity(it.Opacity);

			std::uint32_t limits = 0;
			if (pmx.header.sizeOfTexture == 1)
				limits = std::numeric_limits<PmxUInt8>::max();
			else if (pmx.header.sizeOfTexture == 2)
				limits = std::numeric_limits<PmxUInt16>::max();
			else if (pmx.header.sizeOfTexture == 4)
				limits = std::numeric_limits<PmxUInt32>::max();

			try
			{
				if (it.TextureIndex < limits)
					material->setColorMap(TextureLoader::load(pmx.textures[it.TextureIndex].fullpath));
			}
			catch (...)
			{
			}

			bool hasAlphaTexture = it.TextureIndex < limits ? std::wstring_view(pmx.textures[it.TextureIndex].name).find(L".png") != std::string::npos : false;

			auto colorMap = material->getColorMap();
			if (colorMap)
			{
				auto textureFormat = material->getColorMap()->getTextureDesc().getTexFormat();
				if (textureFormat == GraphicsFormat::B8G8R8A8SRGB ||
					textureFormat == GraphicsFormat::B8G8R8A8UNorm ||
					textureFormat == GraphicsFormat::R8G8B8A8SRGB ||
					textureFormat == GraphicsFormat::R8G8B8A8UNorm)
				{
					hasAlphaTexture = true;
				}
			}

			if (it.Opacity < 1.0 || hasAlphaTexture) {
				material->setBlendEnable(true);
				material->setBlendSrc(BlendMode::SrcAlpha);
				material->setBlendDest(BlendMode::OneMinusSrcAlpha);
			}

			materials.emplace_back(std::move(material));
		}
	}

	void createMeshes(const PMX& pmx, GameObjectPtr& object, const GameObjects& bones) noexcept(false)
	{
		Materials materials;
		createMaterials(pmx, materials);

		math::float4x4s bindposes(pmx.bones.size());
		for (std::size_t i = 0; i < pmx.bones.size(); i++)
			bindposes[i].makeTranslate(-math::float3(pmx.bones[i].position.x, pmx.bones[i].position.y, pmx.bones[i].position.z));

		math::float3s vertices_;
		math::float3s normals_;
		math::float2s texcoords_;
		std::vector<VertexWeight> weights;
		std::vector<std::shared_ptr<Mesh>> meshes;

		vertices_.resize(pmx.numVertices);
		normals_.resize(pmx.numVertices);
		texcoords_.resize(pmx.numVertices);

		if (pmx.numBones)
			weights.resize(pmx.numVertices);

		for (std::size_t i = 0; i < pmx.numVertices; i++)
		{
			auto& v = pmx.vertices[i];

			vertices_[i].set(v.position.x, v.position.y, v.position.z);
			normals_[i].set(v.normal.x, v.normal.y, v.normal.z);
			texcoords_[i].set(v.coord.x, v.coord.y);

			if (pmx.numBones)
			{
				VertexWeight weight;
				weight.weight1 = v.weight.weight1;
				weight.weight2 = v.weight.weight2;
				weight.weight3 = v.weight.weight3;
				weight.weight4 = v.weight.weight4;
				weight.bone1 = v.weight.bone1 < pmx.numBones ? v.weight.bone1 : 0;
				weight.bone2 = v.weight.bone2 < pmx.numBones ? v.weight.bone2 : 0;
				weight.bone3 = v.weight.bone3 < pmx.numBones ? v.weight.bone3 : 0;
				weight.bone4 = v.weight.bone4 < pmx.numBones ? v.weight.bone4 : 0;

				weights[i] = weight;
			}
		}

		auto mesh = std::make_shared<Mesh>();
		mesh->setBindposes(std::move(bindposes));
		mesh->setVertexArray(std::move(vertices_));
		mesh->setNormalArray(std::move(normals_));
		mesh->setTexcoordArray(std::move(texcoords_));
		mesh->setWeightArray(std::move(weights));

		PmxUInt32 startIndices = 0;

		for (std::size_t i = 0; i < pmx.materials.size(); i++)
		{
			math::uint1s indices_(pmx.materials[i].FaceCount);

			for (std::size_t j = startIndices; j < startIndices + pmx.materials[i].FaceCount; j++)
			{
				std::uint32_t index = 0;
				if (pmx.header.sizeOfIndices == 1)
					index = *((std::uint8_t*)pmx.indices.data() + j);
				else if (pmx.header.sizeOfIndices == 2)
					index = *((std::uint16_t*)pmx.indices.data() + j);
				else if (pmx.header.sizeOfIndices == 4)
					index = *((std::uint32_t*)pmx.indices.data() + j);

				indices_[j - startIndices] = index;
			}

			mesh->setIndicesArray(std::move(indices_), i);

			startIndices += pmx.materials[i].FaceCount;
		}

		mesh->computeBoundingBox();

		object->addComponent<MeshFilterComponent>(std::move(mesh));

		if (bones.empty())
		{
			auto meshRender = object->addComponent<MeshRendererComponent>(std::move(materials));
			meshRender->setGlobalIllumination(true);
		}
		else
		{
			auto smr = SkinnedMeshRendererComponent::create();
			smr->setMaterials(std::move(materials));
			smr->setTransforms(bones);
			smr->setMorphBlendEnable(true);
			smr->setTextureBlendEnable(true);
			smr->setGlobalIllumination(true);

			object->addComponent(smr);
		}
	}

	std::shared_ptr<Geometry>
	PMXLoader::loadGeometry(const PMX& pmx) noexcept(false)
	{
		Materials materials;
		createMaterials(pmx, materials);

		math::float4x4s bindposes(pmx.bones.size());
		for (std::size_t i = 0; i < pmx.bones.size(); i++)
			bindposes[i].makeTranslate(-math::float3(pmx.bones[i].position.x, pmx.bones[i].position.y, pmx.bones[i].position.z));

		math::float3s vertices_;
		math::float3s normals_;
		math::float2s texcoords_;
		std::vector<VertexWeight> weights;
		std::vector<std::shared_ptr<Mesh>> meshes;

		vertices_.resize(pmx.numVertices);
		normals_.resize(pmx.numVertices);
		texcoords_.resize(pmx.numVertices);

		if (pmx.numBones)
			weights.resize(pmx.numVertices);

		for (std::size_t i = 0; i < pmx.numVertices; i++)
		{
			auto& v = pmx.vertices[i];

			vertices_[i].set(v.position.x, v.position.y, v.position.z);
			normals_[i].set(v.normal.x, v.normal.y, v.normal.z);
			texcoords_[i].set(v.coord.x, v.coord.y);

			if (pmx.numBones)
			{
				VertexWeight weight;
				weight.weight1 = v.weight.weight1;
				weight.weight2 = v.weight.weight2;
				weight.weight3 = v.weight.weight3;
				weight.weight4 = v.weight.weight4;
				weight.bone1 = v.weight.bone1 < pmx.numBones ? v.weight.bone1 : 0;
				weight.bone2 = v.weight.bone2 < pmx.numBones ? v.weight.bone2 : 0;
				weight.bone3 = v.weight.bone3 < pmx.numBones ? v.weight.bone3 : 0;
				weight.bone4 = v.weight.bone4 < pmx.numBones ? v.weight.bone4 : 0;

				weights[i] = weight;
			}
		}

		auto mesh = std::make_shared<Mesh>();
		mesh->setBindposes(std::move(bindposes));
		mesh->setVertexArray(std::move(vertices_));
		mesh->setNormalArray(std::move(normals_));
		mesh->setTexcoordArray(std::move(texcoords_));
		mesh->setWeightArray(std::move(weights));

		PmxUInt32 startIndices = 0;

		for (std::size_t i = 0; i < pmx.materials.size(); i++)
		{
			math::uint1s indices_(pmx.materials[i].FaceCount);

			for (std::size_t j = startIndices; j < startIndices + pmx.materials[i].FaceCount; j++)
			{
				std::uint32_t index = 0;
				if (pmx.header.sizeOfIndices == 1)
					index = *((std::uint8_t*)pmx.indices.data() + j);
				else if (pmx.header.sizeOfIndices == 2)
					index = *((std::uint16_t*)pmx.indices.data() + j);
				else if (pmx.header.sizeOfIndices == 4)
					index = *((std::uint32_t*)pmx.indices.data() + j);

				indices_[j - startIndices] = index;
			}

			mesh->setIndicesArray(std::move(indices_), i);

			startIndices += pmx.materials[i].FaceCount;
		}

		mesh->computeBoundingBox();

		auto geometry = std::make_shared<Geometry>();
		geometry->setMesh(mesh);
		geometry->setMaterials(materials);

		return geometry;
	}

	std::shared_ptr<GameObject>
	PMXLoader::load(std::string_view filepath) noexcept(false)
	{
		PMX pmx;
		if (PMX::load(filepath, pmx))
		{
			if (pmx.numMaterials > 0)
			{
				GameObjects bones;
				GameObjectPtr actor = GameObject::create();

				if (!pmx.description.japanModelName.empty())
				{
					std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;
					actor->setName(cv.to_bytes(pmx.description.japanModelName.data()));
				}

				createBones(pmx, bones);
				createColliders(pmx, bones);
				createRigidbodies(pmx, bones);
				createJoints(pmx, bones);

				createMeshes(pmx, actor, bones);
				createMorph(pmx, actor);
				createClothes(pmx, actor, bones);

				return actor;
			}
		}

		return nullptr;
	}

	std::shared_ptr<GameObject>
	PMXLoader::load(const PMX& pmx) noexcept(false)
	{
		if (pmx.numMaterials > 0)
		{
			GameObjects bones;
			GameObjectPtr actor = GameObject::create();

			if (!pmx.description.japanModelName.empty())
			{
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;
				actor->setName(cv.to_bytes(pmx.description.japanModelName.data()));
			}

			createBones(pmx, bones);
			createColliders(pmx, bones);
			createRigidbodies(pmx, bones);
			createJoints(pmx, bones);

			createMeshes(pmx, actor, bones);
			createMorph(pmx, actor);
			createClothes(pmx, actor, bones);

			return actor;
		}

		return nullptr;
	}

	bool
	PMXLoader::save(const GameObject& gameObject, PMX& pmx) noexcept(false)
	{
		if (!gameObject.getName().empty())
		{
			pmx.header.magic[0] = 'P';
			pmx.header.magic[1] = 'M';
			pmx.header.magic[2] = 'X';
			pmx.header.offset = 0x20;
			pmx.header.version = 2.0f;
			pmx.header.dataSize = 0x08;
			pmx.header.encode = 0x0;
			pmx.header.addUVCount = 0;
			pmx.header.sizeOfIndices = 4;
			pmx.header.sizeOfTexture = 1;
			pmx.header.sizeOfMaterial = 1;
			pmx.header.sizeOfBone = 2;
			pmx.header.sizeOfMorph = 1;
			pmx.header.sizeOfBody = 1;

			pmx.numVertices = 0;
			pmx.numIndices = 0;
			pmx.numTextures = 0;
			pmx.numMaterials = 0;
			pmx.numBones = 0;
			pmx.numMorphs = 0;
			pmx.numDisplayFrames = 0;
			pmx.numRigidbodys = 0;
			pmx.numJoints = 0;
			pmx.numSoftbodies = 0;

			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;
			auto utf16 = cv.from_bytes(gameObject.getName());
			pmx.description.japanModelLength = utf16.length() * 2;
			pmx.description.japanModelName.resize(utf16.length());

			std::memcpy(pmx.description.japanModelName.data(), utf16.data(), pmx.description.japanModelLength);

			auto mf = gameObject.getComponent<MeshFilterComponent>();
			if (mf && mf->getMesh())
			{
				auto mesh = mf->getMesh();
				auto& weight = mesh->getWeightArray();

				pmx.numVertices = mesh->getNumVertices();
				pmx.numIndices = mesh->getNumIndices();

				pmx.vertices.resize(pmx.numVertices);
				pmx.indices.resize(pmx.numIndices * sizeof(std::uint32_t));

				for (std::size_t i = 0; i < pmx.numVertices; i++)
				{
					auto& pmxVertex = pmx.vertices[i];
					pmxVertex.position = mesh->getVertexArray()[i];
					pmxVertex.normal = mesh->getNormalArray()[i];
					pmxVertex.coord = mesh->getTexcoordArray()[i];
					pmxVertex.edge = 0;
					pmxVertex.type = PmxVertexSkinningType::PMX_BDEF1;
					pmxVertex.weight.bone1 = weight[i].bone1;
					pmxVertex.weight.bone2 = weight[i].bone2;
					pmxVertex.weight.bone3 = weight[i].bone3;
					pmxVertex.weight.bone4 = weight[i].bone4;
					pmxVertex.weight.weight1 = weight[i].weight1;
					pmxVertex.weight.weight2 = weight[i].weight2;
					pmxVertex.weight.weight3 = weight[i].weight3;
					pmxVertex.weight.weight4 = weight[i].weight4;

					if (pmxVertex.weight.weight1 != 0 && pmxVertex.weight.weight2 != 0 && pmxVertex.weight.weight3 != 0 && pmxVertex.weight.weight4 != 0)
						pmxVertex.type = PmxVertexSkinningType::PMX_BDEF4;
					else if (pmxVertex.weight.weight1 != 0 && pmxVertex.weight.weight2 != 0)
						pmxVertex.type = PmxVertexSkinningType::PMX_BDEF2;
					else if (pmxVertex.weight.weight1 != 0)
						pmxVertex.type = PmxVertexSkinningType::PMX_BDEF1;
				}

				for (std::size_t i = 0, offset = 0; i < mesh->getNumSubsets(); i++)
				{
					auto& indices = mesh->getIndicesArray(i);
					std::memcpy(pmx.indices.data() + offset, indices.data(), indices.size() * sizeof(std::uint32_t));
					offset += indices.size() * sizeof(std::uint32_t);
				}

				auto smr = gameObject.getComponent<SkinnedMeshRendererComponent>();
				if (smr)
				{
					std::map<std::string, std::size_t> textureMap;

					pmx.numMaterials = smr->getMaterials().size();
					pmx.materials.resize(pmx.numMaterials);

					for (std::size_t i = 0; i < pmx.numMaterials; i++)
					{
						auto standard = smr->getMaterial(i)->downcast_pointer<MeshStandardMaterial>();
						auto color = math::linear2srgb(standard->getColor());
						auto texture = standard->getColorMap();

						if (texture)
						{
							auto name = texture->getTextureDesc().getName();
							if (textureMap.find(name) == textureMap.end())
								textureMap.insert(std::make_pair(name, textureMap.size()));
						}

						auto& pmxMaterial = pmx.materials[i];
						pmxMaterial.name = PmxName(standard->getName());
						pmxMaterial.nameEng = PmxName("");
						pmxMaterial.Diffuse = color;
						pmxMaterial.Ambient = math::float3(0.5f, 0.5f, 0.5f);
						pmxMaterial.Opacity = standard->getOpacity();
						pmxMaterial.TextureIndex = texture ? textureMap[texture->getTextureDesc().getName()] : 255;
						pmxMaterial.ToonIndex = 255;
						pmxMaterial.SphereTextureIndex = 255;
						pmxMaterial.FaceCount = mesh->getIndicesArray(i).size();
					}

					pmx.numTextures = textureMap.size();
					pmx.textures.resize(textureMap.size());

					for (auto& it : textureMap)
						pmx.textures[it.second] = PmxName(it.first.substr(it.first.find_last_of("\\") + 1));

					auto transforms = smr->getTransforms();
					if (!transforms.empty())
					{
						pmx.numBones = smr->getTransforms().size();
						pmx.bones.resize(pmx.numBones);

						std::map<GameObject*, std::size_t> boneMap;

						for (std::size_t i = 0; i < pmx.numBones; i++)
							boneMap[smr->getTransforms()[i].get()] = i;

						for (std::size_t i = 0; i < pmx.numBones; i++)
						{
							auto transform = smr->getTransforms()[i];
							auto translate = transform->getComponent<octoon::TransformComponent>()->getTranslate();

							auto& pmxBone = pmx.bones[i];
							pmxBone.name = transform->getName();
							pmxBone.Visable = true;
							pmxBone.position = translate;
							pmxBone.Parent = transform->getParent() ? boneMap[transform->getParent()] : -1;
							pmxBone.ProvidedParentBoneIndex = -1;
							pmxBone.Flag |= PMX_BONE_ROTATION | PMX_BONE_MOVE | PMX_BONE_OPERATOR | PMX_BONE_DISPLAY;

							auto linkLimit = transform->getComponent<RotationLinkLimitComponent>();
							if (linkLimit)
							{
								if (linkLimit->getAdditiveUseLocal())
									pmxBone.Flag |= PMX_BONE_ADD_LOCAL;
								if (linkLimit->getAdditiveMoveRatio() != 0.0f)
									pmxBone.Flag |= PMX_BONE_ADD_MOVE;
								if (linkLimit->getAdditiveRotationRatio() != 0.0f)
									pmxBone.Flag |= PMX_BONE_ADD_ROTATION;
							}

							auto slover = transform->getComponent<CCDSolverComponent>();
							if (slover)
							{
								pmxBone.Flag |= PMX_BONE_IK;
								pmxBone.IKLoopCount = slover->getIterations();
								pmxBone.IKTargetBoneIndex = boneMap[slover->getTarget().get()];
								pmxBone.IKLinkCount = slover->getBones().size();
								pmxBone.IKList.resize(pmxBone.IKLinkCount);

								for (std::size_t n = 0; n < pmxBone.IKLinkCount; n++)
								{
									auto bone = slover->getBone(n);
									auto rotationLimit = bone->getComponent<RotationLimitComponent>();

									auto& ik = pmxBone.IKList[n];
									ik.BoneIndex = boneMap[bone.get()];
									
									if (rotationLimit)
									{
										ik.rotateLimited = rotationLimit->getMaximumAngle();
										ik.minimumRadian = rotationLimit->getMinimumAxis();
										ik.maximumRadian = rotationLimit->getMaximumAxis();
									}
								}
							}
						}

						for (std::size_t i = 0; i < pmx.numBones; i++)
						{
							auto bone = smr->getTransforms()[i];
							auto it = bone->getComponent<RigidbodyComponent>();
							if (it)
							{
								PmxRigidbody pmxRigidbody;
								pmxRigidbody.bone = i;
								pmxRigidbody.name = it->getName();
								pmxRigidbody.mass = it->getMass();
								pmxRigidbody.group = it->getGroup();
								pmxRigidbody.groupMask  = it->getGroupMask();
								pmxRigidbody.elasticity = it->getRestitution();
								pmxRigidbody.friction = it->getStaticFriction();
								pmxRigidbody.movementDecay = it->getLinearDamping();
								pmxRigidbody.rotationDecay = it->getAngularDamping();
								pmxRigidbody.physicsOperation = it->getIsKinematic() ? 0 : 1;

								auto collider = bone->getComponent<ColliderComponent>();
								if (collider)
								{
									auto baseTransform = bone->getComponent<TransformComponent>()->getTransform();
									auto localTransform = math::transformMultiply(baseTransform, math::makeRotation(collider->getQuaternion(), collider->getCenter()));

									math::float3 translate;
									math::float3 scale;
									math::Quaternion rotation;
									localTransform.getTransform(translate, rotation, scale);

									pmxRigidbody.position = translate;
									pmxRigidbody.rotate = math::eulerAngles(rotation);
								}

								if (auto boxCollider = bone->getComponent<BoxColliderComponent>())
								{
									pmxRigidbody.shape = PmxShapeType::ShapeTypeSquare;
									pmxRigidbody.scale = boxCollider->getSize();
								}								
								else if (auto capsuleCollider = bone->getComponent<CapsuleColliderComponent>())
								{
									pmxRigidbody.shape = PmxShapeType::ShapeTypeCapsule;
									pmxRigidbody.scale.x = capsuleCollider->getRadius();
									pmxRigidbody.scale.y = capsuleCollider->getHeight();
									pmxRigidbody.scale.z = 0.0f;
								}
								else if (auto sphereCollider = bone->getComponent<SphereColliderComponent>())
								{
									pmxRigidbody.shape = PmxShapeType::ShapeTypeSphere;
									pmxRigidbody.scale.x = sphereCollider->getRadius();
									pmxRigidbody.scale.y = 0.0f;
									pmxRigidbody.scale.z = 0.0f;
								}

								pmx.rigidbodies.push_back(pmxRigidbody);
							}

							pmx.numRigidbodys = pmx.rigidbodies.size();
						}
					}
				}
			}

			GameComponents morphComponents;
			gameObject.getComponents<SkinnedMorphComponent>(morphComponents);

			if (!morphComponents.empty())
			{
				pmx.numMorphs = morphComponents.size();
				pmx.morphs.resize(pmx.numMorphs);

				for (std::size_t i = 0; i < pmx.numMorphs; i++)
				{
					auto& offsets = morphComponents[i]->downcast<SkinnedMorphComponent>()->getOffsets();
					auto& indices = morphComponents[i]->downcast<SkinnedMorphComponent>()->getIndices();

					auto& morph = pmx.morphs[i];
					morph.name = morphComponents[i]->getName();
					morph.morphType = PmxMorphType::PMX_MorphTypeVertex;
					morph.control = morphComponents[i]->downcast<SkinnedMorphComponent>()->getControl();
					morph.morphCount = indices.size();
					morph.vertices.resize(indices.size());

					for (std::size_t n = 0; n < indices.size(); n++)
					{
						morph.vertices[n].index = indices[n];
						morph.vertices[n].offset = offsets[n];
					}
				}
			}
		}

		return true;
	}

	bool
	PMXLoader::save(const GameObject& gameObject, std::string_view path) noexcept(false)
	{
		auto stream = octoon::io::ofstream(std::string(path), std::ios_base::in | std::ios_base::out);
		if (stream)
		{
			auto pmx = std::make_unique<PMX>();
			save(gameObject, *pmx);

			PMX::save(stream, *pmx);

			return true;
		}

		return false;
	}
}