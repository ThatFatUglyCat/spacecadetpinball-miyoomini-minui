// SPDX-License-Identifier: MIT
#include "libretro.h"

#include "content.h"
#include "errorscreen.h"
#include "input.h"
#include "log.h"

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

bool game_ready = false;
bool show_error = false;
std::string error_message;
std::vector<uint16_t> frame_rgb565;

void present_error_or_black() {
  if (!video_cb) return;
  if (frame_rgb565.size() != static_cast<std::size_t>(scp::kScreenW * scp::kScreenH))
    frame_rgb565.assign(static_cast<std::size_t>(scp::kScreenW * scp::kScreenH), 0);

  if (show_error)
    scp::render_error_screen(frame_rgb565.data(), scp::kScreenW, error_message.c_str());
  else
    std::memset(frame_rgb565.data(), 0, frame_rgb565.size() * sizeof(uint16_t));

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

  // Legacy SET_VARIABLES for minarch compatibility (placeholders until Phase 6).
  static const struct retro_variable vars[] = {
    {"scp_music", "Background Music; disabled|enabled"},
    {"scp_plunger_mode", "Plunger Mode; hold-release|tap"},
    {"scp_nudge", "Allow Nudge; enabled|disabled"},
    {"scp_show_fps", "Show FPS (debug); disabled|enabled"},
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
  info->library_version = "0.1.0";
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
  info->timing.fps = 60.0;
  info->timing.sample_rate = 44100.0;
}

void retro_init(void) {
  frame_rgb565.assign(static_cast<std::size_t>(scp::kScreenW * scp::kScreenH), 0);
  game_ready = false;
  show_error = false;
  error_message.clear();
}

void retro_deinit(void) {
  game_ready = false;
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

  game_ready = false;
  show_error = false;
  error_message.clear();

  const char* path = (game && game->path) ? game->path : nullptr;
  scp::ContentCheck check = scp::validate_dat(path);
  if (check.status != scp::ContentStatus::Ok) {
    show_error = true;
    error_message = check.message;
    scp::log(RETRO_LOG_WARN, "Content check failed (%s): %s\n",
             scp::status_label(check.status), check.message.c_str());
    // Stay loaded so minarch shows the help screen instead of a silent fail.
    return true;
  }

  // Phase 2 will call scp::game::init(path, ...). Until then, keep the error
  // screen stating that the game engine is not wired yet, while confirming DAT OK.
  show_error = true;
  error_message =
      "DAT header OK.\nGame engine not linked yet (Phase 2).\n"
      "Controls map is registered for MinUI.";
  scp::log(RETRO_LOG_INFO, "Validated DAT at %s\n", path ? path : "(null)");
  return true;
}

void retro_unload_game(void) {
  game_ready = false;
  show_error = false;
  error_message.clear();
}

void retro_run(void) {
  scp::poll_input(input_poll_cb, input_state_cb);
  present_error_or_black();
  // Phase 5: audio_batch_cb with sample_rate/fps silence or mixed SFX.
  if (audio_batch_cb) {
    int16_t silence[735 * 2];  // 44100/60
    std::memset(silence, 0, sizeof(silence));
    audio_batch_cb(silence, 735);
  }
}

void retro_reset(void) {
  // Phase 6: new game.
}

void retro_set_controller_port_device(unsigned /*port*/, unsigned /*device*/) {}

unsigned retro_get_region(void) { return RETRO_REGION_NTSC; }

bool retro_load_game_special(unsigned /*type*/, const struct retro_game_info* /*info*/,
                             size_t /*num*/) {
  return false;
}

size_t retro_serialize_size(void) { return 0; }
bool retro_serialize(void* /*data*/, size_t /*size*/) { return false; }
bool retro_unserialize(const void* /*data*/, size_t /*size*/) { return false; }

void* retro_get_memory_data(unsigned /*id*/) { return nullptr; }
size_t retro_get_memory_size(unsigned /*id*/) { return 0; }

void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned /*index*/, bool /*enabled*/, const char* /*code*/) {}

}  // extern "C"
