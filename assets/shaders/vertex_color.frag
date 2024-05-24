#version 460
#extension GL_ARB_shading_language_include : require
#include "common.glsl"

layout(location = 0) in vec4 _vertColor;
layout(location = 1) in vec2 _uv;

layout(location = 0) out vec4 o_fragColor;
 

layout(set=SET_INDEX_PER_MATERIAL, binding=0) uniform sampler2D _mainTex; 
layout(set=SET_INDEX_PER_MATERIAL, binding=1) uniform _PerMaterial
{ 
    vec4 _mainColor;
};

void main()
{
    o_fragColor = _vertColor * texture(_mainTex, _uv) * _mainColor;
}