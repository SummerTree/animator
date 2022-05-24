#ifndef OCTOON_MESH_COPY_MATERIAL_H_
#define OCTOON_MESH_COPY_MATERIAL_H_

#include <octoon/material/material.h>

namespace octoon
{
	class OCTOON_EXPORT MeshCopyMaterial final : public Material
	{
		OctoonDeclareSubClass(MeshCopyMaterial, Material);
	public:
		MeshCopyMaterial() noexcept;
		virtual ~MeshCopyMaterial() noexcept;

		void setColorMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		const std::shared_ptr<GraphicsTexture>& getColorMap() const noexcept;

		std::shared_ptr<Material> clone() const noexcept override;

	private:
		MeshCopyMaterial(const MeshCopyMaterial&) = delete;
		MeshCopyMaterial& operator=(const MeshCopyMaterial&) = delete;

	private:
		std::shared_ptr<GraphicsTexture> colorMap_;
	};
}

#endif