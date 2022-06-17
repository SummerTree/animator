#include <octoon/ass_loader.h>
#include <octoon/asset_database.h>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/transform_component.h>
#include <octoon/point_light_component.h>
#include <octoon/film_camera_component.h>
#include <octoon/mesh_renderer_component.h>

namespace octoon
{
	struct ASS_Material
	{
		math::float3 albedo;
		float materialType;
		math::float3 emission;

		float metallic;
		float roughness;
		float specular;
		float anisotropic;
		float ior;
		float clearcoat;
		float clearcoatgloss;
		float subsurface;
		float spectrans;

		std::string albedoTex;
		std::string metallicRoughnessTex;
		std::string normalmapTex;
	};

	struct ASS_Light
	{
		math::float3 position;
		math::float3 emission;
		math::float3 u;
		math::float3 v;
		float radius;
		float area;
		std::string type;
	};

	ASSLoader::ASSLoader() noexcept
	{
	}

	ASSLoader::~ASSLoader() noexcept
	{
	}

	bool
	ASSLoader::doCanRead(io::istream& stream) noexcept
	{
		return false;
	}

	bool
	ASSLoader::doCanRead(const char* type) noexcept
	{
		return std::strncmp(type, "ass", 3) == 0;
	}

	GameObjects
	ASSLoader::load(std::string_view filepath) noexcept(false)
	{
		static constexpr int kMaxLineLength = 2048;

		FILE* file = fopen(std::string(filepath).c_str(), "r");

		GameObjects objects;
		if (!file)
			return objects;

		try
		{
			std::map<std::string, std::shared_ptr<MeshStandardMaterial>> materialMap;
			std::string path = std::string(filepath.substr(0, filepath.find_last_of("/\\"))) + "/";

			char line[kMaxLineLength];
			auto defaultMaterial = std::make_shared<MeshStandardMaterial>();

			while (fgets(line, kMaxLineLength, file))
			{
				if (line[0] == '#')
					continue;

				char name[kMaxLineLength] = { 0 };

				if (sscanf(line, " material %s", name) == 1)
				{
					ASS_Material material;
					material.albedo = math::float3::One;
					material.emission = math::float3::Zero;
					material.metallic = 0;
					material.roughness = 0;
					material.specular = 0.5;
					material.materialType = 0.0f;
					material.ior = 1.5f;
					material.clearcoat = 0.0f;
					material.clearcoatgloss = 0.0f;
					material.anisotropic = 0.0f;
					material.subsurface = 0.0f;
					material.spectrans = 0.0f;

					char albedoTexName[100] = "None";
					char normalTexName[100] = "None";
					char metallicRoughnessTexName[100] = "None";

					while (fgets(line, kMaxLineLength, file))
					{
						if (strchr(line, '}'))
							break;

						sscanf(line, " name %s", name);
						sscanf(line, " color %f %f %f", &material.albedo.x, &material.albedo.y, &material.albedo.z);
						sscanf(line, " emission %f %f %f", &material.emission.x, &material.emission.y, &material.emission.z);
						sscanf(line, " materialType %f", &material.materialType);
						sscanf(line, " metallic %f", &material.metallic);
						sscanf(line, " roughness %f", &material.roughness);
						sscanf(line, " ior %f", &material.ior);
						sscanf(line, " specular %f", &material.specular);
						sscanf(line, " clearcoat %f", &material.clearcoat);
						sscanf(line, " clearcoatgloss %f", &material.clearcoatgloss);
						sscanf(line, " anisotropic %f", &material.anisotropic);
						sscanf(line, " subsurface %f", &material.subsurface);
						sscanf(line, " spectrans %f", &material.spectrans);

						sscanf(line, " albedotexture %s", albedoTexName);
						sscanf(line, " metallicRoughnessTexture %s", metallicRoughnessTexName);
						sscanf(line, " normalTexture %s", normalTexName);
					}

					if (strcmp(albedoTexName, "None") != 0) material.albedoTex = path + albedoTexName;
					if (strcmp(metallicRoughnessTexName, "None") != 0) material.metallicRoughnessTex = path + metallicRoughnessTexName;
					if (strcmp(normalTexName, "None") != 0) material.normalmapTex = path + normalTexName;

					if (materialMap.find(name) == materialMap.end())
					{
						auto standard = std::make_shared<MeshStandardMaterial>(name);
						standard->setColor(material.albedo);
						standard->setEmissive(material.emission);
						standard->setRoughness(material.roughness);
						standard->setSpecular(material.specular);
						standard->setMetalness(material.metallic);
						standard->setAnisotropy(material.anisotropic);
						standard->setRefractionRatio(material.ior);
						standard->setClearCoat(material.clearcoat);
						standard->setClearCoatRoughness(1.0f - material.clearcoatgloss);
						standard->setTransmission(material.spectrans);

						if (!material.albedoTex.empty())
						{
							auto texture = AssetDatabase::instance()->loadAssetAtPath<Texture>(material.albedoTex);
							if (texture)
							{
								texture->apply();
								standard->setColorMap(std::move(texture));
							}
						}

						if (!material.normalmapTex.empty())
						{
							auto texture = AssetDatabase::instance()->loadAssetAtPath<Texture>(material.normalmapTex);
							if (texture)
							{
								texture->apply();
								standard->setNormalMap(std::move(texture));
							}
						}

						if (!material.metallicRoughnessTex.empty())
						{
							auto texture = AssetDatabase::instance()->loadAssetAtPath<Texture>(material.metallicRoughnessTex);
							if (texture)
							{
								texture->apply();
								standard->setMetalness(1.0f);
								standard->setMetalnessMap(std::move(texture));
							}
						}

						materialMap[name] = standard;
					}
				}

				if (strstr(line, "light"))
				{
					ASS_Light light;
					math::float3 v1, v2;
					char light_type[20] = "None";

					while (fgets(line, kMaxLineLength, file))
					{
						if (strchr(line, '}'))
							break;

						sscanf(line, " position %f %f %f", &light.position.x, &light.position.y, &light.position.z);
						sscanf(line, " emission %f %f %f", &light.emission.x, &light.emission.y, &light.emission.z);

						sscanf(line, " radius %f", &light.radius);
						sscanf(line, " v1 %f %f %f", &v1.x, &v1.y, &v1.z);
						sscanf(line, " v2 %f %f %f", &v2.x, &v2.y, &v2.z);
						sscanf(line, " type %s", light_type);
					}

					if (strcmp(light_type, "Quad") == 0)
					{
						light.type = light_type;
						light.u = v1 - light.position;
						light.v = v2 - light.position;
						light.area = math::length(math::cross(light.u, light.v));
					}
					else if (strcmp(light_type, "Sphere") == 0)
					{
						light.type = light_type;
						light.area = 4.0f * math::PI * light.radius * light.radius;
					}

					if (light.type.empty())
					{
						auto object = std::make_shared<GameObject>();
						object->getComponent<TransformComponent>()->setTranslate(light.position);
						auto pointLight = object->addComponent<PointLightComponent>();
						pointLight->setColor(light.emission);
						objects.emplace_back(std::move(object));
					}
				}

				if (strstr(line, "Camera"))
				{
					math::float3 position;
					math::float3 lookAt;
					float fov = 45;
					float aperture = 0, focalDist = 1;

					while (fgets(line, kMaxLineLength, file))
					{
						if (strchr(line, '}'))
							break;

						sscanf(line, " position %f %f %f", &position.x, &position.y, &position.z);
						sscanf(line, " lookAt %f %f %f", &lookAt.x, &lookAt.y, &lookAt.z);
						sscanf(line, " aperture %f ", &aperture);
						sscanf(line, " focaldist %f", &focalDist);
						sscanf(line, " fov %f", &fov);
					}

					auto object = std::make_shared<GameObject>();
					object->getComponent<TransformComponent>()->setTransform(math::makeLookatLH(position, lookAt, math::float3::UnitY));
					auto filmCamera = object->addComponent<FilmCameraComponent>();
					filmCamera->setFov(fov);
					filmCamera->setAperture(aperture);
					filmCamera->setFocusDistance(focalDist);

					//objects.emplace_back(std::move(object));
				}

				if (strstr(line, "mesh"))
				{
					std::string filename;
					math::float3 pos = math::float3::Zero;
					math::float3 scale = math::float3::One;
					std::shared_ptr<MeshStandardMaterial> material;
					char meshName[200] = "None";

					while (fgets(line, kMaxLineLength, file))
					{
						if (strchr(line, '}'))
							break;

						char matName[100];

						sscanf(line, " name %[^\t\n]s", meshName);
						sscanf(line, " position %f %f %f", &pos.x, &pos.y, &pos.z);
						sscanf(line, " scale %f %f %f", &scale.x, &scale.y, &scale.z);

						char fileName[2048];
						if (sscanf(line, " file %s", fileName) == 1)
							filename = path + fileName;

						if (sscanf(line, " material %s", matName) == 1)
							material = materialMap.find(matName) != materialMap.end() ? materialMap[matName] : defaultMaterial;
					}

					if (!filename.empty())
					{
						auto model = octoon::AssetDatabase::instance()->loadAssetAtPath<octoon::GameObject>(filename);
						if (model)
						{
							model->setName(meshName);
							model->getComponent<TransformComponent>()->setTransform(pos, math::Quaternion::Zero, scale);

							auto renderer = model->getComponent<MeshRendererComponent>();
							if (renderer)
								renderer->setMaterial(material);
							else
								model->addComponent<MeshRendererComponent>(material);

							objects.push_back(std::move(model));
						}
					}
				}
			}

			fclose(file);
		}
		catch (...)
		{
			fclose(file);
		}

		return objects;
	}
}