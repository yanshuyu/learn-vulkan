#pragma once
#include"core\CoreUtils.h"
#include<map>
#include<unordered_map>
#include<memory>

class ShaderProgram;

class DescriptorManager
{
private:
    class ProgramDescriptorPool
    {
    private:
        ShaderProgram* _program{nullptr};
        size_t _maxSetPerPool{0};

        std::vector<VkDescriptorPool> _pools{};
        int _activePoolIdx{-1};

        std::map<VkDescriptorType, size_t> _poolSz{};

    private:
        VkDescriptorPool _alloc_pool();
        VkDescriptorPool _get_active_pool();    

    public:
        ProgramDescriptorPool(ShaderProgram* program, size_t maxSetSz = 2);
        ~ProgramDescriptorPool();

        NONE_COPYABLE_NONE_MOVEABLE(ProgramDescriptorPool);

        bool AllocateProgramDescriptorSet(std::vector<VkDescriptorSet>& pResult);
        void Reset();
        void Release();

    };

private:
    static std::unordered_map<ShaderProgram*, size_t> s_poolSzHints;
    static std::unordered_map<ShaderProgram*, std::unique_ptr<ProgramDescriptorPool>> s_pools; 

public:
    DescriptorManager() = delete;
    ~DescriptorManager() = delete;

    NONE_COPYABLE_NONE_MOVEABLE(DescriptorManager)

    static void SetProgramDescriptorPoolSizeHint(ShaderProgram* program, size_t maxSetPerPool) { s_poolSzHints[program] = maxSetPerPool; };
    static size_t GetProgramDescriptorPoolSizeHint(ShaderProgram* program);
    static bool AllocProgramDescriptorSet(ShaderProgram* program, std::vector<VkDescriptorSet>& sets);
    static void ResetProgramDescriptorSets(ShaderProgram* program);
    static void ReleaseProgramDescriptorSets(ShaderProgram* program);
    static void Release();
};

