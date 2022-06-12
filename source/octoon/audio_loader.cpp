#include <octoon/audio_loader.h>
#include <octoon/audio/ogg_stream.h>
#include <octoon/audio/mp3_stream.h>
#include <octoon/audio/wav_stream.h>
#include <octoon/audio/flac_stream.h>
#include <octoon/io/vstream.h>

namespace octoon
{
	AudioLoader::AudioLoader() noexcept
	{
	}

	AudioLoader::~AudioLoader() noexcept
	{
	}

	bool
	AudioLoader::doCanRead(io::istream& stream) noexcept
	{
		return false;
	}

	bool
	AudioLoader::doCanRead(const char* type) noexcept
	{
		return false;
	}

	std::shared_ptr<AudioReader>
	AudioLoader::load(std::shared_ptr<io::istream> stream) noexcept(false)
	{
		return nullptr;
	}

	std::shared_ptr<AudioReader>
	AudioLoader::load(const std::filesystem::path& path) noexcept(false)
	{
		auto ext = path.extension().u8string();
		for (auto& it : ext)
			it = (char)std::tolower(it);

		if (ext == u8".ogg")
		{
			auto reader = std::make_shared<OggAudioReader>(path);
			if (reader->is_open())
				return reader;
		}
		else if (ext == u8".mp3")
		{
			auto reader = std::make_shared<Mp3AudioReader>(path);
			if (reader->is_open())
				return reader;
		}
		else if (ext == u8".wav")
		{
			auto reader = std::make_shared<WavAudioReader>(path);
			if (reader->is_open())
				return reader;
		}
		else if (ext == u8".flac")
		{
			auto reader = std::make_shared<FlacAudioReader>(path);
			if (reader->is_open())
				return reader;
		}

		return nullptr;
	}

	void
	AudioLoader::save(io::ostream& stream, const std::shared_ptr<AudioReader>& animation) noexcept(false)
	{
		
	}
}