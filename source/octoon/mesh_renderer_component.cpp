#include <octoon/mesh_renderer_component.h>
#include <octoon/transform_component.h>
#include <octoon/video_feature.h>
#include <octoon/video/renderer.h>
#include <octoon/asset_database.h>
#include <octoon/runtime/guid.h>

namespace octoon
{
	OctoonImplementSubClass(MeshRendererComponent, RenderComponent, "MeshRenderer")

	MeshRendererComponent::MeshRendererComponent() noexcept
		: visible_(true)
		, renderOrder_(0)
		, globalIllumination_(false)
	{
	}

	MeshRendererComponent::MeshRendererComponent(MaterialPtr&& material) noexcept
		: MeshRendererComponent()
	{
		this->setMaterial(std::move(material));
	}

	MeshRendererComponent::MeshRendererComponent(const MaterialPtr& material) noexcept
		: MeshRendererComponent()
	{
		this->setMaterial(material);
	}

	MeshRendererComponent::MeshRendererComponent(Materials&& materials) noexcept
		: MeshRendererComponent()
	{
		this->setMaterials(std::move(materials));
	}

	MeshRendererComponent::MeshRendererComponent(const Materials& materials) noexcept
		: MeshRendererComponent()
	{
		this->setMaterials(materials);
	}

	MeshRendererComponent::~MeshRendererComponent() noexcept
	{
	}

	void
	MeshRendererComponent::setVisible(bool visible) noexcept
	{
		if (this->visible_ != visible)
		{
			this->geometry_->setVisible(visible);
			this->visible_ = visible;
		}
	}

	bool
	MeshRendererComponent::getVisible() const noexcept
	{
		return this->visible_;
	}

	void
	MeshRendererComponent::setRenderOrder(std::int32_t order) noexcept
	{
		if (this->renderOrder_ != order)
		{
			if (this->geometry_)
				this->geometry_->setRenderOrder(order);
			this->renderOrder_ = order;
		}
	}

	std::int32_t
	MeshRendererComponent::getRenderOrder() const noexcept
	{
		return this->renderOrder_;
	}

	void
	MeshRendererComponent::setGlobalIllumination(bool enable) noexcept
	{
		if (this->globalIllumination_ != enable)
		{
			if (this->geometry_)
				this->geometry_->setGlobalIllumination(enable);
			this->globalIllumination_ = enable;
		}
	}

	bool
	MeshRendererComponent::getGlobalIllumination() const noexcept
	{
		return this->globalIllumination_;
	}

	void
	MeshRendererComponent::load(const nlohmann::json& json, AssetDatabase& assetDatabase) noexcept(false)
	{
		GameComponent::load(json, assetDatabase);

		if (json.contains("materials"))
		{
			auto& materialJson = json["materials"];

			std::vector<std::shared_ptr<octoon::Material>> materials;

			for (std::size_t i = 0; i < materialJson.size(); i ++)
			{
				auto& it = materialJson[i];
				if (it.is_object())
				{
					auto guid = it["guid"].get<std::string>();
					auto localId = it["localId"].get<int>();

					auto assetPath = assetDatabase.getAssetPath(guid);
					if (!assetPath.empty())
					{
						auto gameObject = assetDatabase.loadAssetAtPath<GameObject>(assetPath);
						if (gameObject)
						{
							auto meshRenderer = gameObject->getComponent<MeshRendererComponent>();
							if (meshRenderer)
								materials.push_back(meshRenderer->getMaterial(i));
						}
					}
				}
				else
				{
					auto data = it.get<nlohmann::json::string_t>();
					auto material = assetDatabase.loadAssetAtPath<octoon::Material>(assetDatabase.getAssetPath(data));
					materials.push_back(std::move(material));
				}
			}

			this->setMaterials(std::move(materials));
		}
	}

	void
	MeshRendererComponent::save(nlohmann::json& json, AssetDatabase& assetDatabase) const noexcept(false)
	{
		GameComponent::save(json, assetDatabase);

		auto& materials = this->getMaterials();

		for (std::size_t i = 0; i < materials.size(); i++)
		{
			if (materials[i])
			{
				std::string guid;
				std::int64_t localId;

				if (assetDatabase.getGUIDAndLocalIdentifier(materials[i], guid, localId))
				{
					nlohmann::json asset;
					asset["guid"] = guid;
					asset["localId"] = localId;

					json["materials"].push_back(std::move(asset));
				}
				else
				{
					auto materialPath = std::filesystem::path("Assets/Materials").append(make_guid() + ".mat");
					assetDatabase.createFolder(std::filesystem::path("Assets/Materials"));
					assetDatabase.createAsset(materials[i], materialPath);
					json["materials"].push_back(assetDatabase.getAssetGuid(materialPath));
				}
			}
		}
	}

	GameComponentPtr
	MeshRendererComponent::clone() const noexcept
	{
		auto instance = std::make_shared<MeshRendererComponent>();
		instance->setName(this->getName());

		if (!this->getMaterials().empty())
		{
			if (this->isSharedMaterial())
				instance->setMaterials(this->getMaterials());
			else
			{
				Materials materials;
				for (auto& it : this->getMaterials())
					materials.push_back(it->clone());
				instance->setMaterials(std::move(materials));
			}
		}

		return instance;
	}

	void
	MeshRendererComponent::onActivate() noexcept
	{
		this->addComponentDispatch(GameDispatchType::MoveAfter);
		this->addMessageListener("octoon:mesh:update", std::bind(&MeshRendererComponent::onMeshReplace, this, std::placeholders::_1));

		this->geometry_ = std::make_shared<Geometry>();
		this->geometry_->setOwnerListener(this);
		this->geometry_->setVisible(this->getVisible());
		this->geometry_->setGlobalIllumination(this->getGlobalIllumination());
		this->geometry_->setRenderOrder(this->getRenderOrder());

		this->onMoveAfter();
		this->onLayerChangeAfter();
		this->onMaterialReplace(this->getMaterials());

		this->sendMessage("octoon:mesh:get");
		this->getFeature<VideoFeature>()->getMainScene()->addRenderObject(geometry_.get());
	}

	void
	MeshRendererComponent::onDeactivate() noexcept
	{
		if (geometry_)
		{
			this->getFeature<VideoFeature>()->getMainScene()->removeRenderObject(geometry_.get());
			this->geometry_.reset();
		}

		this->removeComponentDispatch(GameDispatchType::MoveAfter);
		this->removeMessageListener("octoon:mesh:update", std::bind(&MeshRendererComponent::onMeshReplace, this, std::placeholders::_1));
	}

	void
	MeshRendererComponent::onMeshReplace(const std::any& mesh_) noexcept
	{
		assert(mesh_.type() == typeid(MeshPtr));
		this->uploadMeshData(std::any_cast<MeshPtr>(mesh_));
	}

	void
	MeshRendererComponent::onMaterialReplace(const Materials& material) noexcept
	{
		this->uploadMaterialData(material);
	}

	void
	MeshRendererComponent::onMoveAfter() noexcept
	{
		auto transform = this->getComponent<TransformComponent>();
		if (this->geometry_)
			this->geometry_->setTransform(transform->getTransform(), transform->getTransformInverse());
	}

	void
	MeshRendererComponent::onLayerChangeAfter() noexcept
	{
		if (this->geometry_)
			this->geometry_->setLayer(this->getGameObject()->getLayer());
	}

	void
	MeshRendererComponent::uploadMeshData(const MeshPtr& mesh) noexcept
	{
		if (geometry_)
			geometry_->setMesh(mesh);
	}

	void
	MeshRendererComponent::uploadMaterialData(const Materials& materials) noexcept
	{
		if (geometry_)
			geometry_->setMaterials(materials);
	}
}