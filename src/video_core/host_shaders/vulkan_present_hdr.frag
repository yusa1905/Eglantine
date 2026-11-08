// SPDX-FileCopyrightText: Copyright 2026 yusa1905 (Eglantine fork)
// SPDX-License-Identifier: GPL-3.0-or-later

#version 460 core

// HDR test: the source is the game's linear float buffer (1.0 = paper white); the swapchain is
// scRGB, where 1.0 = 80 nits. 203 nits is the BT.2408 reference white for graphics.
layout (location = 0) in vec2 frag_tex_coord;
layout (location = 0) out vec4 color;
layout (binding = 0) uniform sampler2D color_texture;

const float PAPER_WHITE_SCALE = 203.0 / 80.0;

void main() {
    vec3 rgb = max(texture(color_texture, frag_tex_coord).rgb, vec3(0.0));
    color = vec4(rgb * PAPER_WHITE_SCALE, 1.0);
}
