#pragma once
#include<string>
#include<unordered_map>
#include<memory>
#include"core\CoreUtils.h"

struct ShaderStageInfo;
class Device;
class ShaderProgram;
class Texture2D;

class AssetsManager
{
private:
    static std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> s_programs;
    static std::unordered_map<std::string, ShaderStageInfo> s_shaderModules;

    static std::unordered_map<std::string, std::unique_ptr<Texture2D>> s_tex2Ds;    

    static Device* s_pDevice;
private:
    static const ShaderStageInfo* _load_shader_moudle(const char * srcFile, const char* entryName, VkShaderStageFlagBits stage);
    static bool _parse_shader_reflection(ShaderProgram* shader);
    static VertexAttribute _attr_name_to_type(const char* name);

public:
    AssetsManager() = delete;
    ~AssetsManager() = delete;

    NONE_COPYABLE_NONE_MOVEABLE(AssetsManager)

    static void Initailize(Device* pDevice);
    static void DeInitailize();

    static ShaderProgram* LoadProgram(const char* vs, const char* fs) { return LoadProgram(vs, "main", fs, "main"); };
    static ShaderProgram* LoadProgram(const char* vs, const char* vsName, const char* fs, const char* fsName);
    static void UnloadProgram(ShaderProgram* program);
    
    static Texture2D* LoadTexture2D(const char* path);
    static void UnloadTexture2D(const char* path);
    static void UnloadTexture2D(Texture2D* tex);
    
};

