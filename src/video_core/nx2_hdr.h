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
///  - a present shader that scales that linear buffer to the paper-white level.
/// The tone curve itself is changed by a shader override file (shader_override folder).
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

} // namespace VideoCommon::Nx2Hdr
