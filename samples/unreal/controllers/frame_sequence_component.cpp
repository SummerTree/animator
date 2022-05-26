#include "frame_sequence_component.h"
#include "unreal_behaviour.h"

#include <fstream>
#include <octoon/image/image.h>

#include <QDir>
#include <QFileInfo>

namespace unreal
{
	FrameSequenceComponent::FrameSequenceComponent() noexcept {}

	void
	FrameSequenceComponent::onEnable() noexcept
	{
	}

	void
	FrameSequenceComponent::onDisable() noexcept
	{
		this->close();
	}

	bool
	FrameSequenceComponent::create(std::string_view filepath) noexcept(false)
	{
		auto& context = this->getContext();
		width_ = context->profile->recordModule->width;
		height_ = context->profile->recordModule->height;

		filepath_ = filepath;
		QFileInfo file(QString::fromStdString(filepath_));
		dirpath_ = file.absoluteDir().absolutePath().toStdString();
		filename_ = file.fileName().toStdString();
		basename_ = file.baseName().toStdString();
		extension_ = file.suffix().toStdString();
		count_ = 0;

		return true;
	}

	void
	FrameSequenceComponent::write(const octoon::math::Vector3* data) noexcept(false)
	{
		std::string outname = dirpath_ + "/" + basename_ + "_" + std::to_string(count_);
		if (extension_ == "png")
		{
			octoon::Image image;
			if (image.create(octoon::Format::R8G8B8SRGB, width_, height_))
			{
				std::uint8_t* img_ptr = const_cast<std::uint8_t*>(image.data());

				for (std::int32_t y = 0; y < height_; y++)
				{
					for (std::uint32_t x = 0; x < width_; x++)
					{
						auto src = y * width_ + x;
						auto dst = ((height_ - y - 1) * width_ + x) * 3;

						img_ptr[dst] = static_cast<std::uint8_t>(data[src].x * 255);
						img_ptr[dst + 1] = static_cast<std::uint8_t>(data[src].y * 255);
						img_ptr[dst + 2] = static_cast<std::uint8_t>(data[src].z * 255);
					}
				}

				image.save(outname + ".png", "png");
			}
		}

		count_ += 1;
	}

	void
	FrameSequenceComponent::close() noexcept
	{
	}
}