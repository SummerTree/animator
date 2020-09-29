#include <octoon/audio/common/ogg_audio_buffer.h>
#include <vorbis/vorbisfile.h>

namespace octoon
{
	std::size_t
	ogg_stream_read(void* ptr, std::size_t elementSize, std::size_t count, void* data)
	{
		assert(data != nullptr);

		auto input = static_cast<io::istream*>(data);
		input->read((char*)(ptr), elementSize * count);
		return input->gcount();
	}

	int
	ogg_stream_seek(void* data, ogg_int64_t pos, int whence)
	{
		assert(data != nullptr);

		auto input = static_cast<io::istream*>(data);
		switch (whence)
		{
		case SEEK_SET:
			input->seekg((int)pos, io::ios_base::beg);
			break;
		case SEEK_CUR:
			input->seekg((int)pos, io::ios_base::cur);
			break;
		case SEEK_END:
			input->seekg((int)pos, io::ios_base::end);
			break;
		}

		return 0;
	}

	int
	ogg_stream_close(void *data)
	{
		return 0;
	}

	long
	ogg_stream_tellg(void *data)
	{
		assert(data != nullptr);
		auto input = static_cast<io::istream*>(data);
		return static_cast<long>(input->tellg());
	}

	OggStreamBuffer::OggStreamBuffer() noexcept
		: _oggVorbisFile(nullptr)
		, _next(0)
	{
	}

	OggStreamBuffer::~OggStreamBuffer() noexcept
	{
		this->close();
	}

	bool
	OggStreamBuffer::access(io::istream& stream) const noexcept
	{
		OggVorbis_File oggfile;

		ov_callbacks callbacks;
		callbacks.read_func = &ogg_stream_read;
		callbacks.seek_func = &ogg_stream_seek;
		callbacks.tell_func = &ogg_stream_tellg;
		callbacks.close_func = &ogg_stream_close;

		auto err = ::ov_test_callbacks(&stream, &oggfile, nullptr, 0, callbacks);
		::ov_clear(&oggfile);

		return (err < 0) ? false : true;
	}

	bool
	OggStreamBuffer::open(std::shared_ptr<io::istream> stream) noexcept
	{
		assert(!_oggVorbisFile);

		OggVorbis_File ogg;

		ov_callbacks callbacks;
		callbacks.read_func = &ogg_stream_read;
		callbacks.seek_func = &ogg_stream_seek;
		callbacks.tell_func = &ogg_stream_tellg;
		callbacks.close_func = &ogg_stream_close;

		_next = 0;
		_stream = stream;

		auto err = ::ov_open_callbacks(_stream.get(), &ogg, nullptr, 0, callbacks);
		if (err < 0)
		{
			::ov_clear(&ogg);
			return false;
		}

		_oggVorbisFile = new OggVorbis_File;
		std::memcpy(_oggVorbisFile, &ogg, sizeof(OggVorbis_File));

		return true;
	}

	io::streamsize
	OggStreamBuffer::read(char* str, std::streamsize cnt) noexcept
	{
		assert(_oggVorbisFile);

		int bitstream = 0;
		io::streamsize bytes = 0;
		io::streamsize offset = 0;

		do
		{
			bytes = ::ov_read(_oggVorbisFile, str + offset, int(cnt - offset), 0, 2, 1, &bitstream);
			offset += bytes;
		} while (bytes && offset < cnt);

		if (bytes > 0)
		{
			auto info = ::ov_info(_oggVorbisFile, -1);
			if (info->channels == 6)
			{
				short* samples = (short*)str;
				for (io::streamsize i = 0; i < (bytes >> 1); i += 6)
				{
					std::swap(samples[i + 1], samples[i + 2]);
					std::swap(samples[i + 3], samples[i + 5]);
					std::swap(samples[i + 4], samples[i + 5]);
				}
			}
		}

		return offset;
	}

	io::streamsize
	OggStreamBuffer::write(const char* str, std::streamsize cnt) noexcept
	{
		assert(false);
		return 0;
	}

	io::streamoff
	OggStreamBuffer::seekg(io::ios_base::off_type pos, io::ios_base::seekdir dir) noexcept
	{
		assert(dir == io::ios_base::beg || dir == io::ios_base::cur || dir == io::ios_base::end);

		if (dir == io::ios_base::beg)
		{
			_next = pos;

			::ov_pcm_seek(_oggVorbisFile, pos);
			return pos;
		}
		else if (dir == io::ios_base::cur)
		{
			_next = _next + pos;
			if (_next > this->size())
			{
				pos = this->size() - _next;
				_next = this->size();
			}

			::ov_pcm_seek(_oggVorbisFile, pos);
			return pos;
		}
		else if (dir == io::ios_base::end)
		{
			std::size_t size = this->size();
			pos = size + pos;
			if (pos > size)
				_next = size;
			else
				_next = pos;

			::ov_pcm_seek(_oggVorbisFile, pos);
			return pos;
		}

		return 0;
	}

	io::streamoff
	OggStreamBuffer::tellg() noexcept
	{
		assert(this->is_open());
		return ::ov_pcm_tell(_oggVorbisFile);
	}

	io::streamsize
	OggStreamBuffer::size() const noexcept
	{
		assert(this->is_open());
		auto info = ::ov_info(_oggVorbisFile, -1);
		return ::ov_pcm_total(_oggVorbisFile, -1) * info->channels * 2;
	}

	bool
	OggStreamBuffer::is_open() const noexcept
	{
		return _oggVorbisFile ? true : false;
	}

	int
	OggStreamBuffer::flush() noexcept
	{
		if (_stream->flush())
			return 0;
		return -1;
	}

	bool
	OggStreamBuffer::close() noexcept
	{
		if (_oggVorbisFile)
		{
			::ov_clear(_oggVorbisFile);

			delete _oggVorbisFile;
			_oggVorbisFile = nullptr;

			return true;
		}

		return false;
	}

	std::uint32_t
	OggStreamBuffer::getBufferChannelCount() const noexcept
	{
		assert(_oggVorbisFile);
		auto info = ::ov_info(_oggVorbisFile, -1);
		return info->channels;
	}

	std::size_t
	OggStreamBuffer::getBufferTotalSamples() const noexcept
	{
		assert(_oggVorbisFile);
		return ::ov_pcm_total(_oggVorbisFile, -1);
	}

	AudioFormat
	OggStreamBuffer::getBufferType() const noexcept
	{
		assert(_oggVorbisFile);

		auto info = ::ov_info(_oggVorbisFile, -1);

		if (info->channels == 1)
			return AudioFormat::Mono16;
		else if (info->channels == 2)
			return AudioFormat::Stereo16;
		else if (info->channels == 4)
			return AudioFormat::Quad16;
		else if (info->channels == 6)
			return AudioFormat::Chn16;
		else
			return AudioFormat::None;
	}

	AudioFrequency
	OggStreamBuffer::getBufferFrequency() const noexcept
	{
		assert(_oggVorbisFile);
		auto info = ::ov_info(_oggVorbisFile, -1);
		return static_cast<AudioFrequency>(info->rate);
	}
}