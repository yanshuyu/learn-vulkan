#version 460

layout(location = 0) in vec4 _vertColor;
 
layout(location = 0) out vec4 o_fragColor; 

void main()
{
    o_fragColor = _vertColor;
} 