#include "audio_importer_layer.hpp"
#include "asset_description.hpp"
#include "audio/wav_file.hpp"
#include "cmd_args_parser.hpp"
#include "engine.hpp"
#include "filesystem.hpp"
#include "log.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <stdexcept>

namespace dc
{

void AudioImporterLayer::add_cmd_line_args(ArgsParser &args_parser)
{
  ArgsParser::Option file_path_option;
  file_path_option.name_        = "file-path";
  file_path_option.description_ = "File path of audio to import (only WAVE)";
  file_path_option.type_        = ArgsParser::OptionType::Value;
  file_path_option.importance_  = ArgsParser::OptionImportance::Required;
  args_parser.add_option(file_path_option);

  ArgsParser::Option name_option;
  name_option.name_        = "name";
  name_option.description_ = "Name of the imported mesh";
  name_option.type_        = ArgsParser::OptionType::Value;
  name_option.importance_  = ArgsParser::OptionImportance::Required;
  args_parser.add_option(name_option);
}

void AudioImporterLayer::eval_cmd_line_args(ArgsParser &args_parser)
{
  file_path_  = args_parser.value_as_string("file-path").value();
  audio_name_ = args_parser.value_as_string("name").value();
}

void AudioImporterLayer::init()
{
  try
  {
    const auto data = read_binary_file(file_path_);

    WavFile    wav_file{};
    wav_file.original_wav_data_.resize(data.size() * sizeof(unsigned char));
    std::memcpy(wav_file.original_wav_data_.data(),
                data.data(),
                data.size() * sizeof(unsigned char));

    const auto asset_directory = Engine::instance()->base_directory() / "audio";
    std::filesystem::create_directories(asset_directory);
    AssetDescription asset_description{};
    wav_file.save(asset_directory / (audio_name_ + ".dcaud"), asset_description);
  }
  catch (const std::runtime_error &error)
  {
    DC_LOG_ERROR("Error while importing audio {}", error.what());
  }
}

void AudioImporterLayer::shutdown() {}

bool AudioImporterLayer::update(float /*delta_time*/)
{
  Engine::instance()->set_close(true);

  return false;
}

bool AudioImporterLayer::render() { return false; }

bool AudioImporterLayer::on_event(const Event & /*event*/) { return false; }

} // namespace dc
