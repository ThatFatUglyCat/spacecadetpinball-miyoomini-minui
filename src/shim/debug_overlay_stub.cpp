// SPDX-License-Identifier: MIT
#include "pch.h"
#include "DebugOverlay.h"

gdrv_bitmap8* DebugOverlay::dbScreen = nullptr;

void DebugOverlay::UnInit() {
  delete dbScreen;
  dbScreen = nullptr;
}

void DebugOverlay::DrawOverlay() {}
void DebugOverlay::DrawCicleType(circle_type&) {}
void DebugOverlay::DrawLineType(line_type&) {}
void DebugOverlay::DrawEdge(TEdgeSegment*) {}
void DebugOverlay::DrawBoxGrid() {}
void DebugOverlay::DrawAllEdges() {}
void DebugOverlay::DrawBallInfo() {}
void DebugOverlay::DrawAllSprites() {}
void DebugOverlay::DrawSoundPositions() {}
void DebugOverlay::DrawBallDepthSteps() {}
void DebugOverlay::DrawComponentAabb() {}
