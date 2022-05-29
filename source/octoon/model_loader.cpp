#include <octoon/model_loader.h>
#include <octoon/model/modtypes.h>
#include <octoon/mesh/mesh.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/model/model.h>
#include <octoon/texture_loader.h>
#include <octoon/io/fstream.h>
#include <octoon/math/mathfwd.h>
#include <octoon/math/mathutil.h>
#include <octoon/runtime/string.h>
#include <octoon/hal/graphics_texture.h>

#include <map>
#include <cstring>
#include <codecvt>

namespace octoon
{
	Model
	ModelLoader::load(const PMX& pmx) noexcept
	{
		Model model;

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

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

			model.materials.emplace_back(std::move(material));
		}

		math::float4x4s bindposes(pmx.bones.size());
		for (std::size_t i = 0; i < pmx.bones.size(); i++)
			bindposes[i].makeTranslate(-math::float3(pmx.bones[i].position.x, pmx.bones[i].position.y, pmx.bones[i].position.z));

		math::float3s vertices_;
		math::float3s normals_;
		math::float2s texcoords_;
		std::vector<VertexWeight> weights;

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
		model.meshes.emplace_back(std::move(mesh));

		for (std::size_t i = 0; i < pmx.bones.size(); i++)
		{
			auto& it = pmx.bones[i];

			Bone bone;
			bone.setName(cv.to_bytes(it.name.name));
			bone.setPosition(math::float3(it.position.x, it.position.y, it.position.z));
			bone.setParent(it.Parent);
			bone.setVisable(it.Visable);
			bone.setAdditiveParent(it.ProvidedParentBoneIndex);
			bone.setAdditiveUseLocal(!(it.Flag & PMX_BONE_ADD_LOCAL));

			if (it.Flag & PMX_BONE_ADD_MOVE)
				bone.setAdditiveMoveRatio(it.ProvidedRatio);
			if (it.Flag & PMX_BONE_ADD_ROTATION)
				bone.setAdditiveRotationRatio(it.ProvidedRatio);

			model.bones.emplace_back(std::make_shared<Bone>(bone));

			if (it.Flag & PMX_BONE_IK)
			{
				IKAttr attr;
				attr.boneIndex = static_cast<uint16_t>(i);
				attr.targetBoneIndex = it.IKTargetBoneIndex;
				attr.chainLength = it.IKLinkCount;
				attr.iterations = it.IKLoopCount;

				for (auto& ik : it.IKList)
				{
					IKChild child;
					child.boneIndex = ik.BoneIndex;
					child.angleRadian = it.IKLimitedRadian;
					child.minimumRadian.set(ik.minimumRadian.x, ik.minimumRadian.y, ik.minimumRadian.z);
					child.maximumRadian.set(ik.maximumRadian.x, ik.maximumRadian.y, ik.maximumRadian.z);
					child.rotateLimited = ik.rotateLimited;

					attr.child.push_back(child);
				}

				model.iks.emplace_back(std::make_shared<IKAttr>(attr));
			}
		}

		for (auto& it : pmx.morphs)
		{
			switch (it.morphType)
			{
			case PmxMorphType::PMX_MorphTypeVertex:
			{
				auto morph = std::make_shared<Morph>();
				morph->name = cv.to_bytes(it.name.name);
				morph->morphType = it.morphType;
				morph->control = it.control;
				morph->morphCount = it.morphCount;
				morph->vertices.reserve(it.vertices.size());

				for (auto& v : it.vertices)
				{
					MorphVertex vertex;
					vertex.index = v.index;
					vertex.offset.set(v.offset.x, v.offset.y, v.offset.z);
					morph->vertices.push_back(vertex);
				}

				model.morphs.emplace_back(morph);
			}
			break;
			}
		}

		for (auto& it : pmx.rigidbodies)
		{
			auto body = std::make_shared<Rigidbody>();
			body->name = cv.to_bytes(it.name.name);
			body->bone = it.bone;
			body->group = it.group;
			body->groupMask = it.groupMask;
			body->shape = (ShapeType)it.shape;
			body->scale.set(it.scale.x, it.scale.y, it.scale.z);
			body->position.set(it.position.x, it.position.y, it.position.z);
			body->rotation.set(it.rotate.x, it.rotate.y, it.rotate.z);
			body->mass = it.mass;
			body->movementDecay = it.movementDecay;
			body->rotationDecay = it.rotationDecay;
			body->elasticity = it.elasticity;
			body->friction = it.friction;
			body->physicsOperation = it.physicsOperation;

			if (body->shape == ShapeType::ShapeTypeCapsule && body->scale.y == 0.0f)
				body->shape = ShapeType::ShapeTypeSphere;

			model.rigidbodies.emplace_back(std::move(body));
		}

		for (auto& it : pmx.joints)
		{
			auto joint = std::make_shared<Joint>();
			joint->name = cv.to_bytes(it.name.name);
			joint->type = it.type;
			joint->bodyIndexA = it.relatedRigidBodyIndexA;
			joint->bodyIndexB = it.relatedRigidBodyIndexB;
			joint->position.set(it.position.x, it.position.y, it.position.z);
			joint->rotation.set(it.rotation.x, it.rotation.y, it.rotation.z);
			joint->movementLowerLimit.set(it.movementLowerLimit.x, it.movementLowerLimit.y, it.movementLowerLimit.z);
			joint->movementUpperLimit.set(it.movementUpperLimit.x, it.movementUpperLimit.y, it.movementUpperLimit.z);
			joint->rotationLowerLimit.set(it.rotationLowerLimit.x, it.rotationLowerLimit.y, it.rotationLowerLimit.z);
			joint->rotationUpperLimit.set(it.rotationUpperLimit.x, it.rotationUpperLimit.y, it.rotationUpperLimit.z);
			joint->springMovementConstant.set(it.springMovementConstant.x, it.springMovementConstant.y, it.springMovementConstant.z);
			joint->springRotationConstant.set(it.springRotationConstant.x, it.springRotationConstant.y, it.springRotationConstant.z);

			model.joints.emplace_back(std::move(joint));
		}

		for (auto& it : pmx.softbodies)
		{
			auto softbody = std::make_shared<Softbody>();
			softbody->name = cv.to_bytes(it.name.name);
			softbody->materialIndex = it.materialIndex;
			softbody->group = it.group;
			softbody->groupMask = it.groupMask;
			softbody->blinkLength = it.blinkLength;
			softbody->numClusters = it.numClusters;
			softbody->totalMass = it.totalMass;
			softbody->collisionMargin = it.collisionMargin;
			softbody->aeroModel = it.aeroModel;
			softbody->LST = it.LST;
			softbody->anchorRigidbodies.reserve(it.numRigidbody);
				
			for (auto& rigidbody : it.anchorRigidbodies)
				softbody->anchorRigidbodies.push_back(rigidbody.rigidBodyIndex);

			for (std::size_t i = 0; i < it.numIndices; i++)
			{
				std::uint32_t index = 0;
				if (pmx.header.sizeOfIndices == 1)
					index = *((std::uint8_t*)it.pinVertexIndices.data() + i);
				else if (pmx.header.sizeOfIndices == 2)
					index = *((std::uint16_t*)it.pinVertexIndices.data() + i);
				else if (pmx.header.sizeOfIndices == 4)
					index = *((std::uint32_t*)it.pinVertexIndices.data() + i);

				softbody->pinVertexIndices.push_back(index);
			}

			model.softbodies.emplace_back(std::move(softbody));
		}

		return model;
	}
}