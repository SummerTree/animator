#ifndef OCTOON_MESH_COLOR_MATERIAL_H_
#define OCTOON_MESH_COLOR_MATERIAL_H_

#include <octoon/material/material.h>

namespace octoon
{
	class OCTOON_EXPORT MeshColorMaterial final : public Material
	{
		OctoonDeclareSubClass(MeshColorMaterial, Material);
	public:
		MeshColorMaterial() noexcept;
		MeshColorMaterial(const math::float3& color) noexcept;
		virtual ~MeshColorMaterial() noexcept;

		void setColor(const math::float3& color) noexcept;
		const math::float3& getColor() const noexcept;

		void setOpacity(float opacity) noexcept;
		float getOpacity() const noexcept;

		void setColorMap(const hal::GraphicsTexturePtr& map) noexcept;
		const hal::GraphicsTexturePtr& getColorMap() const noexcept;

		std::shared_ptr<Material> clone() const noexcept override;

	private:
		MeshColorMaterial(const MeshColorMaterial&) = delete;
		MeshColorMaterial& operator=(const MeshColorMaterial&) = delete;

	private:
		float opacity_;
		math::float3 color_;
		hal::GraphicsTexturePtr edgeTexture_;
	};
}

#endif