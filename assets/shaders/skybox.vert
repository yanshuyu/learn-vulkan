#version 460

#include"common.glsl"

layout(location = 0) in vec3 i_posOS;
layout(location = 0) out vec3 _uvw;

void main()
{
    vec4 position = vec4(0, 0, 0, 1);
    position.xyz =  mat3x3(_Matrix_M) * i_posOS;
    _uvw = normalize(position.xyz);
    position.xyz = mat3x3(_Matrix_V) * position.xyz;
    position = _Matrix_P * position;
    position.z = position.w;
    gl_Position = position;
}