#version 450

layout(location = 0) in  vec4 v_color;
layout(location = 0) out vec4 fragColor;

void main()
{
    if (v_color.a < 0.01) discard;

    vec2 uv = gl_PointCoord - vec2(0.5);
    float distSq = dot(uv, uv) * 4.0;
    if (distSq > 1.0) discard;

    float t = 1.0 - distSq;
    float core = t * t * t * t * t * t;  // t^6: bordo molto netto
    float halo = t * t;                   // t^2: alone contenuto

    float intensity = core * 1.2 + halo * 0.3;

    fragColor = vec4(v_color.rgb * intensity, v_color.a * halo);
}