#pragma once
#include"core\CoreUtils.h"
#include<map>
#include<unordered_map>
#include<memory>

class Device;


enum SetIndices
{
    PerFrame,
    PerCamera,
    PerMaterial,
    PerObject,
    MaxSetIndex,
};





class DescriptorSetManager
{
private:
    class AutoDescriptorPool
    {
    private:
        std::vector<VkDescriptorSetLayoutBinding> _setLayoutBindings;
        VkDescriptorSetLayout _setLayout{VK_NULL_HANDLE};
        size_t _maxSetPerPool{0};

        std::vector<VkDescriptorPool> _pools{};
        int _activePoolIdx{-1};

        std::map<VkDescriptorType, size_t> _poolSz{};
        VkDescriptorPoolCreateFlags _poolFlags{0};

        VkDevice _device{VK_NULL_HANDLE};
    private:
        VkDescriptorPool _alloc_pool();
        VkDescriptorPool _get_active_pool();    

    public:
        AutoDescriptorPool(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& setBindings , size_t maxSetPerPool, VkDescriptorPoolCreateFlags poolFlags = 0);
        AutoDescriptorPool(VkDevice device, std::vector<VkDescriptorSetLayoutBinding>&& setBindings , size_t maxSetPerPool, VkDescriptorPoolCreateFlags poolFlags = 0);
        ~AutoDescriptorPool();

        NONE_COPYABLE_NONE_MOVEABLE(AutoDescriptorPool);

        VkDescriptorSet AllocDescriptorSet();
        bool FreeDescriptorSet(VkDescriptorSet set);
        VkDescriptorSetLayout GetDescriptorSetLayout() const { return _setLayout; }
        const std::vector<VkDescriptorSetLayoutBinding>& GetDescriptorSetLayoutBindings() const { return _setLayoutBindings; }
        void Reset();
        void Release();
    };

private:
    static std::array<std::unique_ptr<AutoDescriptorPool>, MaxSetIndex> s_CommonDescriptorPools;
    static std::unordered_map<size_t, std::unique_ptr<AutoDescriptorPool>> s_PerMaterialDescriptorPools;
    static Device* s_Device;

public:
    DescriptorSetManager() = delete;
    ~DescriptorSetManager() = delete;

    NONE_COPYABLE_NONE_MOVEABLE(DescriptorSetManager)

    static void Initailize(Device* pdevice);
    static void DeInitailize();

    static void RegisterSetLayout(SetIndices setIdx, size_t setHash, const std::vector<VkDescriptorSetLayoutBinding>& setBindings, bool freeableSet = false, size_t maxSetPerPool = 1);
    static void RegisterSetLayout(SetIndices setIdx, size_t setHash, std::vector<VkDescriptorSetLayoutBinding>&& setBindings, bool freeableSet = false, size_t maxSetPerPool = 1);
    static VkDescriptorSetLayout GetSetLayout(SetIndices setIdx, size_t setHash);
    static const std::vector<VkDescriptorSetLayoutBinding>* GetSetLayoutBindings(SetIndices setIdx, size_t setHash);
    static VkDescriptorSet AllocDescriptorSet(SetIndices setIdx, size_t setHash);
    static bool FreeDescriptorSet(SetIndices setIdx, size_t setHash, VkDescriptorSet set);
    static void ResetDescriptorPool(SetIndices setIdx, size_t setHash);
   
};

