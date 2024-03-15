#version 460

layout (location = 0) in vec3 i_posOS;
layout (location = 1) in vec4 i_vertColor;

layout (set = 0, binding = 0) uniform PerObject
{
    mat4 _Matrix_M;
    mat4 _Matrix_V;
    mat4 _Matrix_P;
};


layout(location = 0) out vec4 _vertColor;

void main()
{
    gl_Position = _Matrix_P * _Matrix_V * _Matrix_M * vec4(i_posOS, 1);
    _vertColor = i_vertColor;
}