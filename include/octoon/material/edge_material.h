#ifndef OCTOON_EDGE_MATERIAL_H_
#define OCTOON_EDGE_MATERIAL_H_

#include <octoon/material/material.h>

namespace octoon
{
	class OCTOON_EXPORT EdgeMaterial final : public Material
	{
		OctoonDeclareSubClass(EdgeMaterial, Material);
	public:
		EdgeMaterial() noexcept;
		EdgeMaterial(const math::float3& color) noexcept;
		virtual ~EdgeMaterial() noexcept;

		void setColor(const math::float3& color) noexcept;
		const math::float3& getColor() const noexcept;

		void setOpacity(float opacity) noexcept;
		float getOpacity() const noexcept;

		void setColorMap(const hal::GraphicsTexturePtr& map) noexcept;
		const hal::GraphicsTexturePtr& getColorMap() const noexcept;

		std::shared_ptr<Material> clone() const noexcept override;

	private:
		EdgeMaterial(const EdgeMaterial&) = delete;
		EdgeMaterial& operator=(const EdgeMaterial&) = delete;

	private:
		float opacity_;
		math::float3 color_;
		hal::GraphicsTexturePtr edgeTexture_;
	};
}

#endif