#include"Mesh.h"
#include"core\Fence.h"
#include"core\Buffer.h"
#include"core\Device.h"
#include"core\CommandBuffer.h"

Mesh::Mesh(Device* pDevice, bool readWriteEnable)
: _pDevice(pDevice)
, _readWriteEnable(readWriteEnable)
{
    assert(_pDevice != nullptr);
    _clear_attr_binding();    
}

Mesh::~Mesh()
{
    Release();
    _pDevice = nullptr;
}

void Mesh::SetVertices(const glm::vec3 *vertices, size_t cnt)
{
    if( _positions.size() < cnt)
        _positions.resize(cnt);

    memcpy(_positions.data(), vertices, sizeof(glm::vec3) * cnt);
    _set_attr_dirty(Position);
    _attrCnt[Position] = cnt;
}

void Mesh::SetNormals(const glm::vec3 *normals, size_t cnt)
{
    if (_normals.size() < cnt)
        _normals.resize(cnt);
    
    memcpy(_normals.data(), normals, sizeof(glm::vec3) * cnt);
    _set_attr_dirty(Normal);
    _attrCnt[Normal] = cnt;
}

void Mesh::SetTangents(const glm::vec3 *tangents, size_t cnt)
{
    if (_tangents.size() < cnt)
        _tangents.resize(cnt);
    
    memcpy(_tangents.data(), tangents, sizeof(glm::vec3) * cnt);
    _set_attr_dirty(Tangent);
    _attrCnt[Tangent] = cnt;
}

void Mesh::SetColors(const glm::vec4 *colors, size_t cnt)
{
    if (_colors.size() < cnt)
        _colors.resize(cnt);
    
    memcpy(_colors.data(), colors, sizeof(glm::vec4) * cnt);
    _set_attr_dirty(Color);
    _attrCnt[Color] = cnt;
}

void Mesh::SetUV1s(const glm::vec2 *uvs, size_t cnt)
{
    if (_uv0.size() < cnt)
        _uv0.resize(cnt);
    
    memcpy(_uv0.data(), uvs, sizeof(glm::vec2) * cnt);
    _set_attr_dirty(UV0);
    _attrCnt[UV0] = cnt;
}

void Mesh::SetUV2s(const glm::vec2 *uvs, size_t cnt)
{
    if (_uv1.size() < cnt)
        _uv1.resize(cnt);

    memcpy(_uv1.data(), uvs, sizeof(glm::vec2) * cnt);
    _set_attr_dirty(UV1);
    _attrCnt[UV1] = cnt;
}

void Mesh::SetIndices(const index_t *idxs, size_t cnt)
{
    if (_indices.size() < cnt)
        _indices.resize(cnt);
    
    memcpy(_indices.data(), idxs, sizeof(index_t) * cnt);
    _set_attr_dirty(MaxAttribute);
    _attrCnt[MaxAttribute] = cnt;
}


bool Mesh::Apply()
{
    if (!_pDevice || !_pDevice->IsValid())
        return false;

    if (!_attrDirty.any())
        return false;

    _clear_attr_binding();    

    CommandBuffer* cmd{nullptr};

    if (!_readWriteEnable)
    {
        cmd = _pDevice->CreateTempraryCommandBuffer(_pDevice->GetMainQueue());
        cmd->Begin();
    }

    // position
    if (_is_attr_dirty(Position))
    {
        size_t sz = _get_attr_byte_size(Position) * _positions.size();
        if (!_attrBuffers[Position])
            _gen_buffer(Position, sz);
        _update_buffer(cmd, Position, (uint8_t*)_positions.data(), sz);
        _append_attr_binding(Position);
    }

    // normal
    if (_is_attr_dirty(Normal))
    {
        size_t sz = _get_attr_byte_size(Normal) * _normals.size();
        if (!_attrBuffers[Normal])
            _gen_buffer(Normal, sz);
        _update_buffer(cmd, Normal, (uint8_t*)_normals.data(), sz);
        _append_attr_binding(Normal);
    }

    // tangent
    if (_is_attr_dirty(Tangent))
    {
        size_t sz = _get_attr_byte_size(Tangent) * _tangents.size();
        if (!_attrBuffers[Tangent])
           _gen_buffer(Tangent, sz);
        _update_buffer(cmd, Tangent, (uint8_t*)_tangents.data(), sz);
        _append_attr_binding(Tangent);
    }

    // color 
    if (_is_attr_dirty(Color))
    {
        size_t sz = _get_attr_byte_size(Color) * _colors.size();
        if (!_attrBuffers[Color])
           _gen_buffer(Color, sz);
        _update_buffer(cmd, Color, (uint8_t*)_colors.data(), sz);
        _append_attr_binding(Color);
    }

    // uv0 
    if (_is_attr_dirty(UV0))
    {
        size_t sz = _get_attr_byte_size(UV0) * _uv0.size();
        if (!_attrBuffers[UV0])
            _gen_buffer(UV0, sz);
        _update_buffer(cmd, UV0, (uint8_t*)_uv0.data(), sz);
        _append_attr_binding(UV0);
    }

    // uv1
    if (_is_attr_dirty(UV1))
    {
        size_t sz = _get_attr_byte_size(UV1) * _uv1.size();
        if (!_attrBuffers[UV1])
            _gen_buffer(UV1, sz);
        _update_buffer(cmd, UV1, (uint8_t*)_uv1.data(), sz);
        _append_attr_binding(UV1);
    }

    // index
    if (_is_attr_dirty(MaxAttribute))
    {
        size_t sz = _get_attr_byte_size(MaxAttribute) * _indices.size();
        if (!_attrBuffers[MaxAttribute])
            _gen_buffer(MaxAttribute, sz);
        _update_buffer(cmd, MaxAttribute, (uint8_t*)_indices.data(), sz); 
    }


    if (!_readWriteEnable)
    {
        cmd->End();
        Fence* f = _pDevice->CreateFence(false);
        cmd->ExecuteAsync(f);
        f->Wait();
        _pDevice->DestroyFence(f);
        _clear_staging_data();
        _clear_cpu_data();
    }

    _attrDirty.reset();

    return true;
}

void Release();



void Mesh::_gen_buffer(VertexAttribute attr, size_t size)
{
    bool isIndex = attr == MaxAttribute;
    VkBufferUsageFlags usage = isIndex ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    usage |= _readWriteEnable ? 0 : VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags memProp = _readWriteEnable ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    _attrBuffers[attr] = _pDevice->CreateBuffer(size, usage, memProp);
}

void Mesh::_gen_staging_data(VertexAttribute attr, uint8_t *data, size_t sz)
{
    assert(_stagingBuffers[attr] == nullptr);
    _stagingBuffers[attr] = _pDevice->CreateBuffer(sz, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uint8_t *mapedData = _stagingBuffers[attr]->Map(Write);
    memcpy(mapedData, data, sz);
    _stagingBuffers[attr]->UnMap();
}

void Mesh::_update_buffer(CommandBuffer *cmd, VertexAttribute attr, uint8_t *data, size_t size)
{
    bool isIndex = attr == MaxAttribute;
    Buffer *buf = _attrBuffers[attr];
    if (_readWriteEnable)
    {
        // host access directly
        uint8_t *mapedPtr = buf->Map(Write);
        memcpy(mapedPtr, data, size);
        buf->UnMap();
    }
    else
    {
        // transfer using staging buffer
        _gen_staging_data(attr, data, size);

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        VkAccessFlags waitOp = isIndex ? VK_ACCESS_INDEX_READ_BIT : VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        VkPipelineStageFlags signalStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        VkAccessFlags signalOp = isIndex ? VK_ACCESS_INDEX_READ_BIT : VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        cmd->CopyBuffer(_stagingBuffers[attr], 0, _attrBuffers[attr], 0, size, waitStage, waitOp, signalStage, signalOp);
    }
}

size_t Mesh::_get_attr_byte_size(VertexAttribute attr) const
{
    switch (attr)
    {
    case Position:
    case Normal:
    case Tangent:
        return sizeof(glm::vec3);

    case Color:
        return sizeof(glm::vec4);

    case UV0:
    case UV1:
        return sizeof(glm::vec2);

    case MaxAttribute:
        return sizeof(index_t);

    default:
        break;
    }

    return 0;
}


VkFormat Mesh::GetAttributeFormat(VertexAttribute attr) const
{
    switch (attr)
    {
    case Position:
    case Normal:
    case Tangent:
        return VK_FORMAT_R32G32B32_SFLOAT;

    case Color:
        return VK_FORMAT_R32G32B32A32_SFLOAT;

    case UV0:
    case UV1:
        return VK_FORMAT_R32G32_SFLOAT;

    default:
        break;
    }

    return VK_FORMAT_MAX_ENUM;
}


void Mesh::_clear_cpu_data()
{
    _positions.clear();
    _normals.clear();
    _tangents.clear();
    _colors.clear();
    _uv0.clear();
    _uv1.clear();
    _indices.clear();
}

void Mesh::_clear_gpu_data()
{
    for (size_t i = 0; i < MaxAttribute + 1; i++)
    {
        if (_attrBuffers[i] != nullptr)
        {
            _pDevice->DestroyBuffer(_attrBuffers[i]);
            _attrBuffers[i] = nullptr;
        }
        _attrCnt[i] = 0;
    }
}

void Mesh::_clear_staging_data()
{
    for (size_t i = 0; i < MaxAttribute + 1; i++)
    {
        if (_stagingBuffers[i] != nullptr)
        {
            _pDevice->DestroyBuffer(_stagingBuffers[i]);
            _stagingBuffers[i] = nullptr;
        }
    }
}


void Mesh::_clear_attr_binding()
{
    for (size_t i = 0; i < MaxAttribute; i++)
    {
        _attrBindingHandls[i] = VK_NULL_HANDLE;
        _attrBindings[i] = -1;
    }
    _attrBindingCnt = 0;
}


void Mesh::_append_attr_binding(VertexAttribute attr)
{
    _attrBindingHandls[_attrBindingCnt] = _attrBuffers[attr]->GetHandle();
    _attrBindings[attr] = _attrBindingCnt;
    _attrBindingCnt++;
}

void Mesh::Release()
{
    _clear_attr_binding();
    _clear_staging_data();
    _clear_gpu_data();
    _clear_cpu_data();
    _attrDirty.reset();
    _readWriteEnable = false;
}