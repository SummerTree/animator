#ifndef OCTOON_GL33_DESCRIPTOR_SET_H_
#define OCTOON_GL33_DESCRIPTOR_SET_H_

#include "gl33_types.h"

namespace octoon
{
	namespace hal
	{
		class GL33GraphicsUniformSet final : public GraphicsUniformSet
		{
			OctoonDeclareSubClass(GL33GraphicsUniformSet, GraphicsUniformSet)
		public:
			GL33GraphicsUniformSet() noexcept;
			virtual ~GL33GraphicsUniformSet() noexcept;

			const std::string& getName() const noexcept;

			void uniform1b(bool value) noexcept override;
			void uniform1i(std::int32_t i1) noexcept override;
			void uniform2i(const int2& value) noexcept override;
			void uniform2i(std::int32_t i1, std::int32_t i2) noexcept override;
			void uniform3i(const int3& value) noexcept override;
			void uniform3i(std::int32_t i1, std::int32_t i2, std::int32_t i3) noexcept override;
			void uniform4i(const int4& value) noexcept override;
			void uniform4i(std::int32_t i1, std::int32_t i2, std::int32_t i3, std::int32_t i4) noexcept override;
			void uniform1ui(std::uint32_t i1) noexcept override;
			void uniform2ui(const uint2& value) noexcept override;
			void uniform2ui(std::uint32_t i1, std::uint32_t i2) noexcept override;
			void uniform3ui(const uint3& value) noexcept override;
			void uniform3ui(std::uint32_t i1, std::uint32_t i2, std::uint32_t i3) noexcept override;
			void uniform4ui(const uint4& value) noexcept override;
			void uniform4ui(std::uint32_t i1, std::uint32_t i2, std::uint32_t i3, std::uint32_t i4) noexcept override;
			void uniform1f(float i1) noexcept override;
			void uniform2f(const float2& value) noexcept override;
			void uniform2f(float i1, float i2) noexcept override;
			void uniform3f(const float3& value) noexcept override;
			void uniform3f(float i1, float i2, float i3) noexcept override;
			void uniform4f(const float4& value) noexcept override;
			void uniform4f(float i1, float i2, float i3, float i4) noexcept override;
			void uniform2fmat(const float* mat2) noexcept override;
			void uniform2fmat(const float2x2& value) noexcept override;
			void uniform3fmat(const float* mat3) noexcept override;
			void uniform3fmat(const float3x3& value) noexcept override;
			void uniform4fmat(const float* mat4) noexcept override;
			void uniform4fmat(const float4x4& value) noexcept override;
			void uniform1iv(const std::vector<int1>& value) noexcept override;
			void uniform1iv(std::size_t num, const std::int32_t* str) noexcept override;
			void uniform2iv(const std::vector<int2>& value) noexcept override;
			void uniform2iv(std::size_t num, const std::int32_t* str) noexcept override;
			void uniform3iv(const std::vector<int3>& value) noexcept override;
			void uniform3iv(std::size_t num, const std::int32_t* str) noexcept override;
			void uniform4iv(const std::vector<int4>& value) noexcept override;
			void uniform4iv(std::size_t num, const std::int32_t* str) noexcept override;
			void uniform1uiv(const std::vector<uint1>& value) noexcept override;
			void uniform1uiv(std::size_t num, const std::uint32_t* str) noexcept override;
			void uniform2uiv(const std::vector<uint2>& value) noexcept override;
			void uniform2uiv(std::size_t num, const std::uint32_t* str) noexcept override;
			void uniform3uiv(const std::vector<uint3>& value) noexcept override;
			void uniform3uiv(std::size_t num, const std::uint32_t* str) noexcept override;
			void uniform4uiv(const std::vector<uint4>& value) noexcept override;
			void uniform4uiv(std::size_t num, const std::uint32_t* str) noexcept override;
			void uniform1fv(const std::vector<float1>& value) noexcept override;
			void uniform1fv(std::size_t num, const float* str) noexcept override;
			void uniform2fv(const std::vector<float2>& value) noexcept override;
			void uniform2fv(std::size_t num, const float* str) noexcept override;
			void uniform3fv(const std::vector<float3>& value) noexcept override;
			void uniform3fv(std::size_t num, const float* str) noexcept override;
			void uniform4fv(const std::vector<float4>& value) noexcept override;
			void uniform4fv(std::size_t num, const float* str) noexcept override;
			void uniform2fmatv(const std::vector<float2x2>& value) noexcept override;
			void uniform2fmatv(std::size_t num, const float* mat2) noexcept override;
			void uniform3fmatv(const std::vector<float3x3>& value) noexcept override;
			void uniform3fmatv(std::size_t num, const float* mat3) noexcept override;
			void uniform4fmatv(const std::vector<float4x4>& value) noexcept override;
			void uniform4fmatv(std::size_t num, const float* mat4) noexcept override;
			void uniformTexture(std::shared_ptr<GraphicsTexture> texture, GraphicsSamplerPtr sampler) noexcept override;
			void uniformBuffer(GraphicsDataPtr ubo) noexcept override;

			bool getBool() const noexcept override;
			int getInt() const noexcept override;
			const int2& getInt2() const noexcept override;
			const int3& getInt3() const noexcept override;
			const int4& getInt4() const noexcept override;
			uint1 getUInt() const noexcept override;
			const uint2& getUInt2() const noexcept override;
			const uint3& getUInt3() const noexcept override;
			const uint4& getUInt4() const noexcept override;
			float getFloat() const noexcept override;
			const float2& getFloat2() const noexcept override;
			const float3& getFloat3() const noexcept override;
			const float4& getFloat4() const noexcept override;
			const float2x2& getFloat2x2() const noexcept override;
			const float3x3& getFloat3x3() const noexcept override;
			const float4x4& getFloat4x4() const noexcept override;
			const std::vector<int1>& getIntArray() const noexcept override;
			const std::vector<int2>& getInt2Array() const noexcept override;
			const std::vector<int3>& getInt3Array() const noexcept override;
			const std::vector<int4>& getInt4Array() const noexcept override;
			const std::vector<uint1>& getUIntArray() const noexcept override;
			const std::vector<uint2>& getUInt2Array() const noexcept override;
			const std::vector<uint3>& getUInt3Array() const noexcept override;
			const std::vector<uint4>& getUInt4Array() const noexcept override;
			const std::vector<float1>& getFloatArray() const noexcept override;
			const std::vector<float2>& getFloat2Array() const noexcept override;
			const std::vector<float3>& getFloat3Array() const noexcept override;
			const std::vector<float4>& getFloat4Array() const noexcept override;
			const std::vector<float2x2>& getFloat2x2Array() const noexcept override;
			const std::vector<float3x3>& getFloat3x3Array() const noexcept override;
			const std::vector<float4x4>& getFloat4x4Array() const noexcept override;
			const std::shared_ptr<GraphicsTexture>& getTexture() const noexcept override;
			const GraphicsSamplerPtr& getTextureSampler() const noexcept override;
			const GraphicsDataPtr& getBuffer() const noexcept override;

			void setGraphicsParam(GraphicsParamPtr param) noexcept;
			const GraphicsParamPtr& getGraphicsParam() const noexcept;

		private:
			GL33GraphicsUniformSet(const GL33GraphicsUniformSet&) = delete;
			GL33GraphicsUniformSet& operator=(const GL33GraphicsUniformSet&) = delete;

		private:
			UniformHolder _variant;
			GraphicsParamPtr _param;
		};

		class GL33DescriptorPool final : public GraphicsDescriptorPool
		{
			OctoonDeclareSubClass(GL33DescriptorPool, GraphicsDescriptorPool)
		public:
			GL33DescriptorPool() noexcept;
			~GL33DescriptorPool() noexcept;

			bool setup(const GraphicsDescriptorPoolDesc& desc) noexcept;
			void close() noexcept;

			const GraphicsDescriptorPoolDesc& getDescriptorPoolDesc() const noexcept override;

		private:
			friend class GL33Device;
			void setDevice(const GraphicsDevicePtr& device) noexcept;
			GraphicsDevicePtr getDevice() const noexcept override;

		private:
			GL33DescriptorPool(const GL33DescriptorPool&) noexcept = delete;
			GL33DescriptorPool& operator=(const GL33DescriptorPool&) noexcept = delete;

		private:
			GraphicsDeviceWeakPtr _device;
			GraphicsDescriptorPoolDesc _descriptorPoolDesc;
		};

		class GL33DescriptorSetLayout final : public GraphicsDescriptorSetLayout
		{
			OctoonDeclareSubClass(GL33DescriptorSetLayout, GraphicsDescriptorSetLayout)
		public:
			GL33DescriptorSetLayout() noexcept;
			~GL33DescriptorSetLayout() noexcept;

			bool setup(const GraphicsDescriptorSetLayoutDesc& desc) noexcept;
			void close() noexcept;

			const GraphicsDescriptorSetLayoutDesc& getDescriptorSetLayoutDesc() const noexcept override;

		private:
			friend class GL33Device;
			void setDevice(const GraphicsDevicePtr& device) noexcept;
			GraphicsDevicePtr getDevice() const noexcept override;

		private:
			GL33DescriptorSetLayout(const GL33DescriptorSetLayout&) noexcept = delete;
			GL33DescriptorSetLayout& operator=(const GL33DescriptorSetLayout&) noexcept = delete;

		private:
			GraphicsDeviceWeakPtr _device;
			GraphicsDescriptorSetLayoutDesc _descripotrSetLayoutDesc;
		};

		class GL33DescriptorSet final : public GraphicsDescriptorSet
		{
			OctoonDeclareSubClass(GL33DescriptorSet, GraphicsDescriptorSet)
		public:
			GL33DescriptorSet() noexcept;
			~GL33DescriptorSet() noexcept;

			bool setup(const GraphicsDescriptorSetDesc& desc) noexcept;
			void close() noexcept;

			void apply(const GL33Program& program) noexcept;

			void copy(std::uint32_t descriptorCopyCount, const GraphicsDescriptorSetPtr descriptorCopies[]) noexcept;

			const GraphicsUniformSets& getUniformSets() const noexcept;
			const GraphicsDescriptorSetDesc& getDescriptorSetDesc() const noexcept override;

		private:
			friend class GL33Device;
			void setDevice(const GraphicsDevicePtr& device) noexcept;
			GraphicsDevicePtr getDevice() const noexcept override;

		private:
			GL33DescriptorSet(const GL33DescriptorSet&) noexcept = delete;
			GL33DescriptorSet& operator=(const GL33DescriptorSet&) noexcept = delete;

		private:
			GraphicsUniformSets _activeUniformSets;
			GraphicsDeviceWeakPtr _device;
			GraphicsDescriptorSetDesc _descriptorSetDesc;
		};
	}
}

#endif