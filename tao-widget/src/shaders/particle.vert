#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec4 color;     // normalised RGBA from ubyte vertex data

layout(std140, binding = 0) uniform buf {
    mat4  qt_Matrix;
    float qt_Opacity;
} ubuf;

layout(location = 0) out vec2 v_texcoord;
layout(location = 1) out vec4 v_color;

void main()
{
    gl_Position = ubuf.qt_Matrix * vec4(position, 0.0, 1.0);
    v_texcoord  = texcoord;
    // Color is already premultiplied. Scale uniformly by scene opacity.
    v_color = color * ubuf.qt_Opacity;
}
