#pragma once
#include<core\CoreUtils.h>

class ShaderProgram;



class Material
{
private:
    ShaderProgram* _shaderProgram{ nullptr };


public:
    Material(ShaderProgram* shader);
    ~Material();

    NONE_COPYABLE_NONE_MOVEABLE(Material)

    const ShaderProgram* GetShaderProgram() const { return _shaderProgram; }
};


