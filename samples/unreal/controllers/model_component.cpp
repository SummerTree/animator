#include "model_component.h"
#include "unreal_behaviour.h"
#include <octoon/image/image.h>
#include <octoon/pmx_loader.h>
#include <octoon/model_loader.h>
#include <octoon/mesh_loader.h>
#include <octoon/runtime/string.h>
#include <fstream>
#include <filesystem>
#include <quuid.h>
#include <codecvt>

namespace unreal
{
	ModelComponent::ModelComponent() noexcept
		: previewWidth_(128)
		, previewHeight_(128)
	{
	}

	ModelComponent::~ModelComponent() noexcept
	{
	}

	nlohmann::json
	ModelComponent::importModel(std::string_view filepath) noexcept(false)
	{
		octoon::PMX pmx;
		octoon::PmxLoader loader;
		if (loader.load(filepath, pmx))
		{
			auto id = QUuid::createUuid().toString();
			auto uuid = id.toStdString().substr(1, id.length() - 2);

			std::wstring wfilepath = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(std::string(filepath));

			auto rootPath = std::filesystem::path(this->getModel()->modelPath).append(uuid);
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
					auto id = QUuid::createUuid().toString();
					auto uuid = id.toStdString().substr(1, id.length() - 2);
					auto previewPath = std::filesystem::path(outputPath).append(uuid + ".png");
					this->createModelPreview(geometry, boundingBox, pixmap, previewWidth_, previewHeight_);
					pixmap.save(QString::fromStdString(previewPath.string()), "png");
					return previewPath.string();
				};

				auto geometry = octoon::MeshLoader::load(octoon::ModelLoader::load(pmx));

				for (auto& v : pmx.bones)
				{
					if (std::wcscmp(v.name.name, L"î^") == 0)
					{
						auto position = v.position.y + 1;
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

				this->getModel()->modelIndexList_.getValue().push_back(uuid);

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
	ModelComponent::getPackage(std::string_view uuid) noexcept
	{
		auto it = this->packageList_.find(std::string(uuid));
		if (it == this->packageList_.end())
		{
			std::ifstream ifs(std::filesystem::path(this->getModel()->modelPath).append(uuid).append("package.json"));
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

	bool
	ModelComponent::removePackage(std::string_view uuid) noexcept
	{
		try
		{
			auto& indexList = this->getModel()->modelIndexList_.getValue();

			for (auto it = indexList.begin(); it != indexList.end(); ++it)
			{
				if ((*it).get<nlohmann::json::string_t>() == uuid)
				{
					auto packagePath = std::filesystem::path(this->getModel()->modelPath).append(uuid);
					std::filesystem::remove_all(packagePath);

					auto package = this->packageList_.find(std::string(uuid));
					if (package != this->packageList_.end())
						this->packageList_.erase(package);

					indexList.erase(it);
					return true;
				}
			}
		}
		catch (...)
		{
		}

		return false;
	}

	void
	ModelComponent::save() noexcept(false)
	{
		try
		{
			if (!std::filesystem::exists(this->getModel()->modelPath))
				std::filesystem::create_directory(this->getModel()->modelPath);

			std::ofstream ifs(this->getModel()->modelPath + "/index.json", std::ios_base::binary);
			if (ifs)
			{
				auto data = this->getModel()->modelIndexList_.getValue().dump();
				ifs.write(data.c_str(), data.size());
			}
		}
		catch (...)
		{
		}
	}

	void
	ModelComponent::createModelPreview(const std::shared_ptr<octoon::Geometry>& geometry, const octoon::math::BoundingBox& boundingBox, QPixmap& pixmap, int w, int h)
	{
		assert(geometry);

		if (scene_)
		{
			auto min = boundingBox.box().min;
			auto max = boundingBox.box().max;

			auto renderer = this->getFeature<octoon::VideoFeature>()->getRenderer();
			if (renderer)
			{
				geometry_->setMesh(geometry->getMesh());
				geometry_->setMaterials(geometry->getMaterials());

				renderer->render(scene_);

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
	ModelComponent::initRenderScene() noexcept(false)
	{
		auto renderer = this->getFeature<octoon::VideoFeature>()->getRenderer();
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

			camera_ = std::make_shared<octoon::PerspectiveCamera>(20, 1, 100);
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
			//scene_->addRenderObject(directionalLight_.get());
			scene_->addRenderObject(environmentLight_.get());
			scene_->addRenderObject(geometry_.get());
		}
	}

	void
	ModelComponent::initPackageIndices() noexcept(false)
	{
		auto& indexList = this->getModel()->modelIndexList_.getValue();

		std::ifstream indexStream(this->getModel()->modelPath + "/index.json");
		if (indexStream)
			indexList = nlohmann::json::parse(indexStream);

		bool needUpdateIndexFile = false;

		std::set<std::string> indexSet;

		for (auto& it : indexList)
		{		 
			if (!std::filesystem::exists(std::filesystem::path(this->getModel()->modelPath).append(it.get<nlohmann::json::string_t>())))
				needUpdateIndexFile = true;
			else
				indexSet.insert(it.get<nlohmann::json::string_t>());
		}

		for (auto& it : std::filesystem::directory_iterator(this->getModel()->modelPath))
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

	void
	ModelComponent::onEnable() noexcept
	{
		this->initRenderScene();

		if (std::filesystem::exists(this->getModel()->modelPath))
			this->initPackageIndices();
		else
			std::filesystem::create_directory(this->getModel()->modelPath);
	}

	void
	ModelComponent::onDisable() noexcept
	{
	}
}