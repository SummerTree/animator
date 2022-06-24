#include <octoon/mesh_filter_component.h>
#include <octoon/asset_database.h>

namespace octoon
{
	OctoonImplementSubClass(MeshFilterComponent, GameComponent, "MeshFilter")

	MeshFilterComponent::MeshFilterComponent() noexcept
		: isSharedMesh_(false)
	{
	}

	MeshFilterComponent::MeshFilterComponent(Mesh&& mesh, bool sharedMesh) noexcept
	{
		this->setMesh(std::make_shared<Mesh>(std::move(mesh)), sharedMesh);
	}

	MeshFilterComponent::MeshFilterComponent(const Mesh& mesh, bool sharedMesh) noexcept
	{
		this->setMesh(std::make_shared<Mesh>(mesh), sharedMesh);
	}

	MeshFilterComponent::MeshFilterComponent(MeshPtr&& mesh, bool sharedMesh) noexcept
	{
		this->setMesh(std::move(mesh), sharedMesh);
	}

	MeshFilterComponent::MeshFilterComponent(const MeshPtr& mesh, bool sharedMesh) noexcept
	{
		this->setMesh(mesh, sharedMesh);
	}

	MeshFilterComponent::~MeshFilterComponent() noexcept
	{
	}

	void
	MeshFilterComponent::setMesh(Mesh&& mesh, bool sharedMesh) noexcept
	{
		this->setMesh(std::make_shared<Mesh>(std::move(mesh)));
	}

	void
	MeshFilterComponent::setMesh(MeshPtr&& mesh, bool sharedMesh) noexcept
	{
		if (mesh_ != mesh)
		{
			mesh_ = std::move(mesh);
			if (mesh_->getBoundingBoxAll().empty())
				mesh_->computeBoundingBox();

			isSharedMesh_ = sharedMesh;
			this->uploadMeshData();
		}
	}

	void
	MeshFilterComponent::setMesh(const MeshPtr& mesh, bool sharedMesh) noexcept
	{
		if (mesh_ != mesh)
		{
			mesh_ = mesh;
			if (mesh_->getBoundingBoxAll().empty())
				mesh_->computeBoundingBox();

			isSharedMesh_ = sharedMesh;
			this->uploadMeshData();
		}
	}

	const MeshPtr&
	MeshFilterComponent::getMesh() const noexcept
	{
		return mesh_;
	}

	bool
	MeshFilterComponent::isSharedMesh() const noexcept
	{
		return isSharedMesh_;
	}

	void
	MeshFilterComponent::uploadMeshData() noexcept
	{
		this->onMeshReplace();
	}

	void
	MeshFilterComponent::load(const nlohmann::json& json, AssetDatabase& assetDatabase) noexcept(false)
	{
		GameComponent::load(json, assetDatabase);

		if (json.contains("mesh"))
		{
			auto guid = json["mesh"]["guid"].get<std::string>();

			auto assetPath = assetDatabase.getAssetPath(guid);
			if (!assetPath.empty())
			{
				auto gameObject = assetDatabase.loadAssetAtPath<GameObject>(assetPath);
				if (gameObject)
				{
					auto mf = gameObject->getComponent<MeshFilterComponent>();
					if (mf)
						this->setMesh(mf->getMesh());
				}
			}
		}
	}

	void
	MeshFilterComponent::save(nlohmann::json& json, AssetDatabase& assetDatabase) const noexcept(false)
	{
		GameComponent::save(json, assetDatabase);

		auto guid = assetDatabase.getAssetGuid(this->getGameObject()->shared_from_this());
		if (!guid.empty())
			json["mesh"]["guid"] = guid;
	}

	GameComponentPtr
	MeshFilterComponent::clone() const noexcept
	{
		auto instance = std::make_shared<MeshFilterComponent>();
		instance->setName(instance->getName());
		instance->setMesh(mesh_ ? (isSharedMesh_ ? mesh_ : mesh_->clone()) : nullptr, isSharedMesh_);

		return instance;
	}

	void
	MeshFilterComponent::onActivate() except
	{
		this->addMessageListener("octoon:mesh:get", std::bind(&MeshFilterComponent::uploadMeshData, this));
		this->uploadMeshData();
	}

	void
	MeshFilterComponent::onDeactivate() noexcept
	{
	}

	void
	MeshFilterComponent::onMeshReplace() noexcept
	{
		this->trySendMessage("octoon:mesh:update", mesh_);
	}
}