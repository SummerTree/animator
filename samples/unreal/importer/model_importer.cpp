#include "model_importer.h"
#include "unreal_behaviour.h"
#include <octoon/image/image.h>
#include <octoon/pmx_loader.h>
#include <octoon/runtime/uuid.h>
#include <octoon/runtime/string.h>
#include <fstream>
#include <filesystem>
#include <codecvt>

namespace unreal
{
	OctoonImplementSingleton(ModelImporter)

	ModelImporter::ModelImporter() noexcept
		: previewWidth_(128)
		, previewHeight_(128)
	{
	}

	ModelImporter::~ModelImporter() noexcept
	{
	}

	void
	ModelImporter::open(std::string indexPath) noexcept(false)
	{
		this->initRenderScene();

		if (std::filesystem::exists(indexPath))
		{
			this->assertPath_ = indexPath;
			this->initPackageIndices();
		}
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

	octoon::GameObjectPtr
	ModelImporter::importModel(std::string_view path) noexcept(false)
	{
		auto ext = path.substr(path.find_last_of("."));
		if (ext == ".pmx")
		{
			auto model = octoon::PMXLoader::load(path);
			if (model)
			{
				modelPathList_[model] = path;
				return model;
			}
		}
		else if (ext == ".abc")
		{
			auto model = octoon::GameObject::create();
			if (model)
			{
				model->addComponent<octoon::MeshAnimationComponent>(path);
				modelPathList_[model] = path;
				return model;
			}
		}

		return nullptr;
	}

	MutableLiveData<nlohmann::json>&
	ModelImporter::getIndexList() noexcept
	{
		return modelIndexList_;
	}

	nlohmann::json
	ModelImporter::importPackage(std::string_view filepath) noexcept(false)
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
					}
				}

				octoon::math::BoundingBox bound;
				for (auto& v : pmx.vertices)
					bound.encapsulate(octoon::math::float3(v.position.x, v.position.y, v.position.z));

				auto minBounding = bound.box().min;
				auto maxBounding = bound.box().max;

				auto writePreview = [this](const std::shared_ptr<octoon::Geometry>& geometry, const octoon::math::BoundingBox& boundingBox, std::filesystem::path outputPath) -> nlohmann::json
				{
					QPixmap pixmap;
					auto previewPath = std::filesystem::path(outputPath).append(octoon::make_guid() + ".png");
					this->createModelPreview(geometry, boundingBox, pixmap, previewWidth_, previewHeight_);
					pixmap.save(QString::fromStdString(previewPath.string()), "png");
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

				nlohmann::json item;
				item["uuid"] = uuid;
				item["name"] = filename.u8string();
				item["path"] = modelPath.u8string();
				item["preview"] = writePreview(geometry, bound, rootPath);
				item["bound"][0] = { minBounding.x, minBounding.y, minBounding.z };
				item["bound"][1] = { maxBounding.x, maxBounding.y, maxBounding.z };

				std::ofstream ifs(packagePath, std::ios_base::binary);
				if (ifs)
				{
					auto dump = item.dump();
					ifs.write(dump.c_str(), dump.size());
				}

				this->modelIndexList_.getValue().push_back(uuid);

				return item;
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
	ModelImporter::getPackage(std::string_view uuid) noexcept
	{
		auto it = this->packageList_.find(std::string(uuid));
		if (it == this->packageList_.end())
		{
			std::ifstream ifs(std::filesystem::path(assertPath_).append(uuid).append("package.json"));
			if (ifs)
			{
				auto package = nlohmann::json::parse(ifs);
				this->packageList_[std::string(uuid)] = package;
				return package;
			}
			else
			{
				return nlohmann::json();
			}			
		}

		return this->packageList_[std::string(uuid)];
	}

	octoon::GameObjectPtr
	ModelImporter::loadPackage(const nlohmann::json& package) noexcept
	{
		if (package["path"].is_string())
		{
			auto path = package["path"].get<nlohmann::json::string_t>();
			auto model = octoon::PMXLoader::load(path);
			if (model)
			{
				modelList_[model] = package;
				return model;
			}
		}

		return nullptr;
	}

	bool
	ModelImporter::removePackage(std::string_view uuid) noexcept
	{
		try
		{
			auto& indexList = this->modelIndexList_.getValue();

			for (auto it = indexList.begin(); it != indexList.end(); ++it)
			{
				if ((*it).get<nlohmann::json::string_t>() == uuid)
				{
					auto packagePath = std::filesystem::path(assertPath_).append(uuid);
					std::filesystem::remove_all(packagePath);

					auto package = this->packageList_.find(std::string(uuid));
					if (package != this->packageList_.end())
						this->packageList_.erase(package);

					indexList.erase(it);
					return true;
				}
			}
		}
		catch (const std::exception&)
		{
		}

		return false;
	}

	octoon::GameObjectPtr
	ModelImporter::loadMetaData(const nlohmann::json& metadata) noexcept
	{
		if (metadata.find("uuid") != metadata.end())
		{
			auto uuid = metadata["uuid"].get<nlohmann::json::string_t>();
			auto package = this->getPackage(uuid);
			if (package.is_object())
				return this->loadPackage(package);
		
		}
		if (metadata.find("path") != metadata.end())
		{
			auto path = metadata["path"].get<nlohmann::json::string_t>();
			return this->importModel(path);
		}

		return nullptr;
	}

	nlohmann::json
	ModelImporter::createMetadata(const octoon::GameObjectPtr& gameObject) const noexcept
	{
		auto it = modelList_.find(gameObject);
		if (it != modelList_.end())
		{
			auto& package = (*it).second;

			nlohmann::json json;
			json["uuid"] = package["uuid"].get<nlohmann::json::string_t>();

			return json;
		}
		auto path = modelPathList_.find(gameObject);
		if (path != modelPathList_.end())
		{
			nlohmann::json json;
			json["path"] = (*path).second;

			return json;
		}

		return std::string();
	}

	void
	ModelImporter::save() noexcept(false)
	{
		try
		{
			if (!std::filesystem::exists(assertPath_))
				std::filesystem::create_directory(assertPath_);

			std::ofstream ifs(assertPath_ + "/index.json", std::ios_base::binary);
			if (ifs)
			{
				auto data = this->modelIndexList_.getValue().dump();
				ifs.write(data.c_str(), data.size());
			}
		}
		catch (...)
		{
		}
	}

	void
	ModelImporter::createModelPreview(const std::shared_ptr<octoon::Geometry>& geometry, const octoon::math::BoundingBox& boundingBox, QPixmap& pixmap, int w, int h)
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
				QImage image(width, height, QImage::Format_RGB888);

				constexpr auto size = 16;

				for (std::uint32_t y = 0; y < height; y++)
				{
					for (std::uint32_t x = 0; x < width; x++)
					{
						auto n = (y * height + x) * 4;

						std::uint8_t u = x / size % 2;
						std::uint8_t v = y / size % 2;
						std::uint8_t bg = (u == 0 && v == 0 || u == v) ? 200u : 255u;

						auto r = octoon::math::lerp(bg, data[n], data[n + 3] / 255.f);
						auto g = octoon::math::lerp(bg, data[n + 1], data[n + 3] / 255.f);
						auto b = octoon::math::lerp(bg, data[n + 2], data[n + 3] / 255.f);

						image.setPixelColor((int)x, (int)y, QColor::fromRgb(r, g, b));
					}
				}

				colorTexture->unmap();

				pixmap.convertFromImage(image);
				pixmap = pixmap.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			}
		}
	}

	void
	ModelImporter::initRenderScene() noexcept(false)
	{
		auto renderer = octoon::Renderer::instance();
		if (renderer)
		{
			std::uint32_t width = previewWidth_ * 2;
			std::uint32_t height = previewHeight_ * 2;

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
			geometry_->setMesh(octoon::SphereMesh::create(0.5));

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

	void
	ModelImporter::initPackageIndices() noexcept(false)
	{
		auto& indexList = this->modelIndexList_.getValue();

		std::ifstream indexStream(assertPath_ + "/index.json");
		if (indexStream)
			indexList = nlohmann::json::parse(indexStream);

		bool needUpdateIndexFile = false;

		std::set<std::string> indexSet;

		for (auto& it : indexList)
		{		 
			if (!std::filesystem::exists(std::filesystem::path(assertPath_).append(it.get<nlohmann::json::string_t>())))
				needUpdateIndexFile = true;
			else
				indexSet.insert(it.get<nlohmann::json::string_t>());
		}

		for (auto& it : std::filesystem::directory_iterator(assertPath_))
		{
			if (std::filesystem::is_directory(it))
			{
				auto filepath = it.path();
				auto filename = filepath.filename();

				auto index = indexSet.find(filename.string());
				if (index == indexSet.end())
				{
					if (std::filesystem::exists(filepath.append("package.json")))
					{
						needUpdateIndexFile = true;
						indexSet.insert(filename.string());
					}
				}
			}
		}

		if (needUpdateIndexFile)
		{
			nlohmann::json json;
			for (auto& it : indexSet)
				json += it;

			indexList = json;
			this->save();
		}
	}
}