#include <octoon/obj_loader.h>
#include <octoon/asset_database.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/mesh_filter_component.h>
#include <octoon/mesh_renderer_component.h>
#include <tiny_obj_loader.h>
#include <fstream>
#include <set>
#include <unordered_map>

namespace octoon
{
	OBJLoader::OBJLoader() noexcept
	{
	}

	OBJLoader::~OBJLoader() noexcept
	{
	}

	bool
	OBJLoader::doCanRead(io::istream& stream) noexcept
	{
		return false;
	}

	bool
	OBJLoader::doCanRead(const char* type) noexcept
	{
		return std::strncmp(type, "obj", 3) == 0;
	}

	std::shared_ptr<GameObject>
	OBJLoader::load(std::istream& stream) noexcept(false)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;

		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &stream);
		if (ret)
		{
			std::vector<std::shared_ptr<MeshStandardMaterial>> standardMaterials(materials.size());

			for (std::size_t i = 0; i < materials.size(); i ++)
			{
				auto& material = materials[i];

				auto standard = std::make_shared<MeshStandardMaterial>();
				standard->setName(material.name);
				standard->setColor(math::float3(material.diffuse[0], material.diffuse[1], material.diffuse[2]));
				standard->setRoughness(material.roughness);
				standard->setMetalness(material.metallic);
				standard->setSheen(material.sheen);
				standard->setClearCoat(material.clearcoat_thickness);
				standard->setClearCoatRoughness(material.clearcoat_roughness);
				standard->setAnisotropy(material.anisotropy);
				standard->setRefractionRatio(material.ior);

				if (!material.diffuse_texname.empty())
				{
					auto texture = AssetDatabase::instance()->loadAssetAtPath<Texture>(material.diffuse_texname);
					if (texture) {
						texture->apply();
						standard->setColorMap(std::move(texture));
					}
				}

				if (!material.normal_texname.empty())
				{
					auto texture = AssetDatabase::instance()->loadAssetAtPath<Texture>(material.normal_texname);
					if (texture) {
						texture->apply();
						standard->setNormalMap(std::move(texture));
					}
				}

				if (!material.roughness_texname.empty())
				{
					auto texture = AssetDatabase::instance()->loadAssetAtPath<Texture>(material.roughness_texname);
					if (texture) {
						texture->apply();
						standard->setRoughnessMap(std::move(texture));
					}
				}

				if (!material.metallic_texname.empty())
				{
					auto texture = AssetDatabase::instance()->loadAssetAtPath<Texture>(material.metallic_texname);
					if (texture) {
						texture->apply();
						standard->setMetalnessMap(std::move(texture));
					}
				}

				if (!material.sheen_texname.empty())
				{
					auto texture = AssetDatabase::instance()->loadAssetAtPath<Texture>(material.sheen_texname);
					if (texture) {
						texture->apply();
						standard->setSheenMap(std::move(texture));
					}
				}

				if (!material.emissive_texname.empty())
				{
					auto texture = AssetDatabase::instance()->loadAssetAtPath<Texture>(material.emissive_texname);
					if (texture) {
						texture->apply();
						standard->setEmissiveMap(std::move(texture));
					}
				}

				standardMaterials[i] = std::move(standard);
			}

			std::set<std::tuple<int, int, int>> vertexSet;
			std::map<std::tuple<int, int, int>, std::uint32_t> vertexMap;

			for (auto& shape : shapes)
			{
				std::size_t numFace = shape.mesh.num_face_vertices.size();

				for (std::size_t f = 0, indexOffset = 0; f < numFace; f++)
				{
					std::size_t numFaceVertices = shape.mesh.num_face_vertices[f];

					for (std::size_t i = 0; i < numFaceVertices; i++)
					{
						auto& index = shape.mesh.indices[indexOffset + i];
						vertexSet.insert(std::make_tuple(index.vertex_index, index.normal_index, index.texcoord_index));
					}

					indexOffset += numFaceVertices;
				}

				std::uint32_t idx = 0;

				for (auto& index : vertexSet)
					vertexMap[index] = idx++;
			}

			auto vertices = math::float3s(vertexSet.size());
			auto normals = math::float3s(vertexSet.size());
			auto texcoords = math::float2s(vertexSet.size());

			if (!attrib.vertices.empty())
			{
				std::uint32_t idx = 0;

				for (auto& index : vertexSet)
				{
					auto v = std::get<0>(index) * 3;
					vertices[idx++].set(attrib.vertices[v], attrib.vertices[v + 1], attrib.vertices[v + 2]);
				}
			}

			if (!attrib.normals.empty())
			{
				std::uint32_t idx = 0;

				for (auto& index : vertexSet)
				{
					auto v = std::get<1>(index) * 3;
					normals[idx++].set(attrib.normals[v], attrib.normals[v + 1], attrib.normals[v + 2]);
				}
			}

			if (!attrib.texcoords.empty())
			{
				std::uint32_t idx = 0;

				for (auto& index : vertexSet)
				{
					auto v = std::get<2>(index) * 2;
					v = v < 0 ? attrib.texcoords.size() + v : v;
					texcoords[idx++].set(attrib.texcoords[v], 1.f - attrib.texcoords[v + 1]);
				}
			}

			auto mesh = std::make_shared<Mesh>();
			mesh->setVertexArray(std::move(vertices));
			mesh->setNormalArray(std::move(normals));
			mesh->setTexcoordArray(std::move(texcoords));

			Materials shapesMaterials(shapes.size());

			for (std::size_t i = 0; i < shapes.size(); i++)
			{
				auto& shape = shapes[i];

				std::size_t numIndices = 0;
				std::size_t numFace = shape.mesh.num_face_vertices.size();

				for (std::size_t f = 0; f < numFace; f++)
				{
					std::size_t numFaceVertices = shape.mesh.num_face_vertices[f];
					if (numFaceVertices == 3)
						numIndices += 3;
					else if (numFaceVertices == 4)
						numIndices += 6;
				}

				math::uint32s indices;
				indices.reserve(numIndices);

				for (std::size_t f = 0, offset = 0; f < numFace; f++)
				{
					std::size_t numFaceVertices = shape.mesh.num_face_vertices[f];
					if (numFaceVertices == 3)
					{
						auto index = shape.mesh.indices[offset++];
						auto index1 = shape.mesh.indices[offset++];
						auto index2 = shape.mesh.indices[offset++];

						indices.emplace_back(vertexMap.at(std::make_tuple(index.vertex_index, index.normal_index, index.texcoord_index)));
						indices.emplace_back(vertexMap.at(std::make_tuple(index1.vertex_index, index1.normal_index, index1.texcoord_index)));
						indices.emplace_back(vertexMap.at(std::make_tuple(index2.vertex_index, index2.normal_index, index2.texcoord_index)));
					}
					else if (numFaceVertices == 4)
					{
						auto index = shape.mesh.indices[offset++];
						auto index1 = shape.mesh.indices[offset++];
						auto index2 = shape.mesh.indices[offset++];
						auto index3 = shape.mesh.indices[offset++];

						indices.emplace_back(vertexMap.at(std::make_tuple(index.vertex_index, index.normal_index, index.texcoord_index)));
						indices.emplace_back(vertexMap.at(std::make_tuple(index1.vertex_index, index1.normal_index, index1.texcoord_index)));
						indices.emplace_back(vertexMap.at(std::make_tuple(index2.vertex_index, index2.normal_index, index2.texcoord_index)));

						indices.emplace_back(vertexMap.at(std::make_tuple(index2.vertex_index, index2.normal_index, index2.texcoord_index)));
						indices.emplace_back(vertexMap.at(std::make_tuple(index3.vertex_index, index3.normal_index, index3.texcoord_index)));
						indices.emplace_back(vertexMap.at(std::make_tuple(index.vertex_index, index.normal_index, index.texcoord_index)));
					}
				}

				mesh->setIndicesArray(std::move(indices), i);

				if (!shape.mesh.material_ids.empty())
				{
					auto index = shape.mesh.material_ids.front();
					if (index < standardMaterials.size())
						shapesMaterials[index] = standardMaterials[index];
				}
			}

			mesh->computeBoundingBox();

			auto object = std::make_shared<GameObject>();
			object->addComponent<MeshFilterComponent>(std::move(mesh));
			object->addComponent<MeshRendererComponent>(std::move(shapesMaterials));

			return object;
		}

		return nullptr;
	}

	std::shared_ptr<GameObject>
	OBJLoader::load(const std::filesystem::path& filepath) noexcept(false)
	{
		std::ifstream stream(filepath);
		if (stream)
			return load(stream);

		return nullptr;
	}
}