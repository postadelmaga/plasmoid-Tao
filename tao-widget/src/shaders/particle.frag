#version 450

layout(location = 0) in  vec2 v_texcoord;
layout(location = 1) in  vec4 v_color;
layout(location = 0) out vec4 fragColor;

void main()
{
    // Procedural radial glow — no texture needed.
    // UV (0,0)=top-left, (1,1)=bottom-right, center=(0.5,0.5)
    vec2  uv   = v_texcoord - vec2(0.5);
    float dist = length(uv) * 2.0;          // 0=center, 1=edge
    float glow = exp(-dist * dist * 3.0);   // smooth gaussian falloff
    fragColor  = v_color * glow;
}
