#pragma once
#include <core/core.hpp>
#include <audio/data/audio_segment.hpp>

namespace legion::audio
{
    struct mp3_audio_loader : public fs::resource_converter<audio_segment, audio_import_settings>
    {
        virtual common::result_decay_more<audio_segment, fs_error> load(const fs::basic_resource& resource, audio_import_settings&& settings) override;
        static ALCcontext* context;
    };
}