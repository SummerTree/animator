#include <octoon/vmd_loader.h>
#include <octoon/math/vector2.h>
#include <octoon/math/vector3.h>
#include <octoon/math/vector4.h>
#include <octoon/math/quat.h>
#include <octoon/runtime/except.h>
#include <octoon/animation/path_interpolator.h>
#include <iconv.h>
#include <map>

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
		VMD_Vector3 location;
		VMD_Quaternion rotate;
		VMD_int8_t interpolation[64];
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

	struct VMD
	{
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

		void load(io::istream& stream) noexcept(false)
		{
			if (!stream.read((char*)&this->Header, sizeof(this->Header))) {
				throw runtime::runtime_error::create(R"(Cannot read property "Header" from stream)");
			}

			if (!stream.read((char*)&this->NumMotion, sizeof(this->NumMotion))) {
				throw runtime::runtime_error::create(R"(Cannot read property "NumMotion" from stream)");
			}

			if (this->NumMotion > 0)
			{
				this->MotionLists.resize(this->NumMotion);

				if (!stream.read((char*)this->MotionLists.data(), sizeof(VMDMotion) * this->NumMotion)) {
					throw runtime::runtime_error::create(R"(Cannot read property "VMDMotion" from stream)");
				}
			}

			if (!stream.read((char*)&this->NumMorph, sizeof(this->NumMorph))) {
				throw runtime::runtime_error::create(R"(Cannot read property "NumMorph" from stream)");
			}

			if (this->NumMorph > 0)
			{
				this->MorphLists.resize(this->NumMorph);

				if (!stream.read((char*)this->MorphLists.data(), sizeof(VMDMorph) * this->NumMorph)) {
					throw runtime::runtime_error::create(R"(Cannot read property "VMDMorph" from stream)");
				}
			}

			if (!stream.read((char*)&this->NumCamera, sizeof(this->NumCamera))) {
				throw runtime::runtime_error::create(R"(Cannot read property "NumCamera" from stream)");
			}

			if (this->NumCamera > 0)
			{
				this->CameraLists.resize(this->NumCamera);

				if (!stream.read((char*)this->CameraLists.data(), sizeof(VMDCamera) * this->NumCamera)) {
					throw runtime::runtime_error::create(R"(Cannot read property "VMDCamera" from stream)");
				}
			}

			if (!stream.read((char*)&this->NumLight, sizeof(this->NumLight))) {
				throw runtime::runtime_error::create(R"(Cannot read property "NumLight" from stream)");
			}

			if (this->NumLight > 0)
			{
				this->LightLists.resize(this->NumLight);

				if (!stream.read((char*)this->LightLists.data(), sizeof(VMDLight) * this->NumLight)) {
					throw runtime::runtime_error::create(R"(Cannot read property "VMDLight" from stream)");
				}
			}

			if (!stream.read((char*)&this->NumSelfShadow, sizeof(this->NumSelfShadow))) {
				throw runtime::runtime_error::create(R"(Cannot read property "NumSelfShadow" from stream)");
			}

			if (this->NumSelfShadow > 0)
			{
				this->SelfShadowLists.resize(this->NumSelfShadow);

				if (!stream.read((char*)this->SelfShadowLists.data(), sizeof(VMDSelfShadow) * this->NumSelfShadow)) {
					throw runtime::runtime_error::create(R"(Cannot read property "VMDSelfShadow" from stream)");
				}
			}
		}
	};

	std::string sjis2utf8(const std::string& sjis)
	{
		std::size_t in_size = sjis.size();
		std::size_t out_size = sjis.size() * 2;

		auto inbuf = std::make_unique<char[]>(in_size);
		auto outbuf = std::make_unique<char[]>(out_size);
		char* in = inbuf.get();
		char* out = outbuf.get();

		std::memcpy(in, sjis.c_str(), in_size);

		iconv_t ic = nullptr;

		try
		{
			ic = iconv_open("utf-8", "SJIS");
			iconv(ic, &in, &in_size, &out, &out_size);
			iconv_close(ic);
		}
		catch (const std::exception&)
		{
			iconv_close(ic);
		}

		return std::string(outbuf.get());
	}

	VMDLoader::VMDLoader() noexcept
	{
	}

	VMDLoader::~VMDLoader() noexcept
	{
	}

	bool
	VMDLoader::doCanRead(io::istream& stream) noexcept
	{
		static_assert(sizeof(VMDMotion) == 111, "");
		static_assert(sizeof(VMDMorph) == 23, "");
		static_assert(sizeof(VMDCamera) == 61, "");
		static_assert(sizeof(VMDLight) == 28, "");
		static_assert(sizeof(VMDSelfShadow) == 9, "");

		VMD_Header hdr;

		if (stream.read((char*)&hdr, sizeof(hdr)))
		{
			if (std::strncmp(hdr.magic, "Vocaloid Motion Data 0002", 30) == 0)
				return true;
		}

		return false;
	}

	bool
	VMDLoader::doCanRead(const char* type) noexcept
	{
		return std::strncmp(type, "vmd", 3) == 0;
	}

	Animation<float>
	VMDLoader::load(io::istream& stream) noexcept(false)
	{
		VMD vmd;
		vmd.load(stream);

		std::map<std::string, AnimationClip<float>> motions;

		for (auto& it : vmd.MotionLists)
		{
			if (motions.find(it.name) == motions.end())
				motions[it.name].setName(sjis2utf8(vmd.Header.name));

			auto& clip = motions[it.name];
			clip.getCurve("Position.X").insert(Keyframe<float, float>((float)it.frame, it.location.x));
			clip.getCurve("Position.Y").insert(Keyframe<float, float>((float)it.frame, it.location.y));
			clip.getCurve("Position.Z").insert(Keyframe<float, float>((float)it.frame, it.location.z));
			clip.getCurve("Rotation.X").insert(Keyframe<float, float>((float)it.frame, it.rotate.x));
			clip.getCurve("Rotation.Y").insert(Keyframe<float, float>((float)it.frame, it.rotate.y));
			clip.getCurve("Rotation.Z").insert(Keyframe<float, float>((float)it.frame, it.rotate.z));
			clip.getCurve("Rotation.W").insert(Keyframe<float, float>((float)it.frame, it.rotate.w));
		}

		Animation animation;
		animation.setName(sjis2utf8(vmd.Header.name));
		for (auto& it : motions)
			animation.addClip(it.second);

		return animation;
	}

	Animation<float>
	VMDLoader::loadCameraMotion(io::istream& stream) noexcept(false)
	{
		VMD vmd;
		vmd.load(stream);

		Animation animation;
		animation.setName(sjis2utf8(vmd.Header.name));

		if (vmd.NumCamera > 0)
		{
			Keyframes<float> distance;
			Keyframes<float> eyeX;
			Keyframes<float> eyeY;
			Keyframes<float> eyeZ;
			Keyframes<float> rotationX;
			Keyframes<float> rotationY;
			Keyframes<float> rotationZ;
			Keyframes<float> fov;

			distance.reserve(vmd.CameraLists.size());
			eyeX.reserve(vmd.CameraLists.size());
			eyeY.reserve(vmd.CameraLists.size());
			eyeZ.reserve(vmd.CameraLists.size());
			rotationX.reserve(vmd.CameraLists.size());
			rotationY.reserve(vmd.CameraLists.size());
			rotationZ.reserve(vmd.CameraLists.size());
			fov.reserve(vmd.CameraLists.size());

			for (auto& it : vmd.CameraLists)
			{
				auto interpolationDistance = std::make_shared<PathInterpolator<float>>(it.interpolation_distance[0] / 127.0f, it.interpolation_distance[2] / 127.0f, it.interpolation_distance[1] / 127.0f, it.interpolation_distance[3] / 127.0f);
				auto interpolationX = std::make_shared<PathInterpolator<float>>(it.interpolation_x[0] / 127.0f, it.interpolation_x[2] / 127.0f, it.interpolation_x[2] / 127.0f, it.interpolation_x[1] / 127.0f);
				auto interpolationY = std::make_shared<PathInterpolator<float>>(it.interpolation_y[0] / 127.0f, it.interpolation_y[2] / 127.0f, it.interpolation_y[2] / 127.0f, it.interpolation_y[1] / 127.0f);
				auto interpolationZ = std::make_shared<PathInterpolator<float>>(it.interpolation_z[0] / 127.0f, it.interpolation_z[2] / 127.0f, it.interpolation_z[2] / 127.0f, it.interpolation_z[1] / 127.0f);
				auto interpolationRotation = std::make_shared<PathInterpolator<float>>(it.interpolation_rotation[0] / 127.0f, it.interpolation_rotation[2] / 127.0f, it.interpolation_rotation[1] / 127.0f, it.interpolation_rotation[3] / 127.0f);
				auto interpolationAngleView = std::make_shared<PathInterpolator<float>>(it.interpolation_angleview[0] / 127.0f, it.interpolation_angleview[2] / 127.0f, it.interpolation_angleview[1] / 127.0f, it.interpolation_angleview[3] / 127.0f);

				distance.emplace_back((float)it.frame / 30.0f, it.length, interpolationDistance);
				eyeX.emplace_back((float)it.frame / 30.0f, it.location.x, interpolationX);
				eyeY.emplace_back((float)it.frame / 30.0f, it.location.y, interpolationY);
				eyeZ.emplace_back((float)it.frame / 30.0f, it.location.z, interpolationZ);
				rotationX.emplace_back((float)it.frame / 30.0f, -it.rotation.x, interpolationRotation);
				rotationY.emplace_back((float)it.frame / 30.0f, -it.rotation.y, interpolationRotation);
				rotationZ.emplace_back((float)it.frame / 30.0f, -it.rotation.z, interpolationRotation);
				fov.emplace_back((float)it.frame / 30.0f, (float)it.viewingAngle, interpolationAngleView);
			}

			AnimationClip clip;
			clip.setCurve("LocalPosition.x", AnimationCurve(std::move(eyeX)));
			clip.setCurve("LocalPosition.y", AnimationCurve(std::move(eyeY)));
			clip.setCurve("LocalPosition.z", AnimationCurve(std::move(eyeZ)));
			clip.setCurve("LocalEulerAnglesRaw.x", AnimationCurve(std::move(rotationX)));
			clip.setCurve("LocalEulerAnglesRaw.y", AnimationCurve(std::move(rotationY)));
			clip.setCurve("LocalEulerAnglesRaw.z", AnimationCurve(std::move(rotationZ)));
			clip.setCurve("Transform:move", AnimationCurve(std::move(distance)));
			clip.setCurve("Camera:fov", AnimationCurve(std::move(fov)));

			animation.addClip(std::move(clip));
		}

		return animation;
	}

	void
	VMDLoader::save(io::ostream& stream, const Animation<float>& animation) noexcept(false)
	{
		VMD vmd;
		std::memset(&vmd.Header, 0, sizeof(vmd.Header));
		std::memcpy(vmd.Header.magic, "Vocaloid Motion Data 0002", 15);

		stream.write((char*)&vmd.Header, sizeof(vmd.Header));
	}
}