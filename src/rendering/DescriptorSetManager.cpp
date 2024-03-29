#include"DescriptorSetManager.h"
#include"core\ShaderProgram.h"
#include"core\Device.h"
#include"rendering\RenderData.h"


DescriptorSetManager::AutoDescriptorPool::AutoDescriptorPool(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& setBindings, size_t maxSetPerPool, VkDescriptorPoolCreateFlags poolFlags)
: _device(device)
, _maxSetPerPool(maxSetPerPool)
, _poolFlags(poolFlags)
{
    assert(VKHANDLE_IS_NOT_NULL(_device));
    for (auto &&binding : setBindings)
    {
        auto itr = _poolSz.find(binding.descriptorType);
        if (itr == _poolSz.end())
            itr = _poolSz.insert(std::make_pair(binding.descriptorType, 0)).first;
        itr->second += binding.descriptorCount; // or itr->second++;
    }

    VkDescriptorSetLayoutCreateInfo setLayoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    setLayoutInfo.bindingCount = setBindings.size();
    setLayoutInfo.pBindings = setBindings.data();
    assert(VKCALL_SUCCESS(vkCreateDescriptorSetLayout(_device, &setLayoutInfo, nullptr, &_setLayout)));
}

DescriptorSetManager::AutoDescriptorPool::~AutoDescriptorPool()
{
    Release();
}

VkDescriptorSet DescriptorSetManager::AutoDescriptorPool::AllocDescriptorSet()
{

    VkDescriptorSetAllocateInfo allocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = _get_active_pool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &_setLayout;
    VkDescriptorSet allocSet{VK_NULL_HANDLE};
    if (vkAllocateDescriptorSets(_device, &allocInfo, &allocSet) == VK_ERROR_OUT_OF_DEVICE_MEMORY)
    {
        _activePoolIdx++;
        allocInfo.descriptorPool = _get_active_pool();
        VkResult code = vkAllocateDescriptorSets(_device, &allocInfo, &allocSet);
        if (code != VK_SUCCESS)
        {
            LOGE("Failed to alloc descriptor set, error: {}", code);
            throw std::runtime_error("Bad descriptor allocation!");
        }
        
    }

    return allocSet;
}


 bool DescriptorSetManager::AutoDescriptorPool::FreeDescriptorSet(VkDescriptorSet set)
 {
    if (!(_poolFlags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT))
        return false;
    
    if (_activePoolIdx < 0)
        return false;

    assert(VKCALL_SUCCESS(vkFreeDescriptorSets(_device, _get_active_pool(), 1, &set)));
    return true;
 }


void DescriptorSetManager::AutoDescriptorPool::Reset()
{
    while (_activePoolIdx >= 0)
    {
        assert(VKCALL_SUCCESS(vkResetDescriptorPool(_device, _pools[_activePoolIdx], 0)));
        _activePoolIdx--;
    }
    
}

void DescriptorSetManager::AutoDescriptorPool::Release()
{
    for (auto &&pool : _pools)
    {
        vkDestroyDescriptorPool(_device, pool, nullptr);   
    }
    _pools.clear();
    _activePoolIdx = -1;

}


 VkDescriptorPool DescriptorSetManager::AutoDescriptorPool::_get_active_pool()
 {
    if (_activePoolIdx < 0 || _activePoolIdx >= _pools.size())
    {
        _pools.push_back(_alloc_pool());
        _activePoolIdx = _pools.size() - 1;
    }

    return _pools[_activePoolIdx];
 }


 VkDescriptorPool DescriptorSetManager::AutoDescriptorPool::_alloc_pool()
 {
    std::vector<VkDescriptorPoolSize> poolSzs(_poolSz.size());
    size_t idx = 0;
    for (auto &&descriptor : _poolSz)
    {
        poolSzs[idx].type = descriptor.first;
        poolSzs[idx].descriptorCount = descriptor.second * _maxSetPerPool;
        idx++;
    }

    VkDescriptorPoolCreateInfo poolCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolCreateInfo.poolSizeCount = poolSzs.size();
    poolCreateInfo.pPoolSizes = poolSzs.data();
    poolCreateInfo.maxSets = _maxSetPerPool;
    poolCreateInfo.flags = _poolFlags;

    VkDescriptorPool createdPool{VK_NULL_HANDLE};
    VkResult result = vkCreateDescriptorPool(_device, &poolCreateInfo, nullptr, &createdPool);
    if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
        throw std::runtime_error("Descritor pool alloc failed -> device out of momery!");
    
    assert(VKCALL_SUCCESS(result));
    
    return createdPool;
 }


 std::array<std::unique_ptr<DescriptorSetManager::AutoDescriptorPool>, MaxSetIndex> DescriptorSetManager::s_CommonDescriptorPools{};
 std::unordered_map<size_t, std::unique_ptr<DescriptorSetManager::AutoDescriptorPool>> DescriptorSetManager::s_PerMaterialDescriptorPools{};

 void DescriptorSetManager::RegisterSetLayout(SetIndices setIdx, size_t setHash, const std::vector<VkDescriptorSetLayoutBinding>& setBindings, bool freeableSet, size_t maxSetPerPool)
 {
    auto pool = std::make_unique<AutoDescriptorPool>(
            s_Device->GetHandle(),
            setBindings,
            maxSetPerPool,
            freeableSet ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0);
    if (setIdx == PerMaterial)
    {
        if (s_PerMaterialDescriptorPools.find(setHash) != s_PerMaterialDescriptorPools.end())
        {
            LOGW("Rregister multiple setlayout to set index: {}, set hash: {}", setIdx, setHash);
        }
        s_PerMaterialDescriptorPools[setHash] = std::move(pool);
    }
    else 
    {
        if (s_CommonDescriptorPools[setIdx])
        {
            LOGW("Rregister multiple setlayout to set index: {}", setIdx);
        }
        s_CommonDescriptorPools[setIdx] = std::move(pool);
    }
 }


 VkDescriptorSetLayout DescriptorSetManager::GetSetLayout(SetIndices setIdx, size_t setHash)
 {
    if (setIdx == PerMaterial)
    {
        auto itr = s_PerMaterialDescriptorPools.find(setHash);
        if (itr == s_PerMaterialDescriptorPools.end())
            return VK_NULL_HANDLE;
        
        return itr->second->GetDescriptorSetLayout();
    }
    else 
    {
        return s_CommonDescriptorPools[setIdx] ? s_CommonDescriptorPools[setIdx]->GetDescriptorSetLayout() : VK_NULL_HANDLE;
    }
 }

 VkDescriptorSet DescriptorSetManager::AllocDescriptorSet(SetIndices setIdx, size_t providerHash)
 {

     return setIdx == PerMaterial ? s_PerMaterialDescriptorPools[providerHash]->AllocDescriptorSet()
                                  : s_CommonDescriptorPools[setIdx]->AllocDescriptorSet();
 }

 bool DescriptorSetManager::FreeDescriptorSet(SetIndices setIdx, size_t providerHash, VkDescriptorSet set)
 {
     return setIdx == PerMaterial ? s_PerMaterialDescriptorPools[providerHash]->FreeDescriptorSet(set)
                                  : s_CommonDescriptorPools[setIdx]->FreeDescriptorSet(set);
 }

 void DescriptorSetManager::ResetDescriptorPool(SetIndices setIdx, size_t providerHash)
 {
     setIdx == PerMaterial ? s_PerMaterialDescriptorPools[providerHash]->Reset()
                           : s_CommonDescriptorPools[setIdx]->Reset();
 }


void DescriptorSetManager::Initailize(Device* pdevice)
{
    assert(s_Device == nullptr);

    s_Device = pdevice;
    PerFrameData::Initailize(pdevice);
    PerCameraData::Initailize(pdevice);
    PerObjectData::Initailize(pdevice);
}
 

void DescriptorSetManager::DeInitailize()
{
    PerObjectData::DeInitailize(s_Device);
    PerCameraData::DeInitailize(s_Device);
    PerFrameData::DeInitailize(s_Device);

    for (auto &&pool : s_PerMaterialDescriptorPools)
    {
        pool.second.release();
    }
    
    for (auto &&pool : s_CommonDescriptorPools)
    {
        pool.release();
    }
    
}