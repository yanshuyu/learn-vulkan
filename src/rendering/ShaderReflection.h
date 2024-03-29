#pragma


class ShaderProgram;


class ShaderReflection
{
public:
    static void Initailze();
    static void DeInitailize();
    static void Parse(ShaderProgram* shader);
};