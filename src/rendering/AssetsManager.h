#pragma once
#include<string>
#include<unordered_map>
#include<memory>
#include"core\CoreUtils.h"

class Device;
class ShaderProgram;

class AssetsManager
{
private:
    static std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> s_programs;
    static std::unordered_map<std::string, std::function<void(ShaderProgram*)>> s_programResourceInitailizer;
    static Device* s_pDevice;

    const static std::string s_shaderDir;

private:
    static VkShaderModule _load_shader_moudle(const char* srcPath);

public:
    AssetsManager() = delete;
    ~AssetsManager() = delete;

    NONE_COPYABLE_NONE_MOVEABLE(AssetsManager)

    static void Initailize(Device* pDevice);

    static ShaderProgram* LoadProgram(const char* vs, const char* fs) { return LoadProgram(vs, "main", fs, "main"); };

    static ShaderProgram* LoadProgram(const char* vs, const char* vsName, const char* fs, const char* fsName);
    
    static void Release();

};

