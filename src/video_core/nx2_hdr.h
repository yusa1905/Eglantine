// SPDX-FileCopyrightText: Copyright 2026 yusa1905 (Eglantine fork)
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <atomic>
#include <cstdlib>

#include "common/common_types.h"

/// HDR test (yusa1905 fork, Mario Kart World). EDEN_HDR=1 turns on:
///  - an scRGB (FP16, extended sRGB linear) swapchain when the driver offers one,
///  - showing the game's float buffer that its last full-screen copy (fragment shader
///    52402d8e8ab55263) reads, instead of the 8-bit sRGB image that copy writes,
///  - a present shader that applies the HDR roll-off at the chosen paper white and peak.
/// The game's own shoulder is removed by a shader override file (shader_override folder).
/// F10 while a game runs cycles the view: HDR / SDR (the game's own curve) / split. Paper white and
/// peak come from <data dir>/hdr_settings.txt ("paper_white 203", "peak 400"), re-read on F10.
namespace VideoCommon::Nx2Hdr {

/// Fragment shader hash of MKW's final full-screen copy into the 8-bit display image.
constexpr u64 FinalCopyShader = 0x52402d8e8ab55263ULL;

inline bool Enabled() {
    static const bool enabled = std::getenv("EDEN_HDR") != nullptr;
    return enabled;
}

/// Slot index of the image view the final copy sampled last (0xffffffff = none yet).
inline std::atomic<u32> source_view{0xffffffffU};

/// Set by the swapchain when it was created in the scRGB colour space.
inline std::atomic<bool> swapchain_hdr{false};

/// Present parameters (read by the present pass when it builds its pipeline).
struct Params {
    float paper_white_nits;
    float peak_nits;
    s32 view_mode; ///< 0 HDR, 1 SDR (game curve), 2 split
};

/// Current parameters; loads the saved ones on first use.
Params GetParams();

/// Polls F10; returns true when the view changed (the present pass must be rebuilt).
bool PollKeys();

} // namespace VideoCommon::Nx2Hdr
