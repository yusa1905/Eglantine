// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

#include "common/common_types.h"

// EDEN_NX2_PASSLOG=<frame>[,<frame>...]: log every draw, clear and dispatch of those frames
// (counted by RasterizerVulkan::TickFrame) with their render targets, sampled textures and shader
// hashes. The hashes match the .spv files gpu_log_shader_dumps writes. Debug aid for finding a
// game's tonemapping pass.
namespace VideoCommon::Nx2PassLog {

inline std::atomic<bool> active{false};
inline std::atomic<u32> lines{0};
constexpr u32 MaxLinesPerFrame = 20000;

inline const std::vector<u64>& Frames() {
    static const std::vector<u64> frames = [] {
        std::vector<u64> result;
        const char* value = std::getenv("EDEN_NX2_PASSLOG");
        if (value == nullptr) {
            return result;
        }
        std::string_view text{value};
        while (!text.empty()) {
            const size_t comma = text.find(',');
            const std::string_view part = text.substr(0, comma);
            result.push_back(std::strtoull(std::string(part).c_str(), nullptr, 10));
            if (comma == std::string_view::npos) {
                break;
            }
            text.remove_prefix(comma + 1);
        }
        return result;
    }();
    return frames;
}

/// Called once a frame; returns whether this frame is logged.
inline bool OnFrame(u64 frame) {
    const auto& frames = Frames();
    const bool log = std::find(frames.begin(), frames.end(), frame) != frames.end();
    active.store(log, std::memory_order_relaxed);
    lines.store(0, std::memory_order_relaxed);
    return log;
}

/// Whether a line may be written now (active and under the per-frame cap).
inline bool Take() {
    return active.load(std::memory_order_relaxed) &&
           lines.fetch_add(1, std::memory_order_relaxed) < MaxLinesPerFrame;
}

} // namespace VideoCommon::Nx2PassLog
