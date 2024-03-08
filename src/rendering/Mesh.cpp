#include"Mesh.h"
#include"core\Device.h"

Mesh::Mesh(Device* pDevice)
: _pDevice(pDevice)
{
}

Mesh::~Mesh()
{
}

void Mesh::SetVertices(const glm::vec3 *vertices, size_t cnt)
{
    if( _positions.size() < cnt)
        _positions.resize(cnt);

    memcpy(_positions.data(), vertices, sizeof(glm::vec3) * cnt);
    _set_attribute_dirty(Position);
    _attributeCnt[Position] = cnt;
}

void Mesh::SetNormals(const glm::vec3 *normals, size_t cnt)
{
    if (_normals.size() < cnt)
        _normals.resize(cnt);
    
    memcpy(_normals.data(), normals, sizeof(glm::vec3) * cnt);
    _set_attribute_dirty(Normal);
    _attributeCnt[Normal] = cnt;
}

void Mesh::SetTangents(const glm::vec3 *tangents, size_t cnt)
{
    if (_tangents.size() < cnt)
        _tangents.resize(cnt);
    
    memcpy(_tangents.data(), tangents, sizeof(glm::vec3) * cnt);
    _set_attribute_dirty(Tangent);
    _attributeCnt[Tangent] = cnt;
}

void Mesh::SetColors(const glm::vec4 *colors, size_t cnt)
{
    if (_colors.size() < cnt)
        _colors.resize(cnt);
    
    memcpy(_colors.data(), colors, sizeof(glm::vec4) * cnt);
    _set_attribute_dirty(Color);
    _attributeCnt[Color] = cnt;
}

void Mesh::SetUV1s(const glm::vec2 *uvs, size_t cnt)
{
    if (_uv0.size() < cnt)
        _uv0.resize(cnt);
    
    memcpy(_uv0.data(), uvs, sizeof(glm::vec2) * cnt);
    _set_attribute_dirty(UV0);
    _attributeCnt[UV0] = cnt;
}

void Mesh::SetUV2s(const glm::vec2 *uvs, size_t cnt)
{
    if (_uv1.size() < cnt)
        _uv1.resize(cnt);

    memcpy(_uv1.data(), uvs, sizeof(glm::vec2) * cnt);
    _set_attribute_dirty(UV1);
    _attributeCnt[UV1] = cnt;
}

void Mesh::SetIndices(const uint32_t *idxs, size_t cnt)
{
    if (_indices.size() < cnt)
        _indices.resize(cnt);
    
    memcpy(_indices.data(), idxs, sizeof(uint32_t) * cnt);
    _set_attribute_dirty(MaxAttribute);
    _attributeCnt[MaxAttribute] = cnt;
}


bool Mesh::Apply()
{
    if (!_pDevice || _pDevice->IsValid())
        return false;

    if (!_attributeDirty.any())
        return false;
    
    // position
    if (_is_attribute_dirty(Position))
    {
        if (!_gpuBuffers[Position])
            _gpuBuffers[Position] = _gen_attribute_buffer(Position, sizeof(glm::vec3) * _positions.size());
        
        _update_buffer(_gpuBuffers[Position], _positions.data(), sizeof(glm::vec3) * _positions.size());

        if (!_readWriteEnable)
            _positions.clear();
    }

    // normal
    if (_is_attribute_dirty(Normal))
    {
    }

    // tangent
    if (_is_attribute_dirty(Tangent))
    {

    }

    // color 
    if (_is_attribute_dirty(Color))
    {

    }

    if (_is_attribute_dirty())


    return true;
}

void Release();



Buffer* Mesh::_gen_attribute_buffer(Attribute attr, size_t size)
{
    
}

