#include"DescriptorManager.h"
#include"core\ShaderProgram.h"
#include"core\Device.h"

DescriptorManager::ProgramDescriptorPool::ProgramDescriptorPool(ShaderProgram* program, size_t maxSetSz)
: _program(program)
, _maxSetPerPool(maxSetSz)
{
    assert(program->IsValid());
    for (auto &&setIdx : program->GetResourceSetIndices())
    {
        for (auto &&setLayoutBindings : program->GetResourceSetLayoutBindins(setIdx))
        {   
            auto itr = _poolSz.find(setLayoutBindings.descriptorType);
            if (itr == _poolSz.end())
                itr = _poolSz.insert(std::make_pair(setLayoutBindings.descriptorType, 0)).first;
            itr->second += setLayoutBindings.descriptorCount;
        }   
    }

}

DescriptorManager::ProgramDescriptorPool::~ProgramDescriptorPool()
{
    Release();
}

bool DescriptorManager::ProgramDescriptorPool::AllocateProgramDescriptorSet(std::vector<VkDescriptorSet>& sets)
{
    size_t setCnt = _program->GetResourceSetLayoutCount();
    sets.resize(setCnt, VK_NULL_HANDLE);
    VkDescriptorSetAllocateInfo allocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = _get_active_pool();
    allocInfo.descriptorSetCount = setCnt;
    allocInfo.pSetLayouts = _program->GetResourceSetLayouts().data();
    VkResult result = vkAllocateDescriptorSets(_program->GetDevice()->GetHandle(), &allocInfo, sets.data());
    if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
    {
        _activePoolIdx++;
        allocInfo.descriptorPool = _get_active_pool();
        result = vkAllocateDescriptorSets(_program->GetDevice()->GetHandle(), &allocInfo, sets.data());
    }

    return result == VK_SUCCESS;
}

void DescriptorManager::ProgramDescriptorPool::Reset()
{
    while (_activePoolIdx >= 0)
    {
        assert(VKCALL_SUCCESS(vkResetDescriptorPool(_program->GetDevice()->GetHandle(), _pools[_activePoolIdx], 0)));
        _activePoolIdx--;
    }
    
}

void DescriptorManager::ProgramDescriptorPool::Release()
{
    for (auto &&pool : _pools)
    {
        vkDestroyDescriptorPool(_program->GetDevice()->GetHandle(), pool, nullptr);   
    }
    _pools.clear();
    _activePoolIdx = -1;

}


 VkDescriptorPool DescriptorManager::ProgramDescriptorPool::_get_active_pool()
 {
    if (_activePoolIdx < 0 || _activePoolIdx >= _pools.size())
    {
        _pools.push_back(_alloc_pool());
        _activePoolIdx = _pools.size() - 1;
    }

    return _pools[_activePoolIdx];
 }


 VkDescriptorPool DescriptorManager::ProgramDescriptorPool::_alloc_pool()
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

    VkDescriptorPool createdPool{VK_NULL_HANDLE};
    VkResult result = vkCreateDescriptorPool(_program->GetDevice()->GetHandle(), &poolCreateInfo, nullptr, &createdPool);
    if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
        throw std::runtime_error("Descritor pool alloc failed -> device out of momery!");
    
    assert(VKCALL_SUCCESS(result));
    
    return createdPool;
 }


 std::unordered_map<ShaderProgram*, size_t> DescriptorManager::s_poolSzHints{};
 std::unordered_map<ShaderProgram*, std::unique_ptr<DescriptorManager::ProgramDescriptorPool>> DescriptorManager::s_pools{};

size_t DescriptorManager::GetProgramDescriptorPoolSizeHint(ShaderProgram* program)
{
    auto itr = s_poolSzHints.find(program);
    if (itr != s_poolSzHints.end())
        return itr->second;
    
    return 0;
}


bool DescriptorManager::AllocProgramDescriptorSet(ShaderProgram* program, std::vector<VkDescriptorSet>& sets)
{
    auto itr = s_pools.find(program);
    if (itr == s_pools.end())
    {
        size_t poolSz = GetProgramDescriptorPoolSizeHint(program);
        itr = s_pools.insert(std::make_pair(program, poolSz > 0
                                                         ? new DescriptorManager::ProgramDescriptorPool(program, poolSz)
                                                         : new DescriptorManager::ProgramDescriptorPool(program))).first;
    }

    return itr->second->AllocateProgramDescriptorSet(sets);
}


void DescriptorManager::ResetProgramDescriptorSets(ShaderProgram* program)
{
    auto itr = s_pools.find(program);
    if (itr != s_pools.end())
        itr->second->Reset();
}


void DescriptorManager::ReleaseProgramDescriptorSets(ShaderProgram* program)
{
    auto itr = s_pools.find(program);
    if (itr != s_pools.end())
        itr->second->Release();
}


void DescriptorManager::Release()
{
    for (auto &&pool : s_pools)
    {
        pool.second->Release();
    }
    
    s_pools.clear();
}