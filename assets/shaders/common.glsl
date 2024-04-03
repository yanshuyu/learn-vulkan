#ifndef _COMMON_GLSL_INC
#define _COMMON_GLSL_INC

#define SET_INDEX_PER_FRAME 0
#define SET_INDEX_PER_CAMERA 1
#define SET_INDEX_PER_MATERIAL 2
#define SET_INDEX_PER_OBJECT 3

layout(set = SET_INDEX_PER_FRAME, binding = 0) uniform _PerFrame 
{
    vec4 _TimeParams;
    
};


layout(set = SET_INDEX_PER_CAMERA, binding = 0) uniform _PerCamera
{
    mat4 _Matrix_V;
    mat4 _Matrix_P;
    mat4 _Matrix_VP;
    mat4 _Matrix_Inv_V;
    mat4 _Matrix_Inv_VP;

};


layout(set = SET_INDEX_PER_OBJECT, binding = 0) uniform _PerObject
{
    mat4 _Matrix_M;
    mat4 _Matrix_Inv_M;

};


vec3 TransformObjectToWorld(vec3 posOS)
{
    return (_Matrix_M * vec4(posOS, 1)).xyz;
}

vec4 TransformObjectToClip(vec3 posOS)
{
    return _Matrix_VP * _Matrix_M * vec4(posOS, 1);
}

#endif //_COMMON_GLSL_INC