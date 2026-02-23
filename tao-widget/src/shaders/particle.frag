#version 450

layout(location = 0) in  vec4 v_color;
layout(location = 0) out vec4 fragColor;

void main()
{
    // Procedural radial glow — no texture needed.
    // gl_PointCoord (0,0)=top-left, (1,1)=bottom-right, center=(0.5,0.5)
    vec2  uv   = gl_PointCoord - vec2(0.5);
    float dist = length(uv) * 2.0;          // 0=center, 1=edge
    float glow = exp(-dist * dist * 3.0);   // smooth gaussian falloff
    fragColor  = v_color * glow;
}
