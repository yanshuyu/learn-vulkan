#version 460
#extension GL_ARB_shading_language_include : require
#include"common.glsl"

layout(location = 0) in vec3 _uvw;
layout(location = 0) out vec4 o_fragColor;

layout(set = SET_INDEX_PER_MATERIAL, binding = 0) uniform samplerCube _SkyCubeTex;


void main()
{
    o_fragColor = texture(_SkyCubeTex, _uvw);
    o_fragColor.a = 1;
}