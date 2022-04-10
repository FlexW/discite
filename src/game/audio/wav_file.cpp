#include "wav_file.hpp"
#include "defer.hpp"
#include "serialization.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace dc;

namespace
{

bool system_little_endian()
{
  int n = 1;
  // little endian if true
  if (*(char *)&n == 1)
  {
    return true;
  }
  return false;
}

std::int32_t convert_to_int(char *buffer, std::size_t len)
{
  std::int32_t a = 0;
  if (system_little_endian())
  {
    std::memcpy(&a, buffer, len);
  }
  else
  {
    for (std::size_t i = 0; i < len; ++i)
    {
      reinterpret_cast<char *>(&a)[3 - i] = buffer[i];
    }
  }
  return a;
}

bool load_wav_file_header(std::ifstream &file,
                          std::uint8_t  &channels,
                          std::int32_t  &sampleRate,
                          std::uint8_t  &bitsPerSample,
                          std::size_t   &size)
{
  char buffer[4];
  if (!file.is_open())
    return false;

  // the RIFF
  if (!file.read(buffer, 4))
  {
    return false;
  }
  if (std::strncmp(buffer, "RIFF", 4) != 0)
  {
    return false;
  }

  // the size of the file
  if (!file.read(buffer, 4))
  {
    return false;
  }

  // the WAVE
  if (!file.read(buffer, 4))
  {
    return false;
  }
  if (std::strncmp(buffer, "WAVE", 4) != 0)
  {
    return false;
  }

  // "fmt/0"
  if (!file.read(buffer, 4))
  {
    return false;
  }

  // this is always 16, the size of the fmt data chunk
  if (!file.read(buffer, 4))
  {
    return false;
  }

  // PCM should be 1?
  if (!file.read(buffer, 2))
  {
    return false;
  }

  // the number of channels
  if (!file.read(buffer, 2))
  {
    return false;
  }
  channels = convert_to_int(buffer, 2);

  // sample rate
  if (!file.read(buffer, 4))
  {
    return false;
  }
  sampleRate = convert_to_int(buffer, 4);

  // (sampleRate * bitsPerSample * channels) / 8
  if (!file.read(buffer, 4))
  {
    return false;
  }

  // ?? dafaq
  if (!file.read(buffer, 2))
  {
    return false;
  }

  // bitsPerSample
  if (!file.read(buffer, 2))
  {
    return false;
  }
  bitsPerSample = convert_to_int(buffer, 2);

  // data chunk header "data"
  if (!file.read(buffer, 4))
  {
    return false;
  }
  if (std::strncmp(buffer, "data", 4) != 0)
  {
    return false;
  }

  // size of data
  if (!file.read(buffer, 4))
  {
    return false;
  }
  size = convert_to_int(buffer, 4);

  /* cannot be at the end of file */
  if (file.eof())
  {
    return false;
  }
  if (file.fail())
  {
    return false;
  }

  return true;
}

std::unique_ptr<WavFile> load_wav(const std::filesystem::path &file_path,
                                  std::ifstream               &in)
{

  if (!in.is_open())
  {
    throw std::runtime_error("Could not open \"" + file_path.string() + "\"");
  }

  std::size_t  size;
  std::uint8_t channels;
  std::int32_t sample_rate;
  std::uint8_t bits_per_sample;
  if (!load_wav_file_header(in, channels, sample_rate, bits_per_sample, size))
  {
    std::runtime_error("Could not load wav header of \"" + file_path.string() +
                       "\"");
  }

  auto data =
      std::vector<unsigned char>(std::istreambuf_iterator<char>(in), {});

  return std::make_unique<WavFile>(file_path,
                                   channels,
                                   sample_rate,
                                   bits_per_sample,
                                   size,
                                   data);
}

} // namespace

namespace dc
{

void WavFile::save(const std::filesystem::path &file_path,
                   const AssetDescription      &asset_description) const
{
  DC_ASSERT(!original_wav_data_.empty(), "Can not save empty wave data");

  const auto file = std::fopen(file_path.string().c_str(), "wb");
  if (!file)
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }
  defer(std::fclose(file));

  asset_description.write(file);

  write_vector(file, original_wav_data_);
}

AssetDescription WavFile::read(const std::filesystem::path &file_path)
{
  std::ifstream in(file_path.string(), std::ios::binary);
  if (!in.is_open())
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }

  AssetDescription asset_description;
  asset_description.read(in);

  load_wav(file_path, in);

  return asset_description;
}

} // namespace dc
