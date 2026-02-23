#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in float size;
layout(location = 2) in vec4 color;

layout(std140, binding = 0) uniform buf {
    mat4  qt_Matrix;
    float qt_Opacity;
} ubuf;

layout(location = 0) out vec4 v_color;

void main()
{
    gl_Position = ubuf.qt_Matrix * vec4(position, 0.0, 1.0);
    gl_PointSize = size;
    v_color = color * ubuf.qt_Opacity;
}
