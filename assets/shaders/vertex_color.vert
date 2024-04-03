#version 460

#include "common.glsl"

layout (location = 0) in vec3 i_posOS;
layout (location = 1) in vec4 i_vertColor;

layout(location = 0) out vec4 _vertColor;

void main()
{
    gl_Position = TransformObjectToClip(i_posOS);
    _vertColor = i_vertColor;
}