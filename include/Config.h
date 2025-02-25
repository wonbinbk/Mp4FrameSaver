#ifndef CONFIG_H
#define CONFIG_H

namespace Config {
static constexpr const char* RESIZER_IN_QUEUE = "/RESIZER_IN_QUEUE";
static constexpr const char* SAVER_IN_QUEUE = "/SAVER_IN_QUEUE";
static constexpr const char* PUBLISHER_IN_QUEUE = "/PUBLISHER_IN_QUEUE";

static constexpr const char* SHM_FRAME = "/SHM_ORIGINAL_FRAME";
static constexpr const char* SHM_RESIZED_FRAME = "/SHM_RESIZED_FRAME";

static constexpr const char* DEFAULT_OUTPUT_DIR = "/home/pokyuser/Mp4ResizedFrames";
}

#endif // CONFIG_H
