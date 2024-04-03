#pragma
#include<unordered_map>
#include<string>
#include<functional>

class ShaderProgram;


typedef std::function<bool(ShaderProgram*)> Fn_Parser;

class ShaderReflection
{
public:
    static void Initailze();
    static void DeInitailize();
    static void Parse(ShaderProgram* program);
    static void RegisterParser(const char* hash, Fn_Parser parser);
    
private:
    static std::unordered_map<std::string, Fn_Parser> s_ShaderParser;
    static void _parse_common(ShaderProgram* program);
};