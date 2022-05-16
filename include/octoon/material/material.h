#ifndef OCTOON_MATERIAL_H_
#define OCTOON_MATERIAL_H_

#include <octoon/math/math.h>
#include <octoon/material/shader.h>
#include <octoon/hal/graphics_state.h>

namespace octoon
{
	enum PropertyTypeInfo
	{
		PropertyTypeInfoFloat = 1 << 0,
		PropertyTypeInfoFloat2 = 1 << 1,
		PropertyTypeInfoFloat3 = 1 << 2,
		PropertyTypeInfoFloat4 = 1 << 3,
		PropertyTypeInfoString = 1 << 4,
		PropertyTypeInfoBool = 1 << 5,
		PropertyTypeInfoInt = 1 << 6,
		PropertyTypeInfoBuffer = 1 << 7,
		PropertyTypeInfoTexture = 1 << 8,
	};

	struct MaterialParam
	{
		std::string key;

		std::size_t length;
		std::size_t type;

		char* data;
		hal::GraphicsTexturePtr texture;
	};

	class OCTOON_EXPORT Material : public runtime::RttiInterface
	{
		OctoonDeclareSubClass(Material, runtime::RttiInterface);
	public:
		Material() noexcept;
		Material(std::string_view name) noexcept;
		Material(const std::shared_ptr<Shader>& shader) noexcept;
		virtual ~Material() noexcept;

		void setName(std::string_view name) noexcept;
		const std::string& getName() const noexcept;

		void setShader(const std::shared_ptr<Shader>& shader) noexcept;
		std::shared_ptr<Shader> getShader() const noexcept;

		void setColorBlends(hal::GraphicsColorBlends&& blends) noexcept;
		void setColorBlends(const hal::GraphicsColorBlends& blends) noexcept;
		hal::GraphicsColorBlends& getColorBlends() noexcept;
		const hal::GraphicsColorBlends& getColorBlends() const noexcept;

		void setCullMode(hal::CullMode mode) noexcept;
		void setPolygonMode(hal::PolygonMode mode) noexcept;
		void setPrimitiveType(hal::VertexType type) noexcept;
		void setFrontFace(hal::FrontFace face) noexcept;
		void setScissorTestEnable(bool enable) noexcept;
		void setLinear2sRGBEnable(bool enable) noexcept;
		void setMultisampleEnable(bool enable) noexcept;
		void setRasterizerDiscardEnable(bool enable) noexcept;
		void setLineWidth(float width) noexcept;

		void setDepthEnable(bool enable) noexcept;
		void setDepthWriteEnable(bool enable) noexcept;
		void setDepthBoundsEnable(bool enable) noexcept;
		void setDepthMin(float mix) noexcept;
		void setDepthMax(float max) noexcept;
		void setDepthFunc(hal::CompareFunction func) noexcept;
		void setDepthBiasEnable(bool enable) noexcept;
		void setDepthBias(float bias) noexcept;
		void setDepthSlopeScaleBias(float bias) noexcept;
		void setDepthBiasClamp(bool bias) noexcept;
		void setDepthClampEnable(bool enable) noexcept;

		void setStencilEnable(bool enable) noexcept;
		void setStencilFrontRef(std::uint32_t ref) noexcept;
		void setStencilFrontFunc(hal::CompareFunction func) noexcept;
		void setStencilFrontReadMask(std::uint32_t mask) noexcept;
		void setStencilFrontWriteMask(std::uint32_t mask) noexcept;
		void setStencilFrontFail(hal::StencilOp stencilOp) noexcept;
		void setStencilFrontZFail(hal::StencilOp stencilOp) noexcept;
		void setStencilFrontPass(hal::StencilOp stencilOp) noexcept;
		void setStencilBackRef(std::uint32_t ref) noexcept;
		void setStencilBackFunc(hal::CompareFunction func) noexcept;
		void setStencilBackReadMask(std::uint32_t mask) noexcept;
		void setStencilBackWriteMask(std::uint32_t mask) noexcept;
		void setStencilBackFail(hal::StencilOp stencilOp) noexcept;
		void setStencilBackZFail(hal::StencilOp stencilOp) noexcept;
		void setStencilBackPass(hal::StencilOp stencilOp) noexcept;

		hal::CullMode getCullMode() const noexcept;
		hal::PolygonMode getPolygonMode() const noexcept;
		hal::VertexType getPrimitiveType() const noexcept;
		hal::FrontFace getFrontFace() const noexcept;
		bool getScissorTestEnable() const noexcept;
		bool getLinear2sRGBEnable() const noexcept;
		bool getMultisampleEnable() const noexcept;
		bool getRasterizerDiscardEnable() const noexcept;
		float getLineWidth() const noexcept;

		bool getDepthEnable() const noexcept;
		bool getDepthWriteEnable() const noexcept;
		bool getDepthBoundsEnable() const noexcept;
		bool getDepthBiasEnable() const noexcept;
		bool getDepthBiasClamp() const noexcept;
		bool getDepthClampEnable() const noexcept;
		float getDepthMin() const noexcept;
		float getDepthMax() const noexcept;
		float getDepthBias() const noexcept;
		float getDepthSlopeScaleBias() const noexcept;
		hal::CompareFunction getDepthFunc() const noexcept;

		bool getStencilEnable() const noexcept;
		hal::CompareFunction getStencilFrontFunc() const noexcept;
		std::uint32_t getStencilFrontRef() const noexcept;
		std::uint32_t getStencilFrontReadMask() const noexcept;
		std::uint32_t getStencilFrontWriteMask() const noexcept;
		hal::StencilOp getStencilFrontFail() const noexcept;
		hal::StencilOp getStencilFrontZFail() const noexcept;
		hal::StencilOp getStencilFrontPass() const noexcept;
		hal::CompareFunction getStencilBackFunc() const noexcept;
		std::uint32_t getStencilBackRef() const noexcept;
		std::uint32_t getStencilBackReadMask() const noexcept;
		std::uint32_t getStencilBackWriteMask() const noexcept;
		hal::StencilOp getStencilBackFail() const noexcept;
		hal::StencilOp getStencilBackZFail() const noexcept;
		hal::StencilOp getStencilBackPass() const noexcept;

		bool set(std::string_view key, bool value) noexcept;
		bool set(std::string_view key, int value) noexcept;
		bool set(std::string_view key, float value) noexcept;
		bool set(std::string_view key, const math::Vector2& value) noexcept;
		bool set(std::string_view key, const math::Vector3& value) noexcept;
		bool set(std::string_view key, const math::Vector4& value) noexcept;
		bool set(std::string_view key, std::string_view value) noexcept;
		bool set(std::string_view key, const hal::GraphicsTexturePtr& value) noexcept;
		bool set(const MaterialParam& value) noexcept;

		bool get(std::string_view key, int& value) const noexcept;
		bool get(std::string_view key, float& value) const noexcept;
		bool get(std::string_view key, math::Vector2& value) const noexcept;
		bool get(std::string_view key, math::Vector3& value) const noexcept;
		bool get(std::string_view key, math::Vector4& value) const noexcept;
		bool get(std::string_view key, std::string& value) const noexcept;
		bool get(std::string_view key, hal::GraphicsTexturePtr& value) const noexcept;
		bool get(std::string_view key, MaterialParam& out) const noexcept;

		void setDirty(bool dirty) noexcept;
		bool isDirty() const noexcept;

		const std::vector<MaterialParam>& getMaterialParams() const noexcept;

		std::size_t hash() const noexcept;

		virtual std::shared_ptr<Material> clone() const noexcept;

	private:
		std::string name_;

		bool dirty_;

		bool _enableScissorTest;
		bool _enableSrgb;
		bool _enableMultisample;
		bool _enableRasterizerDiscard;
		bool _enableDepth;
		bool _enableDepthWrite;
		bool _enableDepthBounds;
		bool _enableDepthClamp;
		bool _enableDepthBias;
		bool _enableDepthBiasClamp;
		bool _enableStencil;

		float _lineWidth;

		hal::CullMode    _cullMode;
		hal::PolygonMode _polygonMode;
		hal::VertexType  _primitiveType;
		hal::FrontFace   _frontFace;

		float _depthMin;
		float _depthMax;
		float _depthBias;
		float _depthSlopeScaleBias;
		hal::CompareFunction _depthFunc;

		std::uint32_t _stencilFrontRef;
		std::uint32_t _stencilFrontReadMask;
		std::uint32_t _stencilFrontWriteMask;
		hal::StencilOp _stencilFrontFail;
		hal::StencilOp _stencilFrontZFail;
		hal::StencilOp _stencilFrontPass;
		hal::CompareFunction _stencilFrontFunc;

		std::uint32_t _stencilBackRef;
		std::uint32_t _stencilBackReadMask;
		std::uint32_t _stencilBackWriteMask;
		hal::StencilOp _stencilBackFail;
		hal::StencilOp _stencilBackZFail;
		hal::StencilOp _stencilBackPass;
		hal::CompareFunction _stencilBackFunc;

		hal::GraphicsColorBlends _blends;

		std::shared_ptr<Shader> _shader;

		std::vector<MaterialParam> _properties;
	};

	using MaterialPtr = std::shared_ptr<Material>;
	using Materials = std::vector<MaterialPtr>;
}

#endif