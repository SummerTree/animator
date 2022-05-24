#ifndef OCTOON_MESH_STANDARD_MATERIAL_H_
#define OCTOON_MESH_STANDARD_MATERIAL_H_

#include <octoon/material/material.h>

namespace octoon
{
	class OCTOON_EXPORT MeshStandardMaterial final : public Material
	{
		OctoonDeclareSubClass(MeshStandardMaterial, Material);
	public:
		MeshStandardMaterial() noexcept;
		MeshStandardMaterial(std::string_view name) noexcept;
		MeshStandardMaterial(const math::float3& color) noexcept;
		virtual ~MeshStandardMaterial() noexcept;

		void setColor(const math::float3& color) noexcept;
		void setEmissive(const math::float3& color) noexcept;
		void setEmissiveIntensity(float intensity) noexcept;
		void setOpacity(float opacity) noexcept;
		void setSmoothness(float smoothness) noexcept;
		void setRoughness(float roughness) noexcept;
		void setMetalness(float metalness) noexcept;
		void setAnisotropy(float anisotropy) noexcept;
		void setSheen(float sheen) noexcept;
		void setSpecular(float reflectivity) noexcept;
		void setRefractionRatio(float refractionRatio) noexcept;
		void setClearCoat(float clearCoat) noexcept;
		void setClearCoatRoughness(float clearCoatRoughness) noexcept;
		void setSubsurface(float subsurface) noexcept;
		void setSubsurfaceColor(const math::float3& subsurfaceColor) noexcept;
		void setReflectionRatio(float ior) noexcept;
		void setTransmission(float transmission) noexcept;
		void setLightMapIntensity(float intensity) noexcept;
		void setColorMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		void setOpacityMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		void setNormalMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		void setSpecularMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		void setRoughnessMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		void setMetalnessMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		void setAnisotropyMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		void setSheenMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		void setClearCoatMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		void setClearCoatRoughnessMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		void setEmissiveMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		void setSubsurfaceMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		void setSubsurfaceColorMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		void setLightMap(const std::shared_ptr<GraphicsTexture>& map) noexcept;
		void setOffset(const math::float2& offset) noexcept;
		void setRepeat(const math::float2& repeat) noexcept;
		void setNormalScale(const math::float2& repeat) noexcept;
		void setGamma(float gamma) noexcept;

		const math::float3& getColor() const noexcept;
		const math::float3& getSubsurfaceColor() const noexcept;
		const math::float3& getEmissive() const noexcept;

		float getEmissiveIntensity() const noexcept;
		float getOpacity() const noexcept;
		float getSmoothness() const noexcept;
		float getRoughness() const noexcept;
		float getMetalness() const noexcept;
		float getAnisotropy() const noexcept;
		float getSheen() const noexcept;
		float getSpecular() const noexcept;
		float getRefractionRatio() const noexcept;
		float getClearCoat() const noexcept;
		float getClearCoatRoughness() const noexcept;
		float getSubsurface() const noexcept;
		float getReflectionRatio() const noexcept;
		float getTransmission() const noexcept;
		float getLightMapIntensity() const noexcept;
		float getGamma() const noexcept;

		const std::shared_ptr<GraphicsTexture>& getColorMap() const noexcept;
		const std::shared_ptr<GraphicsTexture>& getOpacityMap() const noexcept;
		const std::shared_ptr<GraphicsTexture>& getNormalMap() const noexcept;
		const std::shared_ptr<GraphicsTexture>& getSpecularMap() const noexcept;
		const std::shared_ptr<GraphicsTexture>& getRoughnessMap() const noexcept;
		const std::shared_ptr<GraphicsTexture>& getMetalnessMap() const noexcept;
		const std::shared_ptr<GraphicsTexture>& getAnisotropyMap() const noexcept;
		const std::shared_ptr<GraphicsTexture>& getSheenMap() const noexcept;
		const std::shared_ptr<GraphicsTexture>& getClearCoatMap() const noexcept;
		const std::shared_ptr<GraphicsTexture>& getClearCoatRoughnessMap() const noexcept;		
		const std::shared_ptr<GraphicsTexture>& getSubsurfaceMap() const noexcept;
		const std::shared_ptr<GraphicsTexture>& getSubsurfaceColorMap() const noexcept;
		const std::shared_ptr<GraphicsTexture>& getEmissiveMap() const noexcept;
		const std::shared_ptr<GraphicsTexture>& getLightMap() const noexcept;

		const math::float2& getOffset() const noexcept;
		const math::float2& getRepeat() const noexcept;
		const math::float2& getNormalScale() const noexcept;

		void setReceiveShadow(bool enable) noexcept;
		bool getReceiveShadow() const noexcept;

		void copy(const MeshStandardMaterial& material);

		std::shared_ptr<Material> clone() const noexcept override;

	private:
		MeshStandardMaterial(const MeshStandardMaterial&) = delete;
		MeshStandardMaterial& operator=(const MeshStandardMaterial&) = delete;

	private:
		bool receiveShadow_;
		float gamma_;
		float opacity_;
		float metalness_;
		float roughness_;
		float anisotropy_;
		float sheen_;
		float specular_;
		float refractionRatio_;
		float clearCoat_;
		float clearCoatRoughness_;
		float subsurface_;
		float reflectionRatio_;
		float transmission_;
		float lightMapIntensity_;
		float emissiveIntensity_;
		math::float2 offset_;
		math::float2 repeat_;
		math::float2 normalScale_;
		math::float3 color_;
		math::float3 emissive_;
		math::float3 subsurfaceColor_;
		std::shared_ptr<GraphicsTexture> colorMap_;
		std::shared_ptr<GraphicsTexture> alphaMap_;
		std::shared_ptr<GraphicsTexture> normalMap_;
		std::shared_ptr<GraphicsTexture> specularMap_;
		std::shared_ptr<GraphicsTexture> roughnessMap_;
		std::shared_ptr<GraphicsTexture> metalnessMap_;
		std::shared_ptr<GraphicsTexture> anisotropyMap_;
		std::shared_ptr<GraphicsTexture> sheenMap_;
		std::shared_ptr<GraphicsTexture> clearcoatMap_;
		std::shared_ptr<GraphicsTexture> clearcoatRoughnessMap_;
		std::shared_ptr<GraphicsTexture> lightMap_;
		std::shared_ptr<GraphicsTexture> subsurfaceMap_;
		std::shared_ptr<GraphicsTexture> subsurfaceColorMap_;
		std::shared_ptr<GraphicsTexture> emissiveMap_;
	};
}

#endif