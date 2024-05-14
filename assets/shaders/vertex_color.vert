#version 460
#extension GL_ARB_shading_language_include : require
#include "common.glsl"

layout (location = 0) in vec3 i_posOS;
layout (location = 1) in vec4 i_vertColor;
layout (location = 2) in vec2 i_UV;

layout(location = 0) out vec4 _vertColor;
layout(location = 1) out vec2 _uv;

void main()
{
    gl_Position = TransformObjectToClip(i_posOS);
    _vertColor = i_vertColor;
    _uv = i_UV;
}