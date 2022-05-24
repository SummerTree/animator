#ifndef OCTOON_RENDER_STATE_H_
#define OCTOON_RENDER_STATE_H_

#include <octoon/hal/graphics_resource.h>

namespace octoon
{
	class OCTOON_EXPORT RenderStateDesc final
	{
	public:
		RenderStateDesc() noexcept;
		~RenderStateDesc() noexcept;

		void setCullMode(hal::CullMode mode) noexcept;
		void setPolygonMode(hal::PolygonMode mode) noexcept;
		void setPrimitiveType(hal::VertexType type) noexcept;
		void setFrontFace(hal::FrontFace face) noexcept;
		void setScissorTestEnable(bool enable) noexcept;
		void setLinear2sRGBEnable(bool enable) noexcept;
		void setMultisampleEnable(bool enable) noexcept;
		void setRasterizerDiscardEnable(bool enable) noexcept;
		void setLineWidth(float width) noexcept;
		void setColorWriteMask(hal::ColorWriteMaskFlags mask) noexcept;

		void setBlendEnable(bool enable) noexcept;
		void setBlendOp(hal::BlendOp blendop) noexcept;
		void setBlendSrc(hal::BlendMode factor) noexcept;
		void setBlendDest(hal::BlendMode factor) noexcept;
		void setBlendAlphaOp(hal::BlendOp blendop) noexcept;
		void setBlendAlphaSrc(hal::BlendMode factor) noexcept;
		void setBlendAlphaDest(hal::BlendMode factor) noexcept;

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
		hal::ColorWriteMaskFlags getColorWriteMask() const noexcept;

		bool getBlendEnable() const noexcept;
		hal::BlendOp getBlendOp() const noexcept;
		hal::BlendMode getBlendSrc() const noexcept;
		hal::BlendMode getBlendDest() const noexcept;
		hal::BlendOp getBlendAlphaOp() const noexcept;
		hal::BlendMode getBlendAlphaSrc() const noexcept;
		hal::BlendMode getBlendAlphaDest() const noexcept;

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

	private:
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
		bool _enableBlend;

		hal::BlendOp _blendOp;
		hal::BlendOp _blendAlphaOp;

		hal::BlendMode _blendSrc;
		hal::BlendMode _blendDest;
		hal::BlendMode _blendAlphaSrc;
		hal::BlendMode _blendAlphaDest;

		hal::ColorWriteMaskFlags _colorWriteMask;

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
	};

	class OCTOON_EXPORT RenderState : public GraphicsResource
	{
		OctoonDeclareSubInterface(RenderState, GraphicsResource)
	public:
		RenderState() noexcept = default;
		virtual ~RenderState() = default;

		virtual const RenderStateDesc& getStateDesc() const noexcept = 0;

	private:
		RenderState(const RenderState&) noexcept = delete;
		RenderState& operator=(const RenderState&) noexcept = delete;
	};
}

#endif