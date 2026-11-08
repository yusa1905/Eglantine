// SPDX-FileCopyrightText: Copyright 2026 yusa1905 (Eglantine fork)
// SPDX-License-Identifier: GPL-3.0-or-later

#version 460 core

// HDR test (Mario Kart World). The source is the game's linear float buffer after its exposure,
// with no shoulder applied (the shader override leaves it out) and the UI composited on top.
// Values up to the game's shoulder start (0.75) are kept; above it this pass rolls off smoothly
// towards the peak instead of the game's 1.0. The swapchain is scRGB: 1.0 = 80 nits.
layout (location = 0) in vec2 frag_tex_coord;
layout (location = 0) out vec4 color;
layout (binding = 0) uniform sampler2D color_texture;

layout (constant_id = 0) const float PAPER_WHITE_NITS = 203.0;
layout (constant_id = 1) const float PEAK_NITS = 400.0;
layout (constant_id = 2) const int VIEW_MODE = 0; // 0 HDR, 1 SDR (game curve), 2 split

const float KNEE = 0.75;

vec3 HdrCurve(vec3 x) {
    float range = max(PEAK_NITS / PAPER_WHITE_NITS - KNEE, 0.001);
    vec3 rolled = KNEE + range * (vec3(1.0) - exp(-max(x - KNEE, vec3(0.0)) / range));
    return mix(x, rolled, step(vec3(KNEE), x));
}

// The game's own SDR shoulder: 1 - 0.25 * e^(-4 (x - 0.75)) above 0.75.
vec3 SdrCurve(vec3 x) {
    vec3 shoulder = vec3(1.0) - 0.25 * exp(-4.0 * max(x - KNEE, vec3(0.0)));
    return mix(x, shoulder, step(vec3(KNEE), x));
}

void main() {
    vec3 x = max(texture(color_texture, frag_tex_coord).rgb, vec3(0.0));
    vec3 shown;
    if (VIEW_MODE == 1) {
        shown = SdrCurve(x);
    } else if (VIEW_MODE == 2) {
        if (abs(frag_tex_coord.x - 0.5) < 0.0008) {
            color = vec4(vec3(PAPER_WHITE_NITS / 80.0), 1.0);
            return;
        }
        shown = frag_tex_coord.x < 0.5 ? HdrCurve(x) : SdrCurve(x);
    } else {
        shown = HdrCurve(x);
    }
    color = vec4(shown * (PAPER_WHITE_NITS / 80.0), 1.0);
}
