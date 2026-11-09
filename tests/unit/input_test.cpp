// SPDX-License-Identifier: MIT
#include "input.h"

#include <cstdio>

static int failures = 0;

static void expect(bool cond, const char* msg) {
  if (!cond) {
    std::fprintf(stderr, "FAIL: %s\n", msg);
    ++failures;
  }
}

static bool has_kind(const scp::InputEvent* ev, int n, scp::InputEventKind kind) {
  for (int i = 0; i < n; ++i)
    if (ev[i].kind == kind) return true;
  return false;
}

static int count_kind(const scp::InputEvent* ev, int n, scp::InputEventKind kind) {
  int c = 0;
  for (int i = 0; i < n; ++i)
    if (ev[i].kind == kind) ++c;
  return c;
}

int main() {
  scp::InputButtons prev{};
  scp::InputHoldState holds{};
  scp::InputOptions opts{};
  scp::InputEvent ev[scp::kMaxInputEvents];

  // Flipper press / release edges; no key-repeat while held.
  {
    prev = scp::InputButtons{};
    holds = scp::InputHoldState{};
    scp::InputButtons now{};
    now.left_flipper = true;
    int n = scp::translate_input_edges(now, prev, holds, opts, false, ev, scp::kMaxInputEvents);
    expect(n == 1 && has_kind(ev, n, scp::InputEventKind::LeftFlipperDown), "flipper down");
    expect(holds.left_flipper, "hold latched");

    now.left_flipper = true;
    n = scp::translate_input_edges(now, prev, holds, opts, false, ev, scp::kMaxInputEvents);
    expect(n == 0, "no repeat while held");

    now.left_flipper = false;
    n = scp::translate_input_edges(now, prev, holds, opts, false, ev, scp::kMaxInputEvents);
    expect(n == 1 && has_kind(ev, n, scp::InputEventKind::LeftFlipperUp), "flipper up");
    expect(!holds.left_flipper, "hold cleared");
  }

  // Pause while flipper held releases same frame.
  {
    prev = scp::InputButtons{};
    holds = scp::InputHoldState{};
    scp::InputButtons now{};
    now.left_flipper = true;
    scp::translate_input_edges(now, prev, holds, opts, false, ev, scp::kMaxInputEvents);
    expect(holds.left_flipper, "armed");

    now.pause = true;
    now.left_flipper = true;
    int n = scp::translate_input_edges(now, prev, holds, opts, false, ev, scp::kMaxInputEvents);
    expect(has_kind(ev, n, scp::InputEventKind::Pause), "pause edge");
    expect(has_kind(ev, n, scp::InputEventKind::LeftFlipperUp), "release on pause");
    expect(!holds.left_flipper, "hold cleared on pause");
  }

  // Unpause while still holding re-presses.
  {
    prev.pause = true;
    prev.left_flipper = true;
    holds = scp::InputHoldState{};
    scp::InputButtons now{};
    now.pause = true;
    now.left_flipper = true;
    // Still blocked (paused), no press.
    int n = scp::translate_input_edges(now, prev, holds, opts, true, ev, scp::kMaxInputEvents);
    expect(n == 0 && !holds.left_flipper, "blocked while paused");

    now.pause = false;  // physical pause released; still blocked until Start edge
    n = scp::translate_input_edges(now, prev, holds, opts, true, ev, scp::kMaxInputEvents);
    expect(n == 0, "no unpause without Start edge");

    now.pause = true;  // Start press toggles pause off
    n = scp::translate_input_edges(now, prev, holds, opts, true, ev, scp::kMaxInputEvents);
    expect(has_kind(ev, n, scp::InputEventKind::Pause), "unpause edge");
    expect(has_kind(ev, n, scp::InputEventKind::LeftFlipperDown), "re-press after unpause");
  }

  // Plunger tap mode: Down+Up on press, no event on release.
  {
    prev = scp::InputButtons{};
    holds = scp::InputHoldState{};
    opts.plunger_tap = true;
    scp::InputButtons now{};
    now.plunger = true;
    int n = scp::translate_input_edges(now, prev, holds, opts, false, ev, scp::kMaxInputEvents);
    expect(count_kind(ev, n, scp::InputEventKind::PlungerDown) == 1, "tap down");
    expect(count_kind(ev, n, scp::InputEventKind::PlungerUp) == 1, "tap up");
    now.plunger = false;
    n = scp::translate_input_edges(now, prev, holds, opts, false, ev, scp::kMaxInputEvents);
    expect(n == 0, "tap release silent");
    opts.plunger_tap = false;
  }

  // Nudge disabled suppresses edges.
  {
    prev = scp::InputButtons{};
    holds = scp::InputHoldState{};
    opts.nudge_enabled = false;
    scp::InputButtons now{};
    now.nudge_left = true;
    int n = scp::translate_input_edges(now, prev, holds, opts, false, ev, scp::kMaxInputEvents);
    expect(n == 0, "nudge disabled");
    opts.nudge_enabled = true;
  }

  // One-shot New Game / High Scores / toggles.
  {
    prev = scp::InputButtons{};
    holds = scp::InputHoldState{};
    scp::InputButtons now{};
    now.new_game = true;
    now.high_scores = true;
    now.toggle_sound = true;
    int n = scp::translate_input_edges(now, prev, holds, opts, false, ev, scp::kMaxInputEvents);
    expect(has_kind(ev, n, scp::InputEventKind::NewGame), "new game");
    expect(has_kind(ev, n, scp::InputEventKind::HighScores), "high scores");
    expect(has_kind(ev, n, scp::InputEventKind::ToggleSound), "toggle sound");
    n = scp::translate_input_edges(now, prev, holds, opts, false, ev, scp::kMaxInputEvents);
    expect(n == 0, "no one-shot repeat");
  }

  // L2 alias covered by read path is not unit-tested here (needs state_cb);
  // OR semantics are documented in INPUT_MAPPING.md.

  if (failures) {
    std::fprintf(stderr, "%d failure(s)\n", failures);
    return 1;
  }
  std::puts("input_unit ok");
  return 0;
}
