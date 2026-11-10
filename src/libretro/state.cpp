// SPDX-License-Identifier: MIT
#include "state.h"

#include "game.h"
#include "log.h"
#include "persist.h"

#include "pch.h"
#include "TBall.h"
#include "TCollisionComponent.h"
#include "TFlipper.h"
#include "TFlipperEdge.h"
#include "TLight.h"
#include "TPinballTable.h"
#include "TPlunger.h"
#include "nudge.h"
#include "pb.h"
#include "timer.h"
#include "winmain.h"

#include <cstring>
#include <vector>

namespace scp {
namespace {

constexpr int kMaxBalls = 20;
constexpr int kMaxFlippers = 4;
constexpr int kMaxComponents = 512;
constexpr int kMaxLights = 256;

#pragma pack(push, 1)
struct BallSnap {
  int8_t active;
  int8_t collision_flag;
  int8_t unused0[2];
  float pos_x, pos_y, pos_z;
  float dir_x, dir_y, dir_z;
  float speed;
  int32_t collision_group_index;  // -1 if none
  int32_t stuck_counter;
};

struct FlipperSnap {
  int32_t group_index;
  int32_t flipper_flag;  // MessageCode
  float current_angle;
  float angle_dst;
  float angle_remainder;
  float move_speed;
};

struct PlungerSnap {
  int8_t pullback_started;
  int8_t unused0;
  int16_t some_counter;
  float boost;
};

struct CompSnap {
  int32_t group_index;
  int8_t active;
  int8_t unused0[3];
  int32_t message_field;
};

struct LightSnap {
  int32_t group_index;
  int8_t light_on;
  int8_t flasher_on;
  int8_t flash_light_on;
  int8_t unused0;
  int32_t light_on_bmp_index;
  int32_t message_field;
};

struct TableSnap {
  int32_t game_mode;
  int32_t time_ticks;
  float time_now;
  float time_next;
  float time_ticks_remainder;
  int32_t player_count;
  int32_t current_player;
  int32_t ball_count;
  int32_t extra_balls;
  int32_t multiball_count;
  int32_t ball_locked;
  int32_t tilt_lock;
  int32_t score_multiplier;
  int32_t jackpot_score;
  int32_t bonus_score;
  int32_t cur_score;
  int32_t ball_in_drain;
  int32_t single_step;
  int32_t player_score[4];
  int32_t player_ball_count[4];
  int32_t player_extra_balls[4];
  float nudge_count;
  int8_t nudged_left, nudged_right, nudged_up;
  int8_t unused1;
};
#pragma pack(pop)

struct Writer {
  uint8_t* p;
  uint8_t* end;
  bool ok;

  explicit Writer(uint8_t* base, std::size_t n)
      : p(base), end(base + n), ok(base != nullptr && n > 0) {}

  void bytes(const void* src, std::size_t n) {
    if (!ok) return;
    if (p + n > end) {
      ok = false;
      return;
    }
    std::memcpy(p, src, n);
    p += n;
  }

  void u8(uint8_t v) { bytes(&v, 1); }
  void u16(uint16_t v) {
    uint8_t b[2] = {static_cast<uint8_t>(v & 0xFF), static_cast<uint8_t>((v >> 8) & 0xFF)};
    bytes(b, 2);
  }
  void u32(uint32_t v) {
    uint8_t b[4] = {
        static_cast<uint8_t>(v & 0xFF), static_cast<uint8_t>((v >> 8) & 0xFF),
        static_cast<uint8_t>((v >> 16) & 0xFF), static_cast<uint8_t>((v >> 24) & 0xFF)};
    bytes(b, 4);
  }
  void i32(int32_t v) { u32(static_cast<uint32_t>(v)); }
  void f32(float v) {
    uint32_t u;
    std::memcpy(&u, &v, 4);
    u32(u);
  }
  void raw(const void* src, std::size_t n) { bytes(src, n); }
};

struct Reader {
  const uint8_t* p;
  const uint8_t* end;
  bool ok;

  explicit Reader(const uint8_t* base, std::size_t n)
      : p(base), end(base + n), ok(base != nullptr && n > 0) {}

  void bytes(void* dst, std::size_t n) {
    if (!ok) return;
    if (p + n > end) {
      ok = false;
      return;
    }
    std::memcpy(dst, p, n);
    p += n;
  }

  uint8_t u8() {
    uint8_t v = 0;
    bytes(&v, 1);
    return v;
  }
  uint16_t u16() {
    uint8_t b[2]{};
    bytes(b, 2);
    return static_cast<uint16_t>(b[0] | (b[1] << 8));
  }
  uint32_t u32() {
    uint8_t b[4]{};
    bytes(b, 4);
    return static_cast<uint32_t>(b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24));
  }
  int32_t i32() { return static_cast<int32_t>(u32()); }
  float f32() {
    uint32_t u = u32();
    float v;
    std::memcpy(&v, &u, 4);
    return v;
  }
  void raw(void* dst, std::size_t n) { bytes(dst, n); }
};

void clear_all_timers() {
  // Drop mid-flight callbacks; IDs on components become stale (zeroed below).
  timer::uninit();
  timer::init(150);
}

int collision_group_index(const TBall* ball) {
  if (!ball || !ball->CollisionComp) return -1;
  return ball->CollisionComp->GroupIndex;
}

TPinballComponent* find_by_group(TPinballTable* table, int group) {
  if (!table || group < 0) return nullptr;
  return table->find_component(group);
}

}  // namespace

std::size_t state_serialize_size() {
  if (!game_ready() || !pb::MainTable) return 0;
  return kStateMaxSize;
}

bool state_serialize(void* data, std::size_t size) {
  if (!data || size < kStateMaxSize) return false;
  if (!game_ready() || !pb::MainTable) return false;

  auto* dst = static_cast<uint8_t*>(data);
  std::memset(dst, 0, size);
  Writer w(dst, size - 4);  // leave room for CRC

  w.raw(kStateMagic, 4);
  w.u16(kStateVersion);
  w.u16(0);  // flags
  w.u32(kStateBuildId);

  TPinballTable* table = pb::MainTable;

  TableSnap ts{};
  ts.game_mode = static_cast<int32_t>(pb::game_mode);
  ts.time_ticks = pb::time_ticks;
  ts.time_now = pb::time_now;
  ts.time_next = pb::time_next;
  ts.time_ticks_remainder = pb::time_ticks_remainder;
  ts.player_count = table->PlayerCount;
  ts.current_player = table->CurrentPlayer;
  ts.ball_count = table->BallCount;
  ts.extra_balls = table->ExtraBalls;
  ts.multiball_count = table->MultiballCount;
  ts.ball_locked = table->BallLockedCounter;
  ts.tilt_lock = table->TiltLockFlag;
  ts.score_multiplier = table->ScoreMultiplier;
  ts.jackpot_score = table->JackpotScore;
  ts.bonus_score = table->BonusScore;
  ts.cur_score = table->CurScore;
  ts.ball_in_drain = table->BallInDrainFlag;
  ts.single_step = winmain::single_step ? 1 : 0;
  for (int i = 0; i < 4; ++i) {
    ts.player_score[i] = table->PlayerScores[i].Score;
    ts.player_ball_count[i] = table->PlayerScores[i].BallCount;
    ts.player_extra_balls[i] = table->PlayerScores[i].ExtraBalls;
  }
  ts.nudge_count = nudge::nudge_count;
  ts.nudged_left = nudge::nudged_left ? 1 : 0;
  ts.nudged_right = nudge::nudged_right ? 1 : 0;
  ts.nudged_up = nudge::nudged_up ? 1 : 0;
  w.raw(&ts, sizeof(ts));

  // Balls
  const int nb = static_cast<int>(table->BallList.size());
  const int nb_w = nb > kMaxBalls ? kMaxBalls : nb;
  w.i32(nb_w);
  for (int i = 0; i < nb_w; ++i) {
    TBall* ball = table->BallList[static_cast<std::size_t>(i)];
    BallSnap b{};
    b.active = ball && ball->ActiveFlag ? 1 : 0;
    b.collision_flag = ball ? static_cast<int8_t>(ball->CollisionFlag) : 0;
    if (ball) {
      b.pos_x = ball->Position.X;
      b.pos_y = ball->Position.Y;
      b.pos_z = ball->Position.Z;
      b.dir_x = ball->Direction.X;
      b.dir_y = ball->Direction.Y;
      b.dir_z = ball->Direction.Z;
      b.speed = ball->Speed;
      b.collision_group_index = collision_group_index(ball);
      b.stuck_counter = ball->StuckCounter;
    }
    w.raw(&b, sizeof(b));
  }

  // Flippers
  const int nf = static_cast<int>(table->FlipperList.size());
  const int nf_w = nf > kMaxFlippers ? kMaxFlippers : nf;
  w.i32(nf_w);
  for (int i = 0; i < nf_w; ++i) {
    TFlipper* flip = table->FlipperList[static_cast<std::size_t>(i)];
    FlipperSnap f{};
    f.group_index = flip ? flip->GroupIndex : -1;
    if (flip && flip->FlipperEdge) {
      f.flipper_flag = static_cast<int32_t>(flip->FlipperEdge->FlipperFlag);
      f.current_angle = flip->FlipperEdge->CurrentAngle;
      f.angle_dst = flip->FlipperEdge->AngleDst;
      f.angle_remainder = flip->FlipperEdge->AngleRemainder;
      f.move_speed = flip->FlipperEdge->MoveSpeed;
    }
    w.raw(&f, sizeof(f));
  }

  // Plunger
  PlungerSnap ps{};
  if (table->Plunger) {
    ps.pullback_started = table->Plunger->PullbackStartedFlag ? 1 : 0;
    ps.some_counter = static_cast<int16_t>(table->Plunger->SomeCounter);
    ps.boost = table->Plunger->Boost;
  }
  w.raw(&ps, sizeof(ps));

  // Components (ActiveFlag + MessageField)
  std::vector<CompSnap> comps;
  comps.reserve(table->ComponentList.size());
  for (auto* c : table->ComponentList) {
    if (!c) continue;
    if (dynamic_cast<TBall*>(c)) continue;  // balls handled above
    CompSnap s{};
    s.group_index = c->GroupIndex;
    s.active = c->ActiveFlag ? 1 : 0;
    s.message_field = c->MessageField;
    comps.push_back(s);
    if (static_cast<int>(comps.size()) >= kMaxComponents) break;
  }
  w.i32(static_cast<int32_t>(comps.size()));
  if (!comps.empty())
    w.raw(comps.data(), comps.size() * sizeof(CompSnap));

  // Lights
  std::vector<LightSnap> lights;
  for (auto* c : table->ComponentList) {
    auto* light = dynamic_cast<TLight*>(c);
    if (!light) continue;
    LightSnap s{};
    s.group_index = light->GroupIndex;
    s.light_on = light->LightOnFlag ? 1 : 0;
    s.flasher_on = light->FlasherOnFlag ? 1 : 0;
    s.flash_light_on = light->FlashLightOnFlag ? 1 : 0;
    s.light_on_bmp_index = light->LightOnBmpIndex;
    s.message_field = light->MessageField;
    lights.push_back(s);
    if (static_cast<int>(lights.size()) >= kMaxLights) break;
  }
  w.i32(static_cast<int32_t>(lights.size()));
  if (!lights.empty())
    w.raw(lights.data(), lights.size() * sizeof(LightSnap));

  if (!w.ok) {
    log(RETRO_LOG_ERROR, "state_serialize: buffer overflow\n");
    return false;
  }

  const std::size_t used = static_cast<std::size_t>(w.p - dst);
  const uint32_t crc = crc32_bytes(dst, used);
  dst[used] = static_cast<uint8_t>(crc & 0xFF);
  dst[used + 1] = static_cast<uint8_t>((crc >> 8) & 0xFF);
  dst[used + 2] = static_cast<uint8_t>((crc >> 16) & 0xFF);
  dst[used + 3] = static_cast<uint8_t>((crc >> 24) & 0xFF);
  // Pad remainder already zeroed.
  return true;
}

bool state_unserialize(const void* data, std::size_t size) {
  if (!data || size < 16) return false;
  if (!game_ready() || !pb::MainTable) return false;

  const auto* src = static_cast<const uint8_t*>(data);
  // Find used length: scan is hard; require full kStateMaxSize from frontend
  // or accept size >= used. Validate CRC over (size-4) only if size is exact.
  // Frontends pass serialize_size bytes; we wrote CRC at end of used region and
  // zero-padded. Recompute: find CRC by verifying header then reading until
  // we parse fully, then check CRC at cursor.

  Reader r(src, size);
  uint8_t magic[4]{};
  r.raw(magic, 4);
  if (!r.ok || magic[0] != kStateMagic[0] || magic[1] != kStateMagic[1] ||
      magic[2] != kStateMagic[2] || magic[3] != kStateMagic[3])
    return false;
  const uint16_t ver = r.u16();
  r.u16();  // flags
  const uint32_t build = r.u32();
  if (!r.ok || ver != kStateVersion || build != kStateBuildId) return false;

  TableSnap ts{};
  r.raw(&ts, sizeof(ts));
  if (!r.ok) return false;

  const int32_t nb = r.i32();
  if (!r.ok || nb < 0 || nb > kMaxBalls) return false;
  std::vector<BallSnap> balls(static_cast<std::size_t>(nb));
  for (int i = 0; i < nb; ++i) r.raw(&balls[static_cast<std::size_t>(i)], sizeof(BallSnap));

  const int32_t nf = r.i32();
  if (!r.ok || nf < 0 || nf > kMaxFlippers) return false;
  std::vector<FlipperSnap> flips(static_cast<std::size_t>(nf));
  for (int i = 0; i < nf; ++i) r.raw(&flips[static_cast<std::size_t>(i)], sizeof(FlipperSnap));

  PlungerSnap ps{};
  r.raw(&ps, sizeof(ps));

  const int32_t nc = r.i32();
  if (!r.ok || nc < 0 || nc > kMaxComponents) return false;
  std::vector<CompSnap> comps(static_cast<std::size_t>(nc));
  if (nc) r.raw(comps.data(), static_cast<std::size_t>(nc) * sizeof(CompSnap));

  const int32_t nl = r.i32();
  if (!r.ok || nl < 0 || nl > kMaxLights) return false;
  std::vector<LightSnap> lights(static_cast<std::size_t>(nl));
  if (nl) r.raw(lights.data(), static_cast<std::size_t>(nl) * sizeof(LightSnap));
  if (!r.ok) return false;

  // CRC follows payload.
  const std::size_t payload_end = static_cast<std::size_t>(r.p - src);
  if (payload_end + 4 > size) return false;
  const uint32_t expect = crc32_bytes(src, payload_end);
  const uint32_t got = static_cast<uint32_t>(src[payload_end] | (src[payload_end + 1] << 8) |
                                             (src[payload_end + 2] << 16) |
                                             (src[payload_end + 3] << 24));
  if (expect != got) return false;

  // --- Apply ---
  clear_all_timers();
  TPinballTable* table = pb::MainTable;

  pb::game_mode = static_cast<GameModes>(ts.game_mode);
  pb::time_ticks = ts.time_ticks;
  pb::time_now = ts.time_now;
  pb::time_next = ts.time_next;
  pb::time_ticks_remainder = ts.time_ticks_remainder;
  table->PlayerCount = ts.player_count;
  table->CurrentPlayer = ts.current_player;
  table->BallCount = ts.ball_count;
  table->ExtraBalls = ts.extra_balls;
  table->MultiballCount = ts.multiball_count;
  table->BallLockedCounter = ts.ball_locked;
  table->TiltLockFlag = ts.tilt_lock;
  table->ScoreMultiplier = ts.score_multiplier;
  table->JackpotScore = ts.jackpot_score;
  table->BonusScore = ts.bonus_score;
  table->CurScore = ts.cur_score;
  table->BallInDrainFlag = ts.ball_in_drain;
  winmain::single_step = ts.single_step != 0;
  for (int i = 0; i < 4; ++i) {
    table->PlayerScores[i].Score = ts.player_score[i];
    table->PlayerScores[i].BallCount = ts.player_ball_count[i];
    table->PlayerScores[i].ExtraBalls = ts.player_extra_balls[i];
  }
  nudge::nudge_count = ts.nudge_count;
  nudge::nudged_left = ts.nudged_left != 0;
  nudge::nudged_right = ts.nudged_right != 0;
  nudge::nudged_up = ts.nudged_up != 0;

  const int nballs = static_cast<int>(table->BallList.size());
  for (int i = 0; i < nb && i < nballs; ++i) {
    TBall* ball = table->BallList[static_cast<std::size_t>(i)];
    if (!ball) continue;
    const BallSnap& b = balls[static_cast<std::size_t>(i)];
    ball->ActiveFlag = b.active ? 1 : 0;
    ball->CollisionFlag = b.collision_flag;
    ball->Position.X = b.pos_x;
    ball->Position.Y = b.pos_y;
    ball->Position.Z = b.pos_z;
    ball->Direction.X = b.dir_x;
    ball->Direction.Y = b.dir_y;
    ball->Direction.Z = b.dir_z;
    ball->Speed = b.speed;
    ball->StuckCounter = b.stuck_counter;
    ball->EdgeCollisionCount = 0;
    ball->CollisionComp = nullptr;
    if (b.collision_group_index >= 0) {
      auto* comp = find_by_group(table, b.collision_group_index);
      ball->CollisionComp = dynamic_cast<TCollisionComponent*>(comp);
    }
    ball->Repaint();
  }

  const int nflip = static_cast<int>(table->FlipperList.size());
  for (int i = 0; i < nf && i < nflip; ++i) {
    TFlipper* flip = table->FlipperList[static_cast<std::size_t>(i)];
    if (!flip || !flip->FlipperEdge) continue;
    const FlipperSnap& f = flips[static_cast<std::size_t>(i)];
    flip->FlipperEdge->FlipperFlag = static_cast<MessageCode>(f.flipper_flag);
    flip->FlipperEdge->CurrentAngle = f.current_angle;
    flip->FlipperEdge->AngleDst = f.angle_dst;
    flip->FlipperEdge->AngleRemainder = f.angle_remainder;
    flip->FlipperEdge->MoveSpeed = f.move_speed;
    flip->FlipperEdge->set_control_points(flip->FlipperEdge->CurrentAngle);
    flip->UpdateSprite();
  }

  if (table->Plunger) {
    table->Plunger->PullbackStartedFlag = ps.pullback_started != 0;
    table->Plunger->SomeCounter = ps.some_counter;
    table->Plunger->Boost = ps.boost;
    table->Plunger->PullbackTimer_ = 0;
    table->Plunger->BallFeedTimer_ = 0;
  }

  for (const CompSnap& s : comps) {
    TPinballComponent* c = find_by_group(table, s.group_index);
    if (!c) continue;
    c->ActiveFlag = s.active ? 1 : 0;
    c->MessageField = s.message_field;
  }

  for (const LightSnap& s : lights) {
    auto* light = dynamic_cast<TLight*>(find_by_group(table, s.group_index));
    if (!light) continue;
    light->LightOnFlag = s.light_on != 0;
    light->FlasherOnFlag = s.flasher_on != 0;
    light->FlashLightOnFlag = s.flash_light_on != 0;
    light->LightOnBmpIndex = s.light_on_bmp_index;
    light->MessageField = s.message_field;
    light->FlashTimer = 0;
    light->TimeoutTimer = 0;
    light->UndoOverrideTimer = 0;
    light->SetSpriteBmp(light->BmpArr[light->LightOnFlag ? 1 : 0]);
  }

  table->LightShowTimer = 0;
  table->EndGameTimeoutTimer = 0;
  table->TiltTimeoutTimer = 0;
  table->ReplayTimer = 0;

  log(RETRO_LOG_INFO, "state_unserialize OK (balls=%d flippers=%d comps=%d lights=%d)\n",
      nb, nf, nc, nl);
  return true;
}

}  // namespace scp
