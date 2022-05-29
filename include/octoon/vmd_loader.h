#ifndef OCTOON_VMD_LOADER_H_
#define OCTOON_VMD_LOADER_H_

#include <octoon/io/iostream.h>
#include <octoon/animation/animation.h>

namespace octoon
{
	typedef char          VMD_char;
	typedef std::int8_t   VMD_int8_t;
	typedef std::uint8_t  VMD_uint8_t;
	typedef std::uint16_t VMD_uint16_t;
	typedef std::uint32_t VMD_uint32_t;

	typedef float VMD_Float;

#pragma pack(push)
#pragma pack(1)

	struct VMD_Vector2
	{
		float x;
		float y;
	};

	struct VMD_Vector3
	{
		float x;
		float y;
		float z;
	};

	struct VMD_Vector4
	{
		float x;
		float y;
		float z;
		float w;
	};

	struct VMD_Quaternion
	{
		float x;
		float y;
		float z;
		float w;
	};

	struct VMD_Header
	{
		VMD_char magic[30];
		VMD_char name[20];
	};

	struct VMDMotion
	{
		VMD_char name[15];
		VMD_uint32_t frame;
		VMD_Vector3 translate;
		VMD_Quaternion rotate;
		VMD_int8_t interpolation_x[4];
		VMD_int8_t interpolation_y[4];
		VMD_int8_t interpolation_z[4];
		VMD_int8_t interpolation_rotation[4];
		VMD_int8_t interpolation[48];
	};

	struct VMDMorph
	{
		VMD_char name[15];
		VMD_uint32_t frame;
		VMD_Float weight;
	};

	struct VMDCamera
	{
		VMD_uint32_t frame;
		VMD_Float length;
		VMD_Vector3 location;
		VMD_Vector3 rotation;
		VMD_int8_t interpolation_x[4];
		VMD_int8_t interpolation_y[4];
		VMD_int8_t interpolation_z[4];
		VMD_int8_t interpolation_rotation[4];
		VMD_int8_t interpolation_distance[4];
		VMD_int8_t interpolation_angleview[4];
		VMD_uint32_t viewingAngle;
		VMD_uint8_t perspective;
	};

	struct VMDLight
	{
		VMD_uint32_t frame;
		VMD_Vector3 rgb;
		VMD_Vector3 location;
	};

	struct VMDSelfShadow
	{
		VMD_uint32_t frame;
		VMD_uint8_t mode; // 00-02
		VMD_Float distance; // 0.1 - (dist * 0.000001)
	};

#pragma pack(pop)

	class OCTOON_EXPORT VMD
	{
	public:
		VMD_Header Header;

		VMD_uint32_t NumMotion;
		VMD_uint32_t NumMorph;
		VMD_uint32_t NumCamera;
		VMD_uint32_t NumLight;
		VMD_uint32_t NumSelfShadow;

		std::vector<VMDMotion> MotionLists;
		std::vector<VMDMorph> MorphLists;
		std::vector<VMDCamera> CameraLists;
		std::vector<VMDLight> LightLists;
		std::vector<VMDSelfShadow> SelfShadowLists;

		void load(io::istream& stream) noexcept(false);
	};

	class OCTOON_EXPORT VMDLoader final
	{
	public:
		VMDLoader() noexcept;
		~VMDLoader() noexcept;

		static bool doCanRead(io::istream& stream) noexcept;
		static bool doCanRead(const char* type) noexcept;

		static Animation<float> load(io::istream& stream) noexcept(false);

		static Animation<float> loadMotion(io::istream& stream) noexcept(false);
		static Animation<float> loadCameraMotion(io::istream& stream) noexcept(false);

		static void save(io::ostream& stream, const Animation<float>& animation) noexcept(false);

	private:
		VMDLoader(const VMDLoader&) = delete;
		VMDLoader& operator=(const VMDLoader&) = delete;
	};
}

#endif