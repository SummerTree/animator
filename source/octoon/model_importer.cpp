#include <octoon/model_importer.h>
#include <octoon/asset_database.h>
#include <octoon/texture/texture.h>
#include <octoon/pmx_loader.h>
#include <octoon/runtime/uuid.h>
#include <octoon/runtime/string.h>
#include <octoon/mesh_animation_component.h>
#include <octoon/mesh/sphere_mesh.h>
#include <fstream>
#include <filesystem>
#include <codecvt>

namespace octoon
{
	OctoonImplementSingleton(ModelImporter)

	ModelImporter::ModelImporter() noexcept
		: previewWidth_(256)
		, previewHeight_(256)
	{
	}

	ModelImporter::~ModelImporter() noexcept
	{
		this->close();
	}

	void
	ModelImporter::open(std::string indexPath) noexcept(false)
	{
		AssetImporter::open(indexPath);
		this->initRenderScene();
	}

	void
	ModelImporter::close() noexcept
	{
		camera_.reset();
		geometry_.reset();
		directionalLight_.reset();
		environmentLight_.reset();
		scene_.reset();
		framebuffer_.reset();
	}

	nlohmann::json
	ModelImporter::createPackage(std::string_view filepath) noexcept(false)
	{
		octoon::PMX pmx;

		if (octoon::PMX::load(filepath, pmx))
		{
			std::wstring wfilepath = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(std::string(filepath));

			auto uuid = octoon::make_guid();
			auto rootPath = std::filesystem::path(assertPath_).append(uuid);
			auto filename = std::filesystem::path(wfilepath).filename();
			auto modelPath = std::filesystem::path(rootPath).append(uuid + ".pmx");
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			try
			{
				std::filesystem::create_directories(rootPath);
				std::filesystem::copy(wfilepath, modelPath);
				std::filesystem::permissions(modelPath, std::filesystem::perms::owner_write);

				for (auto& texture : pmx.textures)
				{
					std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;
					auto path = cv.from_bytes(std::string(texture.fullpath));
					auto texturePath = std::filesystem::path(rootPath).append(texture.name);

					if (std::filesystem::exists(path) && !std::filesystem::exists(texturePath))
					{
						auto textureRootPath = octoon::runtime::string::directory(texturePath.string());
						std::filesystem::create_directories(textureRootPath);
						std::filesystem::copy(path, texturePath);
						std::filesystem::permissions(texturePath, std::filesystem::perms::owner_write);
					}
				}

				octoon::math::BoundingBox bound;
				for (auto& v : pmx.vertices)
					bound.encapsulate(octoon::math::float3(v.position.x, v.position.y, v.position.z));

				auto minBounding = bound.box().min;
				auto maxBounding = bound.box().max;

				auto writePreview = [this](const std::shared_ptr<octoon::Geometry>& geometry, const octoon::math::BoundingBox& boundingBox, std::filesystem::path outputPath) -> nlohmann::json
				{
					octoon::Texture texture;
					auto previewPath = std::filesystem::path(outputPath).append(octoon::make_guid() + ".png");
					this->createModelPreview(geometry, boundingBox, texture);
					texture.save(previewPath.string(), "png");
					return previewPath.string();
				};

				auto geometry = octoon::PMXLoader::loadGeometry(pmx);

				for (auto& v : pmx.bones)
				{
					if (std::wcscmp(v.name.name, L"×óÄ¿") == 0 || std::wcscmp(v.name.name, L"ÓÒÄ¿") == 0)
					{
						auto position = v.position.y;
						camera_->setTransform(octoon::math::makeLookatRH(octoon::math::float3(0, position, 10), octoon::math::float3(0, position, 0), -octoon::math::float3::UnitY));
					}
				}

				nlohmann::json package;
				package["uuid"] = uuid;
				package["visible"] = true;
				package["name"] = (char*)filename.u8string().c_str();
				package["path"] = (char*)modelPath.u8string().c_str();
				package["preview"] = writePreview(geometry, bound, rootPath);
				package["bound"][0] = { minBounding.x, minBounding.y, minBounding.z };
				package["bound"][1] = { maxBounding.x, maxBounding.y, maxBounding.z };

				std::ofstream ifs(packagePath, std::ios_base::binary);
				if (ifs)
				{
					auto dump = package.dump();
					ifs.write(dump.c_str(), dump.size());
				}

				this->indexList_.push_back(uuid);

				return package;
			}
			catch (std::exception& e)
			{
				if (std::filesystem::exists(rootPath))
					std::filesystem::remove_all(rootPath);

				throw e;
			}
		}

		return nlohmann::json();
	}

	nlohmann::json
	ModelImporter::createPackage(const octoon::GameObjectPtr& gameObject) const noexcept
	{
		auto it = assetList_.find(gameObject);
		if (it != assetList_.end())
		{
			auto& package = (*it).second;

			nlohmann::json json;
			json["uuid"] = package["uuid"].get<nlohmann::json::string_t>();

			return json;
		}
		auto path = assetPathList_.find(gameObject);
		if (path != assetPathList_.end())
		{
			nlohmann::json json;
			json["path"] = (char*)(*path).second.c_str();

			return json;
		}
		auto assetPath = AssetDatabase::instance()->getAssetPath(gameObject);
		if (!assetPath.empty())
		{
			nlohmann::json json;
			json["path"] = assetPath;

			return json;
		}

		return std::string();
	}

	octoon::GameObjectPtr
	ModelImporter::loadPackage(const nlohmann::json& package, octoon::PMXLoadFlags flags) noexcept(false)
	{
		if (package["path"].is_string())
		{
			auto path = package["path"].get<nlohmann::json::string_t>();
			auto model = octoon::PMXLoader::load(path, flags);
			if (model)
			{
				assetList_[model] = package;
				return model;
			}
		}

		return nullptr;
	}

	octoon::GameObjectPtr
	ModelImporter::loadMetaData(const nlohmann::json& metadata, octoon::PMXLoadFlags flags) noexcept
	{
		if (metadata.find("uuid") != metadata.end())
		{
			auto uuid = metadata["uuid"].get<nlohmann::json::string_t>();
			auto package = this->getPackage(uuid);
			if (package.is_object())
				return this->loadPackage(package, flags);
		
		}
		if (metadata.find("path") != metadata.end())
		{
			auto path = metadata["path"].get<nlohmann::json::string_t>();
			auto texture = AssetDatabase::instance()->loadAssetAtPath(path, flags);
			if (texture) return texture->downcast_pointer<octoon::GameObject>();
		}

		return nullptr;
	}

	void
	ModelImporter::createModelPreview(const std::shared_ptr<octoon::Geometry>& geometry, const octoon::math::BoundingBox& boundingBox, octoon::Texture& texture)
	{
		assert(geometry);

		if (scene_)
		{
			auto min = boundingBox.box().min;
			auto max = boundingBox.box().max;

			auto renderer = octoon::Renderer::instance();
			if (renderer)
			{
				geometry_->setMesh(geometry->getMesh());
				geometry_->setMaterials(geometry->getMaterials());

				octoon::Renderer::instance()->render(scene_);

				geometry_->setDirty(true);
			}

			auto framebufferDesc = framebuffer_->getFramebufferDesc();
			auto width = framebufferDesc.getWidth();
			auto height = framebufferDesc.getHeight();

			auto colorTexture = framebufferDesc.getColorAttachment(0).getBindingTexture();

			std::uint8_t* data;
			if (colorTexture->map(0, 0, framebufferDesc.getWidth(), framebufferDesc.getHeight(), 0, (void**)&data))
			{
				texture.create(octoon::Format::R8G8B8SRGB, width, height);

				auto destData = texture.data();

				constexpr auto size = 16;

				for (std::uint32_t y = 0; y < height; y++)
				{
					for (std::uint32_t x = 0; x < width; x++)
					{
						auto src = (y * height + x) * 4;
						auto dest = (y * height + x) * 3;

						std::uint8_t u = x / size % 2;
						std::uint8_t v = y / size % 2;
						std::uint8_t bg = (u == 0 && v == 0 || u == v) ? 200u : 255u;

						destData[dest] = octoon::math::lerp(bg, data[src], data[src + 3] / 255.f);
						destData[dest + 1] = octoon::math::lerp(bg, data[src + 1], data[src + 3] / 255.f);
						destData[dest + 2] = octoon::math::lerp(bg, data[src + 2], data[src + 3] / 255.f);
					}
				}

				colorTexture->unmap();
			}
		}
	}

	void
	ModelImporter::initRenderScene() noexcept(false)
	{
		auto renderer = octoon::Renderer::instance();
		if (renderer)
		{
			std::uint32_t width = previewWidth_;
			std::uint32_t height = previewHeight_;

			octoon::GraphicsTextureDesc textureDesc;
			textureDesc.setSize(width, height);
			textureDesc.setTexDim(octoon::TextureDimension::Texture2D);
			textureDesc.setTexFormat(octoon::GraphicsFormat::R8G8B8A8UNorm);
			auto colorTexture = renderer->getGraphicsDevice()->createTexture(textureDesc);
			if (!colorTexture)
				throw std::runtime_error("createTexture() failed");

			octoon::GraphicsTextureDesc depthTextureDesc;
			depthTextureDesc.setSize(width, height);
			depthTextureDesc.setTexDim(octoon::TextureDimension::Texture2D);
			depthTextureDesc.setTexFormat(octoon::GraphicsFormat::D16UNorm);
			auto depthTexture = renderer->getGraphicsDevice()->createTexture(depthTextureDesc);
			if (!depthTexture)
				throw std::runtime_error("createTexture() failed");

			octoon::GraphicsFramebufferLayoutDesc framebufferLayoutDesc;
			framebufferLayoutDesc.addComponent(octoon::GraphicsAttachmentLayout(0, octoon::GraphicsImageLayout::ColorAttachmentOptimal, octoon::GraphicsFormat::R8G8B8A8UNorm));
			framebufferLayoutDesc.addComponent(octoon::GraphicsAttachmentLayout(1, octoon::GraphicsImageLayout::DepthStencilAttachmentOptimal, octoon::GraphicsFormat::D16UNorm));

			octoon::GraphicsFramebufferDesc framebufferDesc;
			framebufferDesc.setWidth(width);
			framebufferDesc.setHeight(height);
			framebufferDesc.setFramebufferLayout(renderer->getGraphicsDevice()->createFramebufferLayout(framebufferLayoutDesc));
			framebufferDesc.setDepthStencilAttachment(octoon::GraphicsAttachmentBinding(depthTexture, 0, 0));
			framebufferDesc.addColorAttachment(octoon::GraphicsAttachmentBinding(colorTexture, 0, 0));

			framebuffer_ = renderer->getGraphicsDevice()->createFramebuffer(framebufferDesc);
			if (!framebuffer_)
				throw std::runtime_error("createFramebuffer() failed");

			camera_ = std::make_shared<octoon::PerspectiveCamera>(23.9f, 1, 100);
			camera_->setClearColor(octoon::math::float4::Zero);
			camera_->setClearFlags(octoon::ClearFlagBits::AllBit);
			camera_->setFramebuffer(framebuffer_);

			geometry_ = std::make_shared<octoon::Geometry>();
			geometry_->setMesh(std::make_shared<octoon::SphereMesh>(0.5));

			octoon::math::Quaternion q1;
			q1.makeRotation(octoon::math::float3::UnitX, octoon::math::PI / 2.75f);
			octoon::math::Quaternion q2;
			q2.makeRotation(octoon::math::float3::UnitY, octoon::math::PI / 4.6f);

			directionalLight_ = std::make_shared<octoon::DirectionalLight>();
			directionalLight_->setColor(octoon::math::float3(1, 1, 1));
			directionalLight_->setTransform(octoon::math::float4x4(q1 * q2));

			environmentLight_ = std::make_shared<octoon::EnvironmentLight>();
			environmentLight_->setColor(octoon::math::float3::One * 0.9f);

			scene_ = std::make_unique<octoon::RenderScene>();
			scene_->addRenderObject(camera_.get());
			scene_->addRenderObject(environmentLight_.get());
			scene_->addRenderObject(geometry_.get());
		}
	}
}