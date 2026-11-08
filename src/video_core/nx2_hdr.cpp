// SPDX-FileCopyrightText: Copyright 2026 yusa1905 (Eglantine fork)
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <algorithm>
#include <fstream>
#include <mutex>
#include <string>

#include "common/fs/path_util.h"
#include "common/logging.h"
#include "video_core/nx2_hdr.h"

namespace VideoCommon::Nx2Hdr {
namespace {

std::mutex params_mutex;
Params params{203.0f, 400.0f, 0};
bool loaded = false;

std::filesystem::path SettingsPath() {
    return Common::FS::GetEdenPath(Common::FS::EdenPath::EdenDir) / "hdr_settings.txt";
}

void Clamp(Params& p) {
    p.paper_white_nits = std::clamp(p.paper_white_nits, 80.0f, 500.0f);
    p.peak_nits = std::clamp(p.peak_nits, p.paper_white_nits, 2000.0f);
    p.view_mode = std::clamp(p.view_mode, 0, 2);
}

void Save() {
    std::ofstream file(SettingsPath(), std::ios::trunc);
    file << "paper_white " << params.paper_white_nits << "\npeak " << params.peak_nits << "\n";
}

void Load() {
    loaded = true;
    if (!std::filesystem::exists(SettingsPath())) {
        Save();
    }
    std::ifstream file(SettingsPath());
    std::string key;
    float value{};
    while (file >> key >> value) {
        if (key == "paper_white") {
            params.paper_white_nits = value;
        } else if (key == "peak") {
            params.peak_nits = value;
        }
    }
    Clamp(params);
    LOG_INFO(Render_Vulkan, "HDR test: paper white {} nits, peak {} nits", params.paper_white_nits,
             params.peak_nits);
}

#ifdef _WIN32
bool Pressed(int key, bool& was_down) {
    const bool down = (GetAsyncKeyState(key) & 0x8000) != 0;
    const bool pressed = down && !was_down;
    was_down = down;
    return pressed;
}
#endif

} // Anonymous namespace

Params GetParams() {
    std::scoped_lock lock{params_mutex};
    if (!loaded) {
        Load();
    }
    return params;
}

bool PollKeys() {
#ifdef _WIN32
    static bool f10{};
    if (!Pressed(VK_F10, f10)) {
        return false;
    }
    std::scoped_lock lock{params_mutex};
    const s32 mode = (params.view_mode + 1) % 3;
    // Re-read the file so edits apply without a restart.
    Load();
    params.view_mode = mode;
    static constexpr const char* names[] = {"HDR", "SDR", "split (left HDR, right SDR)"};
    LOG_INFO(Render_Vulkan, "HDR test: {} | paper white {} nits | peak {} nits",
             names[params.view_mode], params.paper_white_nits, params.peak_nits);
    return true;
#else
    return false;
#endif
}

} // namespace VideoCommon::Nx2Hdr
