#pragma once
#include<glm\glm.hpp>
#include<vector>
#include<bitset>
#include"core\CoreUtils.h"

class Buffer;
class Device;

class Mesh
{
public:
    enum Attribute 
    {
        Position,
        Normal,
        Tangent,
        Color,
        UV0,
        UV1,
        MaxAttribute,
    };

private:
    std::vector<glm::vec3> _positions{};
    std::vector<glm::vec3> _normals{};
    std::vector<glm::vec3> _tangents{};
    std::vector<glm::vec4> _colors{};
    std::vector<glm::vec2> _uv0{};
    std::vector<glm::vec2> _uv1{};
    std::vector<uint32_t> _indices{};
    VkPrimitiveTopology _topology{VK_PRIMITIVE_TOPOLOGY_MAX_ENUM};

    bool _readWriteEnable{false};

    Device* _pDevice;
    Buffer* _gpuBuffers[MaxAttribute + 1] {};

    size_t _attributeCnt[MaxAttribute + 1] {};
    std::bitset<MaxAttribute+1> _attributeDirty{0};
private:
    void _set_attribute_dirty(Attribute attr) { _attributeDirty.set(attr, true); }
    void _unset_attribute_dirty(Attribute attr) { _attributeDirty.set(attr, false); }
    bool _is_attribute_dirty(Attribute attr) { return _attributeDirty.test(attr); }
    Buffer* _gen_attribute_buffer(Attribute attr, size_t size);
    void _update_buffer(Buffer* buf, void* data, size_t size);
public:
    Mesh(Device* pDevice);
    ~Mesh();

    void SetVertices(const glm::vec3* vertices, size_t cnt);
    void SetNormals(const glm::vec3* normals, size_t cnt);
    void SetTangents(const glm::vec3* tangents, size_t cnt);
    void SetColors(const glm::vec4* colors, size_t cnt);
    void SetUV1s(const glm::vec2* uvs, size_t cnt);
    void SetUV2s(const glm::vec2* uvs, size_t cnt);
    void SetIndices(const uint32_t* idxs, size_t cnt);
    void SetTopology(VkPrimitiveTopology pt) { _topology = pt; }
    void SetReadWriteEnable(bool enable) { _readWriteEnable = enable; };

    bool Apply();
    void Release();

    bool HasVertices() const { return _attributeCnt[Position] > 0; }
    bool HasNormals() const { return _attributeCnt[Normal] > 0; }
    bool HasTangents() const { return _attributeCnt[Tangent] > 0; }
    bool HasColors() const { return _attributeCnt[Color] > 0; }
    bool HasUV1s() const { return _attributeCnt[UV0] > 0; }
    bool hasUV2s() const { return _attributeCnt[UV1] > 0; } 

    size_t GetVerticesCount() const { return _attributeCnt[Position]; }
    size_t GetIndicesCount() const { return _attributeCnt[MaxAttribute]; }
    VkPrimitiveTopology GetTopology() const { return _topology; }
    bool GetReadWriteEnable() const {return _readWriteEnable; }
    const std::vector<glm::vec3>& GetVertices() const { return _positions; }
    const std::vector<glm::vec3>& GetNormals() const { return _normals; }
    const std::vector<glm::vec3>& GetTangents() const { return _tangents; }
    const std::vector<glm::vec4>& GetColors() const { return _colors; }
    const std::vector<glm::vec2>& GetUV1s() const { return _uv0; }
    const std::vector<glm::vec2>& GetUV2s() const { return _uv1; }
    const std::vector<uint32_t>& GetIndices() const { return _indices; }

    Buffer* GetGPUBuffer(Attribute attr) const { return _gpuBuffers[attr]; }
};

