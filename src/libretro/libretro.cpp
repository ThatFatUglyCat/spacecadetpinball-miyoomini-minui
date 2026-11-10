// SPDX-License-Identifier: MIT
#include "libretro.h"

#include "audio.h"
#include "content.h"
#include "errorscreen.h"
#include "game.h"
#include "input.h"
#include "log.h"
#include "persist.h"
#include "video.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace {

retro_environment_t env_cb;
retro_video_refresh_t video_cb;
retro_audio_sample_t audio_cb;
retro_audio_sample_batch_t audio_batch_cb;
retro_input_poll_t input_poll_cb;
retro_input_state_t input_state_cb;

bool show_error = false;
std::string error_message;
std::vector<uint16_t> frame_rgb565;

const char* get_dir(unsigned cmd) {
  if (!env_cb) return nullptr;
  const char* dir = nullptr;
  if (env_cb(cmd, &dir) && dir && dir[0]) return dir;
  return nullptr;
}

bool read_bool_option(const char* key, const char* enabled_value) {
  if (!env_cb || !key) return false;
  struct retro_variable var = {key, nullptr};
  if (!env_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) || !var.value) return false;
  return std::strcmp(var.value, enabled_value) == 0;
}

scp::InputOptions read_input_options() {
  scp::InputOptions opts;
  opts.plunger_tap = read_bool_option("scp_plunger_mode", "tap");
  opts.nudge_enabled = !read_bool_option("scp_nudge", "disabled");
  return opts;
}

void present_frame() {
  if (!video_cb) return;
  if (frame_rgb565.size() != static_cast<std::size_t>(scp::kScreenW * scp::kScreenH))
    frame_rgb565.assign(static_cast<std::size_t>(scp::kScreenW * scp::kScreenH), 0);

  scp::video_set_show_fps(read_bool_option("scp_show_fps", "enabled"));

  if (show_error) {
    scp::render_error_screen(frame_rgb565.data(), scp::kScreenW, error_message.c_str());
  } else if (scp::game_ready()) {
    const scp::FramebufferView view = scp::game_framebuffer();
    if (!scp::present_framebuffer(view, frame_rgb565.data(), true)) {
      std::memset(frame_rgb565.data(), 0, frame_rgb565.size() * sizeof(uint16_t));
    } else {
      scp::video_on_frame_presented();
      scp::video_draw_fps_overlay(frame_rgb565.data());
      if (scp::game_high_scores_visible()) {
        const scp::HighScoreTableView hs = scp::game_high_score_table();
        scp::render_high_score_overlay(frame_rgb565.data(), scp::kScreenW, hs);
      }
    }
  } else {
    std::memset(frame_rgb565.data(), 0, frame_rgb565.size() * sizeof(uint16_t));
  }

  video_cb(frame_rgb565.data(), scp::kScreenW, scp::kScreenH, scp::kScreenW * sizeof(uint16_t));
}

}  // namespace

extern "C" {

void retro_set_environment(retro_environment_t cb) {
  env_cb = cb;
  bool no_game = false;
  if (env_cb) env_cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_game);

  struct retro_log_callback log;
  if (env_cb && env_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log) && log.log)
    scp::g_log_cb = log.log;

  scp::register_input_descriptors(env_cb);

  // Labels kept short for MinUI; values match docs/INPUT_MAPPING.md.
  static const struct retro_variable vars[] = {
    {"scp_music", "Background Music; disabled|enabled"},
    {"scp_plunger_mode", "Plunger Mode; hold-release|tap"},
    {"scp_nudge", "Allow Nudge; enabled|disabled"},
    {"scp_show_fps", "Show FPS overlay; disabled|enabled"},
    {nullptr, nullptr},
  };
  if (env_cb) env_cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars);
}

void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb = cb; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

unsigned retro_api_version(void) { return RETRO_API_VERSION; }

void retro_get_system_info(struct retro_system_info* info) {
  std::memset(info, 0, sizeof(*info));
  info->library_name = "SpaceCadetPinball";
  info->library_version = "0.7.0";
  info->need_fullpath = true;
  info->valid_extensions = "dat";
  info->block_extract = true;
}

void retro_get_system_av_info(struct retro_system_av_info* info) {
  std::memset(info, 0, sizeof(*info));
  info->geometry.base_width = scp::kScreenW;
  info->geometry.base_height = scp::kScreenH;
  info->geometry.max_width = scp::kScreenW;
  info->geometry.max_height = scp::kScreenH;
  info->geometry.aspect_ratio = 4.0f / 3.0f;
  info->timing.fps = static_cast<double>(scp::kAudioFps);
  info->timing.sample_rate = static_cast<double>(scp::kAudioSampleRate);
}

void retro_init(void) {
  frame_rgb565.assign(static_cast<std::size_t>(scp::kScreenW * scp::kScreenH), 0);
  show_error = false;
  error_message.clear();
  scp::save_ram_clear();
  scp::log(RETRO_LOG_INFO, "SpaceCadetPinball libretro %s\n", "0.7.0");
}

void retro_deinit(void) {
  scp::game_export_save_ram();
  scp::game_shutdown();
  show_error = false;
  error_message.clear();
  frame_rgb565.clear();
}

bool retro_load_game(const struct retro_game_info* game) {
  enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
  if (env_cb && !env_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
    scp::log(RETRO_LOG_ERROR, "Frontend rejected RGB565.\n");
    return false;
  }

  scp::game_shutdown();
  show_error = false;
  error_message.clear();

  const char* path = (game && game->path) ? game->path : nullptr;
  scp::ContentCheck check = scp::validate_dat(path);
  if (check.status != scp::ContentStatus::Ok) {
    show_error = true;
    error_message = check.message;
    scp::log(RETRO_LOG_WARN, "Content check failed (%s): %s\n",
             scp::status_label(check.status), check.message.c_str());
    return true;
  }

  scp::GameConfig cfg;
  cfg.dat_path = path;
  cfg.system_dir = get_dir(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY);
  cfg.save_dir = get_dir(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY);
  cfg.music_enabled = read_bool_option("scp_music", "enabled");

  if (!scp::game_init(cfg)) {
    show_error = true;
    error_message =
        "Engine init failed.\nCheck that PINBALL.DAT and its WAV files\n"
        "are readable, then see the core log.";
    scp::log(RETRO_LOG_ERROR, "game_init failed for %s\n", path ? path : "(null)");
    return true;
  }

  show_error = false;
  scp::log(RETRO_LOG_INFO, "Loaded %s\n", path);
  return true;
}

void retro_unload_game(void) {
  scp::game_export_save_ram();
  scp::game_shutdown();
  show_error = false;
  error_message.clear();
}

void retro_run(void) {
  // Frontend typically fills SAVE_RAM after load_game; import once on first run.
  if (scp::game_ready() && !show_error) {
    scp::game_import_save_ram();
    scp::game_set_music_enabled(read_bool_option("scp_music", "enabled"));
  }

  scp::poll_input(input_poll_cb, input_state_cb, read_input_options());
  if (scp::game_ready() && !show_error)
    scp::game_step(1000.0f / static_cast<float>(scp::kAudioFps));
  present_frame();
  if (scp::game_ready() && !show_error)
    scp::game_export_save_ram();
  if (audio_batch_cb) {
    int16_t pcm[scp::kAudioMaxFramesPerVideoFrame * 2];
    const std::size_t frames =
        scp::audio_mix_frame(pcm, scp::kAudioMaxFramesPerVideoFrame);
    if (frames > 0)
      audio_batch_cb(pcm, frames);
  }
}

void retro_reset(void) {
  if (scp::game_ready()) {
    scp::InputEvent ev{scp::InputEventKind::NewGame};
    scp::game_handle_input(&ev, 1);
  }
}

void retro_set_controller_port_device(unsigned, unsigned) {}
unsigned retro_get_region(void) { return RETRO_REGION_NTSC; }
bool retro_load_game_special(unsigned, const struct retro_game_info*, size_t) { return false; }
size_t retro_serialize_size(void) { return 0; }
bool retro_serialize(void*, size_t) { return false; }
bool retro_unserialize(const void*, size_t) { return false; }
void* retro_get_memory_data(unsigned id) {
  if (id == RETRO_MEMORY_SAVE_RAM) return scp::save_ram_data();
  return nullptr;
}

size_t retro_get_memory_size(unsigned id) {
  if (id == RETRO_MEMORY_SAVE_RAM) return scp::save_ram_size();
  return 0;
}
void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned, bool, const char*) {}

}  // extern "C"
