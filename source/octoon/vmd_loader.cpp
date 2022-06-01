#include <octoon/vmd_loader.h>
#include <octoon/math/vector2.h>
#include <octoon/math/vector3.h>
#include <octoon/math/vector4.h>
#include <octoon/math/quat.h>
#include <octoon/runtime/except.h>
#include <octoon/animation/path_interpolator.h>
#include <octoon/io/fstream.h>
#include <iconv.h>
#include <map>

namespace octoon
{
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

			return std::string(outbuf.get());
		}
		catch (const std::exception&)
		{
			iconv_close(ic);
		}

		return std::string();
	}

	std::string utf82sjis(const std::string& utf8)
	{
		std::size_t in_size = utf8.size();
		std::size_t out_size = utf8.size() * 2;

		auto inbuf = std::make_unique<char[]>(in_size);
		auto outbuf = std::make_unique<char[]>(out_size);
		char* in = inbuf.get();
		char* out = outbuf.get();

		std::memcpy(in, utf8.c_str(), in_size);

		iconv_t ic = nullptr;

		try
		{
			ic = iconv_open("SJIS", "utf-8");
			iconv(ic, &in, &in_size, &out, &out_size);
			iconv_close(ic);

			return std::string(outbuf.get());
		}
		catch (const std::exception&)
		{
			iconv_close(ic);
		}

		return std::string();
	}

	void
	VMD::load(io::istream& stream) noexcept(false)
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

	void
	VMD::save(io::ostream& stream) noexcept(false)
	{
		if (!stream.write((char*)&this->Header, sizeof(this->Header))) {
			throw runtime::runtime_error::create(R"(Cannot write property "Header" from stream)");
		}

		if (!stream.write((char*)&this->NumMotion, sizeof(this->NumMotion))) {
			throw runtime::runtime_error::create(R"(Cannot write property "NumMotion" from stream)");
		}

		if (this->NumMotion > 0 && this->MotionLists.size() > 0)
		{
			if (!stream.write((char*)this->MotionLists.data(), sizeof(VMDMotion) * this->NumMotion)) {
				throw runtime::runtime_error::create(R"(Cannot write property "VMDMotion" from stream)");
			}
		}

		if (!stream.write((char*)&this->NumMorph, sizeof(this->NumMorph))) {
			throw runtime::runtime_error::create(R"(Cannot write property "NumMorph" from stream)");
		}

		if (this->NumMorph > 0 && this->MorphLists.size() > 0)
		{
			if (!stream.write((char*)this->MorphLists.data(), sizeof(VMDMorph) * this->NumMorph)) {
				throw runtime::runtime_error::create(R"(Cannot write property "VMDMorph" from stream)");
			}
		}

		if (!stream.write((char*)&this->NumCamera, sizeof(this->NumCamera))) {
			throw runtime::runtime_error::create(R"(Cannot write property "NumCamera" from stream)");
		}

		if (this->NumCamera > 0 && this->CameraLists.size() > 0)
		{
			if (!stream.write((char*)this->CameraLists.data(), sizeof(VMDCamera) * this->NumCamera)) {
				throw runtime::runtime_error::create(R"(Cannot write property "VMDCamera" from stream)");
			}
		}

		if (!stream.write((char*)&this->NumLight, sizeof(this->NumLight))) {
			throw runtime::runtime_error::create(R"(Cannot write property "NumLight" from stream)");
		}

		if (this->NumLight > 0 && this->LightLists.size() > 0)
		{
			if (!stream.write((char*)this->LightLists.data(), sizeof(VMDLight) * this->NumLight)) {
				throw runtime::runtime_error::create(R"(Cannot write property "VMDLight" from stream)");
			}
		}

		if (!stream.write((char*)&this->NumSelfShadow, sizeof(this->NumSelfShadow))) {
			throw runtime::runtime_error::create(R"(Cannot write property "NumSelfShadow" from stream)");
		}

		if (this->NumSelfShadow > 0 && this->SelfShadowLists.size() > 0)
		{
			if (!stream.write((char*)this->SelfShadowLists.data(), sizeof(VMDSelfShadow) * this->NumSelfShadow)) {
				throw runtime::runtime_error::create(R"(Cannot write property "VMDSelfShadow" from stream)");
			}
		}
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
			clip.getCurve("Position.X").insert(Keyframe<float>((float)it.frame, it.translate.x));
			clip.getCurve("Position.Y").insert(Keyframe<float>((float)it.frame, it.translate.y));
			clip.getCurve("Position.Z").insert(Keyframe<float>((float)it.frame, it.translate.z));
			clip.getCurve("Rotation.X").insert(Keyframe<float>((float)it.frame, it.rotate.x));
			clip.getCurve("Rotation.Y").insert(Keyframe<float>((float)it.frame, it.rotate.y));
			clip.getCurve("Rotation.Z").insert(Keyframe<float>((float)it.frame, it.rotate.z));
			clip.getCurve("Rotation.W").insert(Keyframe<float>((float)it.frame, it.rotate.w));
		}

		Animation animation;
		animation.setName(sjis2utf8(vmd.Header.name));
		for (auto& it : motions)
			animation.addClip(it.second);

		return animation;
	}

	std::shared_ptr<Animation<float>>
	VMDLoader::loadMotion(io::istream& stream) noexcept(false)
	{
		VMD vmd;
		vmd.load(stream);

		Animation animation;
		animation.setName(sjis2utf8(vmd.Header.name));

		std::unordered_map<std::string, std::vector<VMDMotion>> motionList;
		for (auto& motion : vmd.MotionLists)
			motionList[motion.name].push_back(motion);

		octoon::AnimationClips<float> clips;
		clips.resize(motionList.size());

		std::size_t i = 0;

		for (auto it = motionList.begin(); it != motionList.end(); ++it, ++i)
		{
			octoon::Keyframes<float> translateX;
			octoon::Keyframes<float> translateY;
			octoon::Keyframes<float> translateZ;
			octoon::Keyframes<float> rotation;

			auto& motionData = (*it).second;

			translateX.reserve(motionData.size());
			translateY.reserve(motionData.size());
			translateZ.reserve(motionData.size());
			rotation.reserve(motionData.size());
			rotation.reserve(motionData.size());
			rotation.reserve(motionData.size());

			for (auto& data : motionData)
			{
				auto interpolationX = std::make_shared<octoon::PathInterpolator<float>>(data.interpolation_x[0] / 127.0f, data.interpolation_x[2] / 127.0f, data.interpolation_x[1] / 127.0f, data.interpolation_x[3] / 127.0f);
				auto interpolationY = std::make_shared<octoon::PathInterpolator<float>>(data.interpolation_y[0] / 127.0f, data.interpolation_y[2] / 127.0f, data.interpolation_y[1] / 127.0f, data.interpolation_y[3] / 127.0f);
				auto interpolationZ = std::make_shared<octoon::PathInterpolator<float>>(data.interpolation_z[0] / 127.0f, data.interpolation_z[2] / 127.0f, data.interpolation_z[1] / 127.0f, data.interpolation_z[3] / 127.0f);
				auto interpolationRotation = std::make_shared<octoon::PathInterpolator<float>>(data.interpolation_rotation[0] / 127.0f, data.interpolation_rotation[2] / 127.0f, data.interpolation_rotation[1] / 127.0f, data.interpolation_rotation[3] / 127.0f);

				translateX.emplace_back((float)data.frame / 30.0f, data.translate.x, interpolationX);
				translateY.emplace_back((float)data.frame / 30.0f, data.translate.y, interpolationY);
				translateZ.emplace_back((float)data.frame / 30.0f, data.translate.z, interpolationZ);
				rotation.emplace_back((float)data.frame / 30.0f, octoon::math::Quaternion(data.rotate.x, data.rotate.y, data.rotate.z, data.rotate.w), interpolationRotation);
			}

			auto& clip = clips[i];
			clip.setName(sjis2utf8((*it).first));
			clip.setCurve("LocalPosition.x", octoon::AnimationCurve(std::move(translateX)));
			clip.setCurve("LocalPosition.y", octoon::AnimationCurve(std::move(translateY)));
			clip.setCurve("LocalPosition.z", octoon::AnimationCurve(std::move(translateZ)));
			clip.setCurve("LocalRotation", octoon::AnimationCurve(std::move(rotation)));
		}

		return std::make_shared<Animation<float>>(std::move(clips));
	}

	std::shared_ptr<Animation<float>>
	VMDLoader::loadCameraMotion(io::istream& stream) noexcept(false)
	{
		VMD vmd;
		vmd.load(stream);

		auto animation = std::make_shared<Animation<float>>();
		animation->setName(sjis2utf8(vmd.Header.name));

		if (vmd.NumCamera > 0)
		{
			Keyframes<float> distance;
			Keyframes<float> eyeX;
			Keyframes<float> eyeY;
			Keyframes<float> eyeZ;
			Keyframes<float> rotation;
			Keyframes<float> fov;

			distance.reserve(vmd.CameraLists.size());
			eyeX.reserve(vmd.CameraLists.size());
			eyeY.reserve(vmd.CameraLists.size());
			eyeZ.reserve(vmd.CameraLists.size());
			rotation.reserve(vmd.CameraLists.size());
			fov.reserve(vmd.CameraLists.size());

			for (auto& it : vmd.CameraLists)
			{
				auto interpolationDistance = std::make_shared<PathInterpolator<float>>(it.interpolation_distance[0] / 127.0f, it.interpolation_distance[2] / 127.0f, it.interpolation_distance[1] / 127.0f, it.interpolation_distance[3] / 127.0f);
				auto interpolationX = std::make_shared<PathInterpolator<float>>(it.interpolation_x[0] / 127.0f, it.interpolation_x[2] / 127.0f, it.interpolation_x[1] / 127.0f, it.interpolation_x[3] / 127.0f);
				auto interpolationY = std::make_shared<PathInterpolator<float>>(it.interpolation_y[0] / 127.0f, it.interpolation_y[2] / 127.0f, it.interpolation_y[1] / 127.0f, it.interpolation_y[3] / 127.0f);
				auto interpolationZ = std::make_shared<PathInterpolator<float>>(it.interpolation_z[0] / 127.0f, it.interpolation_z[2] / 127.0f, it.interpolation_z[1] / 127.0f, it.interpolation_z[3] / 127.0f);
				auto interpolationRotation = std::make_shared<PathInterpolator<float>>(it.interpolation_rotation[0] / 127.0f, it.interpolation_rotation[2] / 127.0f, it.interpolation_rotation[1] / 127.0f, it.interpolation_rotation[3] / 127.0f);
				auto interpolationAngleView = std::make_shared<PathInterpolator<float>>(it.interpolation_angleview[0] / 127.0f, it.interpolation_angleview[2] / 127.0f, it.interpolation_angleview[1] / 127.0f, it.interpolation_angleview[3] / 127.0f);

				distance.emplace_back((float)it.frame / 30.0f, it.distance, interpolationDistance);
				eyeX.emplace_back((float)it.frame / 30.0f, it.location.x, interpolationX);
				eyeY.emplace_back((float)it.frame / 30.0f, it.location.y, interpolationY);
				eyeZ.emplace_back((float)it.frame / 30.0f, it.location.z, interpolationZ);
				rotation.emplace_back((float)it.frame / 30.0f, -math::float3(it.rotation.x, it.rotation.y, it.rotation.z), interpolationRotation);
				fov.emplace_back((float)it.frame / 30.0f, (float)it.viewingAngle, interpolationAngleView);
			}

			AnimationClip clip;
			clip.setCurve("LocalPosition.x", AnimationCurve(std::move(eyeX)));
			clip.setCurve("LocalPosition.y", AnimationCurve(std::move(eyeY)));
			clip.setCurve("LocalPosition.z", AnimationCurve(std::move(eyeZ)));
			clip.setCurve("LocalEulerAnglesRaw", AnimationCurve(std::move(rotation)));
			clip.setCurve("LocalForward", AnimationCurve(std::move(distance)));
			clip.setCurve("Camera:fov", AnimationCurve(std::move(fov)));

			animation->addClip(std::move(clip));
		}

		return animation;
	}

	std::shared_ptr<Animation<float>>
	VMDLoader::loadCameraMotion(std::string_view filepath) noexcept(false)
	{
		io::ifstream stream(std::string(filepath), std::ios_base::binary);
		if (stream)
			return loadCameraMotion(stream);

		return nullptr;
	}

	void
	VMDLoader::saveCameraMotion(io::ostream& stream, const Animation<float>& animation) noexcept(false)
	{
		auto sjis = utf82sjis(animation.name);

		VMD vmd;
		std::memset(&vmd.Header, 0, sizeof(vmd.Header));
		std::memcpy(&vmd.Header.magic, "Vocaloid Motion Data 0002", 15);
		std::memcpy(&vmd.Header.name, sjis.data(), sjis.size());

		vmd.NumLight = 0;
		vmd.NumCamera = 0;
		vmd.NumMorph = 0;
		vmd.NumMotion = 0;
		vmd.NumSelfShadow = 0;

		if (!animation.clips.empty())
		{
			auto& clip = animation.clips.front();

			auto& eyeX = clip.getCurve("LocalPosition.x");
			auto& eyeY = clip.getCurve("LocalPosition.y");
			auto& eyeZ = clip.getCurve("LocalPosition.z");
			auto& rotation = clip.getCurve("LocalEulerAnglesRaw");
			auto& distance = clip.getCurve("LocalForward");
			auto& fov = clip.getCurve("Camera:fov");

			for (std::size_t i = 0; i < eyeX.frames.size(); i++)
			{
				auto interpolatorEyeX = std::dynamic_pointer_cast<PathInterpolator<float>>(eyeX.frames[i].interpolator);
				auto interpolatorEyeY = std::dynamic_pointer_cast<PathInterpolator<float>>(eyeY.frames[i].interpolator);
				auto interpolatorEyeZ = std::dynamic_pointer_cast<PathInterpolator<float>>(eyeZ.frames[i].interpolator);
				auto interpolatorRotation = std::dynamic_pointer_cast<PathInterpolator<float>>(rotation.frames[i].interpolator);
				auto interpolatorDistance = std::dynamic_pointer_cast<PathInterpolator<float>>(distance.frames[i].interpolator);
				auto interpolatorViewingAngle = std::dynamic_pointer_cast<PathInterpolator<float>>(fov.frames[i].interpolator);

				VMDCamera motion;
				motion.frame = eyeX.frames[i].time * 30.0f;
				motion.location.x = eyeX.frames[i].value.getFloat();
				motion.location.y = eyeY.frames[i].value.getFloat();
				motion.location.z = eyeZ.frames[i].value.getFloat();
				motion.distance = distance.frames[i].value.getFloat();
				motion.rotation = -rotation.frames[i].value.getFloat3();
				motion.viewingAngle = fov.frames[i].value.getFloat();
				motion.perspective = 1;

				motion.interpolation_x[0] = static_cast<std::int8_t>(interpolatorEyeX->xa * 127);
				motion.interpolation_x[2] = static_cast<std::int8_t>(interpolatorEyeX->xb * 127);
				motion.interpolation_x[1] = static_cast<std::int8_t>(interpolatorEyeX->ya * 127);
				motion.interpolation_x[3] = static_cast<std::int8_t>(interpolatorEyeX->yb * 127);

				motion.interpolation_y[0] = static_cast<std::int8_t>(interpolatorEyeY->xa * 127);
				motion.interpolation_y[2] = static_cast<std::int8_t>(interpolatorEyeY->xb * 127);
				motion.interpolation_y[1] = static_cast<std::int8_t>(interpolatorEyeY->ya * 127);
				motion.interpolation_y[3] = static_cast<std::int8_t>(interpolatorEyeY->yb * 127);

				motion.interpolation_z[0] = static_cast<std::int8_t>(interpolatorEyeZ->xa * 127);
				motion.interpolation_z[2] = static_cast<std::int8_t>(interpolatorEyeZ->xb * 127);
				motion.interpolation_z[1] = static_cast<std::int8_t>(interpolatorEyeZ->ya * 127);
				motion.interpolation_z[3] = static_cast<std::int8_t>(interpolatorEyeZ->yb * 127);

				motion.interpolation_rotation[0] = static_cast<std::int8_t>(interpolatorRotation->xa * 127);
				motion.interpolation_rotation[2] = static_cast<std::int8_t>(interpolatorRotation->xb * 127);
				motion.interpolation_rotation[1] = static_cast<std::int8_t>(interpolatorRotation->ya * 127);
				motion.interpolation_rotation[3] = static_cast<std::int8_t>(interpolatorRotation->yb * 127);

				motion.interpolation_distance[0] = static_cast<std::int8_t>(interpolatorDistance->xa * 127);
				motion.interpolation_distance[2] = static_cast<std::int8_t>(interpolatorDistance->xb * 127);
				motion.interpolation_distance[1] = static_cast<std::int8_t>(interpolatorDistance->ya * 127);
				motion.interpolation_distance[3] = static_cast<std::int8_t>(interpolatorDistance->yb * 127);

				motion.interpolation_angleview[0] = static_cast<std::int8_t>(interpolatorViewingAngle->xa * 127);
				motion.interpolation_angleview[2] = static_cast<std::int8_t>(interpolatorViewingAngle->xb * 127);
				motion.interpolation_angleview[1] = static_cast<std::int8_t>(interpolatorViewingAngle->ya * 127);
				motion.interpolation_angleview[3] = static_cast<std::int8_t>(interpolatorViewingAngle->yb * 127);

				vmd.CameraLists.push_back(motion);
			}

			vmd.NumCamera = vmd.CameraLists.size();
		}

		vmd.save(stream);
	}

	void
	VMDLoader::saveCameraMotion(std::string_view filepath, const Animation<float>& animation) noexcept(false)
	{
		io::ofstream stream(std::string(filepath), io::ios_base::in | io::ios_base::out);
		if (stream)
			saveCameraMotion(stream, animation);
	}
}