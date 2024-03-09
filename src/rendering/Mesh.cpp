#include"Mesh.h"
#include"core\Buffer.h"
#include"core\Device.h"
#include"core\CommandBuffer.h"

Mesh::Mesh(Device* pDevice)
: _pDevice(pDevice)
{
    assert(_pDevice != nullptr);
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
    _attributeCnt[Position] = cnt;
}

void Mesh::SetNormals(const glm::vec3 *normals, size_t cnt)
{
    if (_normals.size() < cnt)
        _normals.resize(cnt);
    
    memcpy(_normals.data(), normals, sizeof(glm::vec3) * cnt);
    _set_attr_dirty(Normal);
    _attributeCnt[Normal] = cnt;
}

void Mesh::SetTangents(const glm::vec3 *tangents, size_t cnt)
{
    if (_tangents.size() < cnt)
        _tangents.resize(cnt);
    
    memcpy(_tangents.data(), tangents, sizeof(glm::vec3) * cnt);
    _set_attr_dirty(Tangent);
    _attributeCnt[Tangent] = cnt;
}

void Mesh::SetColors(const glm::vec4 *colors, size_t cnt)
{
    if (_colors.size() < cnt)
        _colors.resize(cnt);
    
    memcpy(_colors.data(), colors, sizeof(glm::vec4) * cnt);
    _set_attr_dirty(Color);
    _attributeCnt[Color] = cnt;
}

void Mesh::SetUV1s(const glm::vec2 *uvs, size_t cnt)
{
    if (_uv0.size() < cnt)
        _uv0.resize(cnt);
    
    memcpy(_uv0.data(), uvs, sizeof(glm::vec2) * cnt);
    _set_attr_dirty(UV0);
    _attributeCnt[UV0] = cnt;
}

void Mesh::SetUV2s(const glm::vec2 *uvs, size_t cnt)
{
    if (_uv1.size() < cnt)
        _uv1.resize(cnt);

    memcpy(_uv1.data(), uvs, sizeof(glm::vec2) * cnt);
    _set_attr_dirty(UV1);
    _attributeCnt[UV1] = cnt;
}

void Mesh::SetIndices(const uint32_t *idxs, size_t cnt)
{
    if (_indices.size() < cnt)
        _indices.resize(cnt);
    
    memcpy(_indices.data(), idxs, sizeof(uint32_t) * cnt);
    _set_attr_dirty(MaxAttribute);
    _attributeCnt[MaxAttribute] = cnt;
}


bool Mesh::Apply()
{
    if (!_pDevice || _pDevice->IsValid())
        return false;

    if (!_attributeDirty.any())
        return false;

    CommandBuffer *cmd{nullptr};
    if (!_readWriteEnable)
    {
        _pDevice->CreateTempraryCommandBuffer(_pDevice->GetTransferQueue());
        cmd->Begin();
    }

    // position
    if (_is_attr_dirty(Position))
    {
        if (!_gpuBuffers[Position])
            _gpuBuffers[Position] = _gen_buffer(Position, _get_attr_byte_size(Position) * _positions.size());
        _update_buffer(cmd, Position, (uint8_t*)_positions.data(), _get_attr_byte_size(Position) * _positions.size());
        if (!_readWriteEnable)
            _positions.clear();
        _unset_attr_dirty(Position);
    }

    // normal
    if (_is_attr_dirty(Normal))
    {
        if (!_gpuBuffers[Normal])
            _gpuBuffers[Normal] = _gen_buffer(Normal, _get_attr_byte_size(Normal) * _normals.size());
        _update_buffer(cmd, Normal, (uint8_t*)_normals.data(), _get_attr_byte_size(Normal) * _normals.size());
        if (!_readWriteEnable)
            _normals.clear();
        _unset_attr_dirty(Normal);
    }

    // tangent
    if (_is_attr_dirty(Tangent))
    {
        if (!_gpuBuffers[Tangent])
            _gpuBuffers[Tangent] = _gen_buffer(Tangent, _get_attr_byte_size(Tangent) * _tangents.size());
        _update_buffer(cmd, Tangent, (uint8_t*)_tangents.data(), _get_attr_byte_size(Tangent) * _tangents.size());
        if (!_readWriteEnable)
            _tangents.clear();
        _unset_attr_dirty(Tangent);
    }

    // color 
    if (_is_attr_dirty(Color))
    {
        if (!_gpuBuffers[Color])
            _gpuBuffers[Color] = _gen_buffer(Color, _get_attr_byte_size(Color) * _colors.size());
        _update_buffer(cmd, Color, (uint8_t*)_colors.data(), _get_attr_byte_size(Color) * _colors.size());
        if (!_readWriteEnable)
            _colors.clear();
        _unset_attr_dirty(Color);  
    }

    // uv0 
    if (_is_attr_dirty(UV0))
    {
        if (!_gpuBuffers[UV0])
            _gpuBuffers[UV0] = _gen_buffer(UV0, _get_attr_byte_size(UV0) * _uv0.size());
        _update_buffer(cmd, UV0, (uint8_t*)_uv0.data(), _get_attr_byte_size(UV0) * _uv0.size());
        if (!_readWriteEnable)
            _uv0.clear();
        _unset_attr_dirty(UV0);  
    }

    // uv1
    if (_is_attr_dirty(UV1))
    {
        if (!_gpuBuffers[UV1])
            _gpuBuffers[UV1] = _gen_buffer(UV1, _get_attr_byte_size(UV1) * _uv1.size());
        _update_buffer(cmd, UV1, (uint8_t*)_uv1.data(), _get_attr_byte_size(UV1) * _uv1.size());
        if (!_readWriteEnable)
            _uv1.clear();
        _unset_attr_dirty(UV1); 
    }

    // index
    if (_is_attr_dirty(MaxAttribute))
    {
        if (!_gpuBuffers[MaxAttribute])
            _gpuBuffers[MaxAttribute] = _gen_buffer(MaxAttribute, _get_attr_byte_size(MaxAttribute) * _indices.size());
        _update_buffer(cmd, MaxAttribute, (uint8_t*)_indices.data(), _get_attr_byte_size(MaxAttribute) * _indices.size());
        if (!_readWriteEnable)
            _indices.clear();
        _unset_attr_dirty(MaxAttribute);  
    }

    if (cmd)
        cmd->ExecuteAsync(nullptr);

    return true;
}

void Release();



Buffer* Mesh::_gen_buffer(Attribute attr, size_t size)
{
    bool isIndex = attr == MaxAttribute;
    VkBufferUsageFlags usage = isIndex ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    usage |= _readWriteEnable ? 0 : VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags memProp = _readWriteEnable ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    return _pDevice->CreateBuffer(size, usage, memProp);
}

void Mesh::_update_buffer(CommandBuffer *cmd, Attribute attr, uint8_t *data, size_t size)
{
    bool isIndex = attr == MaxAttribute;
    Buffer *buf = _gpuBuffers[attr];
    if (_readWriteEnable)
    {
        uint8_t *mapedPtr = buf->Map();
        memcpy(mapedPtr, data, size);
        buf->UnMap();
    }
    else
    {
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        VkAccessFlags waitOp = isIndex ? VK_ACCESS_INDEX_READ_BIT : VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        VkPipelineStageFlags signalStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        VkAccessFlags signalOp = isIndex ? VK_ACCESS_INDEX_READ_BIT : VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        cmd->UpdateBuffer(buf, 0, data, 0, size, waitStage, waitOp, signalStage, signalOp);
    }
}

size_t Mesh::_get_attr_byte_size(Attribute attr)
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


void Mesh::Release()
{

    for (size_t i = 0; i < MaxAttribute + 1; i++)
    {
        if (_gpuBuffers[i] != nullptr)
        {
            _pDevice->DestroyBuffer(_gpuBuffers[i]);
            _gpuBuffers[i] = nullptr;
        }
        _attributeCnt[i] = 0;
    }

    _positions.clear();
    _normals.clear();
    _tangents.clear();
    _colors.clear();
    _uv0.clear();
    _uv1.clear();
    _indices.clear();

    _attributeDirty.reset();
    _readWriteEnable = false;
}