#include <octoon/fbx_loader.h>
#include <octoon/asset_loader.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/transform_component.h>
#include <octoon/point_light_component.h>
#include <octoon/film_camera_component.h>
#include <octoon/mesh_filter_component.h>
#include <octoon/mesh_renderer_component.h>
#include <fstream>
#include <tuple>
#include <set>
#include <fbxsdk.h>

namespace octoon
{
	FBXLoader::FBXLoader() noexcept
	{
	}

	FBXLoader::~FBXLoader() noexcept
	{
	}

	bool
	FBXLoader::doCanRead(std::istream& stream) noexcept
	{
		return false;
	}

	bool
	FBXLoader::doCanRead(const char* type) noexcept
	{
		return std::strncmp(type, "fbx", 3) == 0;
	}

	void ParseChannel(FbxNode* node, FbxAnimLayer* layer)
	{
		auto curve = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
		if (curve == 0)
			return;

		FbxTime time;
		char timeString[256];

		auto keyCount = curve->KeyGetCount();

		for (int i = 0; i < keyCount; i++)
		{
			time = curve->KeyGetTime(i);

			auto matLocal = node->EvaluateLocalTransform(time);
			auto matGlobal = node->EvaluateGlobalTransform(time);

			time.GetTimeString(timeString, FbxUShort(256));

			std::cout << timeString << std::endl;
		}
	}

	void ParseLayer(FbxNode* node, FbxAnimLayer* layer)
	{
		ParseChannel(node, layer);

		auto childCount = node->GetChildCount();

		for (int i = 0; i < childCount; i++)
			ParseLayer(node->GetChild(i), layer);
	}

	void ParseAnimation(FbxNode* node)
	{
		auto stackCount = node->GetSrcObjectCount<FbxAnimStack>();

		for (int i = 0; i < stackCount; i++)
		{
			auto stack = node->GetSrcObject<FbxAnimStack>(i);
			auto stackName = stack->GetName();

			auto layerCount = stack->GetMemberCount<FbxAnimLayer>();
			for (int j = 0; j < layerCount; j++)
			{
				auto layer = stack->GetMember<FbxAnimLayer>(j);
				auto layerName = layer->GetName();
				ParseLayer(node, layer);
			}
		}
	}

	FbxString GetTexturePath(FbxSurfaceMaterial* surfaceMaterial, const char* name)
	{
		auto fbxProperty = surfaceMaterial->FindProperty(name);
		if (fbxProperty.IsValid())
		{
			auto textureCount = fbxProperty.GetSrcObjectCount<FbxTexture>();
			if (textureCount > 0)
			{
				auto fileTexture = fbxProperty.GetSrcObject<FbxFileTexture>(0);
				if (fileTexture)
				{
					FbxString filename = FbxPathUtils::GetFileName(fileTexture->GetFileName());
					return filename;
				}
			}
		}

		return FbxString();
	}

	std::shared_ptr<Texture> LoadTexture(FbxSurfaceMaterial* surfaceMaterial, const char* name, const std::filesystem::path& fbxPath)
	{
		FbxString filename = GetTexturePath(surfaceMaterial, name);
		if (!filename.IsEmpty())
		{
			std::filesystem::path path = std::filesystem::path(fbxPath.parent_path()).append(filename.Buffer());

			if (std::filesystem::exists(path))
			{
				auto texture = AssetLoader::instance()->loadAssetAtPath<Texture>(path);
				if (texture)
				{
					texture->apply();
					AssetLoader::instance()->addObjectToAsset(texture, path);
					return texture;
				}
			}
		}

		return nullptr;
	}

	std::shared_ptr<Material> LoadMaterialAttribute(FbxSurfaceMaterial* surfaceMaterial, const std::filesystem::path& path)
	{
		auto material = std::make_shared<MeshStandardMaterial>();
		material->setName(surfaceMaterial->GetName());
		material->setColorMap(LoadTexture(surfaceMaterial, FbxSurfaceMaterial::sDiffuse, path));
		material->setNormalMap(LoadTexture(surfaceMaterial, FbxSurfaceMaterial::sNormalMap, path));
		material->setEmissiveMap(LoadTexture(surfaceMaterial, FbxSurfaceMaterial::sEmissive, path));

		if (!material->getNormalMap())
			material->setNormalMap(LoadTexture(surfaceMaterial, FbxSurfaceMaterial::sBump, path));

		if (surfaceMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
		{
			auto phongMaterial = FbxCast<FbxSurfacePhong>(surfaceMaterial);
			auto diffuse = phongMaterial->Diffuse.Get();
			auto diffuseFactor = phongMaterial->DiffuseFactor.Get();
			auto specular = phongMaterial->Specular.Get();
			auto specularFactory = phongMaterial->SpecularFactor.Get();
			auto emissive = phongMaterial->Emissive.Get();
			auto emissiveFactor = phongMaterial->EmissiveFactor.Get();
			auto transparencyColor = phongMaterial->TransparentColor.Get();
			auto transparencyfactor = phongMaterial->TransparencyFactor.Get();
			auto shininess = phongMaterial->Shininess.Get();
			auto reflection = phongMaterial->Reflection.Get();
			auto reflectionfactor = phongMaterial->ReflectionFactor.Get();

			material->setRoughness(std::sqrt(2.0f / ((float)shininess + 2.0f)));
			material->setColor(math::float3((float)diffuse[0], (float)diffuse[1], (float)diffuse[2]) * (float)diffuseFactor);
			material->setEmissive(math::float3((float)emissive[0], (float)emissive[1], (float)emissive[2]));
			material->setEmissiveIntensity((float)emissiveFactor);
		}
		else if (surfaceMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
		{
			auto lambertMaterial = FbxCast<FbxSurfaceLambert>(surfaceMaterial);
			auto diffuse = lambertMaterial->Diffuse.Get();
			auto diffuseFactor = lambertMaterial->DiffuseFactor.Get();
			auto emissive = lambertMaterial->Emissive.Get();
			auto emissiveFactor = lambertMaterial->EmissiveFactor.Get();
			auto transparencyColor = lambertMaterial->TransparentColor.Get();
			auto transparencyfactor = lambertMaterial->TransparencyFactor.Get();

			material->setColor(math::float3((float)diffuse[0], (float)diffuse[1], (float)diffuse[2]) * (float)diffuseFactor);
			material->setEmissive(math::float3((float)emissive[0], (float)emissive[1], (float)emissive[2]));
			material->setEmissiveIntensity((float)emissiveFactor);
		}

		return material;
	}

	std::size_t LoadMaterial(FbxMesh* mesh, std::vector<std::shared_ptr<Material>>& materials, const std::filesystem::path& path)
	{
		if (mesh && mesh->GetNode())
		{
			auto node = mesh->GetNode();
			auto materialCount = mesh->GetElementMaterialCount();

			for (int materialIndex = 0; materialIndex < materialCount; materialIndex++)
				materials.push_back(LoadMaterialAttribute(node->GetMaterial(materialIndex), path));

			return materialCount;
		}

		return 0;
	}

	math::float3 ReadVertex(FbxMesh* mesh, int index)
	{
		auto fbxPoint = mesh->GetControlPoints()[index];

		math::float3 vertex;
		vertex.x = (float)(fbxPoint[0]);
		vertex.y = (float)(fbxPoint[1]);
		vertex.z = (float)(fbxPoint[2]);

		return vertex;
	}

	int ReadColor(FbxMesh* mesh, int pointIndex, int vertexCounter, math::float4& color)
	{
		if (mesh->GetElementVertexColorCount() < 1)
			return -1;

		auto vertexColor = mesh->GetElementVertexColor(0);
		auto mappingMode = vertexColor->GetMappingMode();
		auto index = mappingMode == FbxLayerElement::eByPolygonVertex ? vertexCounter : pointIndex;

		if (mappingMode == FbxLayerElement::eByControlPoint || mappingMode == FbxLayerElement::eByPolygonVertex)
		{
			switch (vertexColor->GetReferenceMode())
			{
			case FbxLayerElement::eDirect:
			{
				auto fbxColor = vertexColor->GetDirectArray().GetAt(index);

				color.r = (float)(fbxColor.mRed);
				color.g = (float)(fbxColor.mGreen);
				color.b = (float)(fbxColor.mBlue);
				color.a = (float)(fbxColor.mAlpha);

				return index;
			}
			break;
			case FbxLayerElement::eIndexToDirect:
			{
				auto fbxColor = vertexColor->GetDirectArray().GetAt(index);

				color.r = (float)(fbxColor.mRed);
				color.g = (float)(fbxColor.mGreen);
				color.b = (float)(fbxColor.mBlue);
				color.a = (float)(fbxColor.mAlpha);

				return index;
			}
			break;
			}
		}

		return -1;
	}

	int ReadUv(FbxMesh* mesh, int pointIndex, int textureIndex, int layer, math::float2& uv)
	{
		if (mesh->GetElementUVCount() <= layer)
			return -1;

		auto element = mesh->GetElementUV(layer);
		auto mappingMode = element->GetMappingMode();
		auto index = mappingMode == FbxLayerElement::eByPolygonVertex ? textureIndex : pointIndex;

		if (mappingMode == FbxLayerElement::eByControlPoint || mappingMode == FbxLayerElement::eByPolygonVertex)
		{
			switch (element->GetReferenceMode())
			{
			case FbxLayerElement::eDirect:
			{
				auto fbxTexcoord = element->GetDirectArray().GetAt(index);

				uv.x = (float)(fbxTexcoord[0]);
				uv.y = (float)(fbxTexcoord[1]);

				return index;
			}
			break;
			case FbxLayerElement::eIndexToDirect:
			{
				auto fbxTexcoord = element->GetDirectArray().GetAt(index);

				uv.x = (float)(fbxTexcoord[0]);
				uv.y = (float)(fbxTexcoord[1]);

				return index;
			}
			break;
			}
		}

		return -1;
	}

	int ReadNormal(FbxMesh* mesh, int pointIndex, int normalIndex, math::float3& normal)
	{
		if (mesh->GetElementNormalCount() < 1)
			return -1;

		auto element = mesh->GetElementNormal(0);
		auto mappingMode = element->GetMappingMode();
		auto index = mappingMode == FbxLayerElement::eByPolygonVertex ? normalIndex : pointIndex;

		if (mappingMode == FbxLayerElement::eByControlPoint || mappingMode == FbxLayerElement::eByPolygonVertex)
		{
			switch (element->GetReferenceMode())
			{
			case FbxLayerElement::eDirect:
			{
				auto fbxNormal = element->GetDirectArray().GetAt(index);

				normal.x = (float)(fbxNormal[0]);
				normal.y = (float)(fbxNormal[1]);
				normal.z = (float)(fbxNormal[2]);

				return index;
			}
			break;
			case FbxLayerElement::eIndexToDirect:
			{
				auto id = element->GetIndexArray().GetAt(index);
				auto fbxNormal = element->GetDirectArray().GetAt(id);

				math::float3 normal;
				normal.x = (float)(fbxNormal[0]);
				normal.y = (float)(fbxNormal[1]);
				normal.z = (float)(fbxNormal[2]);

				return index;
			}
			break;
			}
		}

		return -1;
	}

	int ReadTangent(FbxMesh* mesh, int pointIndex, int tangentIndex, math::float3& tangent)
	{
		if (mesh->GetElementNormalCount() <= 1)
			return -1;

		auto element = mesh->GetElementTangent(0);
		auto mappingMode = element->GetMappingMode();
		auto index = mappingMode == FbxLayerElement::eByPolygonVertex ? tangentIndex : pointIndex;

		if (mappingMode == FbxLayerElement::eByControlPoint || mappingMode == FbxLayerElement::eByPolygonVertex)
		{
			switch (element->GetReferenceMode())
			{
			case FbxLayerElement::eDirect:
			{
				auto fbxTangent = element->GetDirectArray().GetAt(index);

				tangent.x = (float)(fbxTangent[0]);
				tangent.y = (float)(fbxTangent[1]);
				tangent.z = (float)(fbxTangent[2]);

				return index;
			}
			break;
			case FbxLayerElement::eIndexToDirect:
			{
				auto id = element->GetIndexArray().GetAt(index);
				auto fbxTangent = element->GetDirectArray().GetAt(id);

				math::float3 tangent;
				tangent.x = (float)(fbxTangent[0]);
				tangent.y = (float)(fbxTangent[1]);
				tangent.z = (float)(fbxTangent[2]);

				return index;
			}
			break;
			}
		}

		return -1;
	}

	std::shared_ptr<GameObject> ParseMesh(FbxNode* node, const std::filesystem::path& path)
	{
		auto fbxMesh = node->GetMesh();
		if (fbxMesh)
		{
			int polygonCount = fbxMesh->GetPolygonCount();
			int polygonVertexCount = fbxMesh->GetPolygonVertexCount();
			int pointsCount = fbxMesh->GetControlPointsCount();

			math::float3s polygonVertices(pointsCount);
			math::float3s polygonNormals(polygonVertexCount);
			math::float2s polygonTexcoords(polygonVertexCount);
			std::vector<std::tuple<int, int, int>> polygonIndices(polygonVertexCount);

			for (int i = 0; i < pointsCount; i++)
				polygonVertices[i] = ReadVertex(fbxMesh, i);

			std::set<std::tuple<int, int, int>> polygonSet;
			std::map<std::tuple<int, int, int>, std::uint32_t> polygonMap;

			for (int i = 0, vertexCounter = 0; i < polygonCount; i++)
			{
				for (int j = 0; j < 3; j++, vertexCounter++)
				{
					math::float3 n;
					math::float2 uv;

					auto pointIndex = fbxMesh->GetPolygonVertex(i, j);
					auto normalIndex = ReadNormal(fbxMesh, pointIndex, vertexCounter, n);
					auto texcoordIndex = ReadUv(fbxMesh, pointIndex, fbxMesh->GetTextureUVIndex(i, j), 0, uv);
					auto tuple = std::make_tuple(pointIndex, normalIndex, texcoordIndex);

					if (normalIndex >= 0)
						polygonNormals[normalIndex] = n;

					if (texcoordIndex >= 0)
						polygonTexcoords[texcoordIndex] = uv;

					polygonIndices[vertexCounter] = tuple;
					polygonSet.insert(tuple);
				}
			}

			std::uint32_t idx = 0;

			auto vertices = math::float3s(polygonSet.size());
			auto normals = math::float3s(polygonSet.size());
			auto texcoords = math::float2s(polygonSet.size());
			auto indices = math::uint1s(polygonVertexCount);

			for (auto& index : polygonSet)
			{
				auto v = std::get<0>(index);
				auto n = std::get<1>(index);
				auto uv = std::get<2>(index);

				vertices[idx] = polygonVertices[v];
				normals[idx] = polygonNormals[n];
				texcoords[idx] = polygonTexcoords[uv];

				polygonMap[index] = idx++;
			}

			std::size_t polygonIndexCount = polygonIndices.size();
			for (std::size_t i = 0; i < polygonIndexCount; i++)
				indices[i] = polygonMap.at(polygonIndices[i]);

			auto mesh = std::make_shared<Mesh>();
			mesh->setVertexArray(std::move(vertices));
			mesh->setNormalArray(std::move(normals));
			mesh->setTexcoordArray(std::move(texcoords));
			mesh->setIndicesArray(std::move(indices));

			AssetLoader::instance()->addObjectToAsset(mesh, path);

			auto gameObject = std::make_shared<GameObject>();
			gameObject->setName(node->GetName());
			gameObject->addComponent<MeshFilterComponent>(std::move(mesh));

			auto meshRenderer = gameObject->addComponent<MeshRendererComponent>();
			meshRenderer->setGlobalIllumination(true);

			if (fbxMesh->GetElementMaterialCount() > 0)
			{
				std::vector<std::shared_ptr<Material>> materials;
				LoadMaterial(fbxMesh, materials, path);

				for (std::size_t i = 0; i < materials.size(); i++)
				{
					auto material = materials[i] ? materials[i] : std::make_shared<MeshStandardMaterial>();
					AssetLoader::instance()->addObjectToAsset(material, path);
					meshRenderer->setMaterial(std::move(material), i);
				}
			}
			else
			{
				auto material = std::make_shared<MeshStandardMaterial>();
				AssetLoader::instance()->addObjectToAsset(material, path);
				meshRenderer->setMaterial(std::move(material));
			}

			auto translation = node->LclTranslation.Get();
			auto rotation = node->LclRotation.Get();
			auto scaling = node->LclScaling.Get();

			auto transform = gameObject->getComponent<TransformComponent>();
			transform->setLocalTranslate(math::float3((float)translation[0], (float)translation[1], (float)translation[2]));
			transform->setLocalEulerAngles(math::float3((float)rotation[0], (float)rotation[1], (float)rotation[2]));
			transform->setLocalScale(math::float3((float)scaling[0], (float)scaling[1], (float)scaling[2]));

			return gameObject;
		}

		return nullptr;
	}

	std::shared_ptr<GameObject> ParseCamera(FbxNode* node)
	{
		auto camera = node->GetCamera();
		if (camera)
			ParseAnimation(node);
		return std::make_shared<GameObject>();
	}

	std::shared_ptr<GameObject> ParseSkeleton(FbxNode* node)
	{
		auto object = std::make_shared<GameObject>();
		return object;
	}

	GameObjectPtr ProcessNode(FbxNode* node, const std::filesystem::path& path)
	{
		GameObjectPtr object;

		auto attribute = node->GetNodeAttribute();
		if (attribute)
		{
			switch (attribute->GetAttributeType())
			{
			case FbxNodeAttribute::eCamera:
				object = ParseCamera(node);
				break;
			case FbxNodeAttribute::eMesh:
				object = ParseMesh(node, path);
				break;
			case FbxNodeAttribute::eSkeleton:
				object = ParseSkeleton(node);
				break;
			default:
				object = std::make_shared<GameObject>();
				break;
			}
		}

		if (object)
		{
			for (int j = 0; j < node->GetChildCount(); j++)
			{
				auto child = ProcessNode(node->GetChild(j), path);
				AssetLoader::instance()->addObjectToAsset(child, path);
				object->addChild(child);
			}
		}

		return object;
	}

	std::shared_ptr<GameObject>
	FBXLoader::load(const std::filesystem::path& filepath) noexcept(false)
	{
		auto lsdkManager = FbxManager::Create();
		if (lsdkManager)
		{
			FbxIOSettings* ios = FbxIOSettings::Create(lsdkManager, IOSROOT);
			lsdkManager->SetIOSettings(ios);

			FbxString extension = "dll";
			FbxString path = FbxGetApplicationDirectory();
			lsdkManager->LoadPluginsDirectory(path.Buffer(), extension.Buffer());

			FbxImporter* importer = FbxImporter::Create(lsdkManager, "");

			if (importer->Initialize((char*)filepath.u8string().c_str(), -1, lsdkManager->GetIOSettings()))
			{
				int major = 0, minor = 0, revision = 0;
				importer->GetFileVersion(major, minor, revision);

				FbxScene* scene = FbxScene::Create(lsdkManager, "myScene");
				importer->Import(scene);
				importer->Destroy();

				FbxArray<FbxString*> animStackNameArray;
				scene->FillAnimStackNameArray(animStackNameArray);

				FbxNode* rootNode = scene->GetRootNode();
				if (rootNode)
				{
					auto object = std::make_shared<GameObject>();

					for (int i = 0; i < rootNode->GetChildCount(); i++)
					{
						auto node = ProcessNode(rootNode->GetChild(i), filepath);
						AssetLoader::instance()->setAssetPath(node, filepath);
						object->addChild(std::move(node));
					}
					
					if (object->getChildCount() > 1)
					{
						AssetLoader::instance()->setAssetPath(object, filepath);

						for (int i = 0; i < object->getChildCount(); i++)
							AssetLoader::instance()->addObjectToAsset(object->getChild(i), filepath);

						return object;
					}
					else
					{
						AssetLoader::instance()->setAssetPath(object->getChild(0), filepath);
						return object->getChild(0);
					}
				}
			}
			else
			{
				printf("Call to FbxImporter::Initialize() failed.\n");
				printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
			}

			lsdkManager->Destroy();
		}

		return nullptr;
	}

	void importerNode(FbxNode* node, std::vector<std::filesystem::path>& dependencies)
	{
		GameObjectPtr object;

		auto attribute = node->GetNodeAttribute();
		if (attribute)
		{
			switch (attribute->GetAttributeType())
			{
			case FbxNodeAttribute::eMesh:
			{
				auto fbxMesh = node->GetMesh();
				if (fbxMesh)
				{
					if (fbxMesh->GetElementMaterialCount() > 0)
					{
						auto node = fbxMesh->GetNode();
						auto materialCount = fbxMesh->GetElementMaterialCount();

						for (int materialIndex = 0; materialIndex < materialCount; materialIndex++)
						{
							auto fbxMaterial = node->GetMaterial(materialIndex);
							auto diffuseMap = GetTexturePath(fbxMaterial, FbxSurfaceMaterial::sDiffuse);
							if (!diffuseMap.IsEmpty())
								dependencies.push_back(diffuseMap.Buffer());
							auto normalMapMap = GetTexturePath(fbxMaterial, FbxSurfaceMaterial::sNormalMap);
							if (!normalMapMap.IsEmpty())
								dependencies.push_back(normalMapMap.Buffer());
							auto emissiveMap = GetTexturePath(fbxMaterial, FbxSurfaceMaterial::sEmissive);
							if (!emissiveMap.IsEmpty())
								dependencies.push_back(emissiveMap.Buffer());
							auto bumpMap = GetTexturePath(fbxMaterial, FbxSurfaceMaterial::sBump);
							if (!bumpMap.IsEmpty())
								dependencies.push_back(bumpMap.Buffer());
						}
					}
				}
			}
			break;
			}
		}

		for (int j = 0; j < node->GetChildCount(); j++)
			importerNode(node->GetChild(j), dependencies);
	}

	std::vector<std::filesystem::path>
	FBXLoader::getDependencies(const std::filesystem::path& filepath) noexcept(false)
	{
		std::vector<std::filesystem::path> dependencies;

		auto lsdkManager = FbxManager::Create();
		if (lsdkManager)
		{
			FbxIOSettings* ios = FbxIOSettings::Create(lsdkManager, IOSROOT);
			lsdkManager->SetIOSettings(ios);

			FbxString extension = "dll";
			FbxString path = FbxGetApplicationDirectory();
			lsdkManager->LoadPluginsDirectory(path.Buffer(), extension.Buffer());

			FbxImporter* importer = FbxImporter::Create(lsdkManager, "");

			if (importer->Initialize((char*)filepath.u8string().c_str(), -1, lsdkManager->GetIOSettings()))
			{
				int major = 0, minor = 0, revision = 0;
				importer->GetFileVersion(major, minor, revision);

				FbxScene* scene = FbxScene::Create(lsdkManager, "myScene");
				importer->Import(scene);
				importer->Destroy();

				FbxNode* rootNode = scene->GetRootNode();
				if (rootNode)
				{
					auto object = std::make_shared<GameObject>();

					for (int i = 0; i < rootNode->GetChildCount(); i++)
						importerNode(rootNode->GetChild(i), dependencies);
				}
			}
			else
			{
				printf("Call to FbxImporter::Initialize() failed.\n");
				printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
			}

			lsdkManager->Destroy();
		}

		return dependencies;
	}
}