#pragma once
#include<glm\glm.hpp>
#include<vector>
#include<bitset>
#include<string>
#include"core\CoreUtils.h"

class Buffer;
class CommandBuffer;
class Device;


class Mesh
{

private:
    std::vector<glm::vec3> _positions{};
    std::vector<glm::vec3> _normals{};
    std::vector<glm::vec3> _tangents{};
    std::vector<glm::vec4> _colors{};
    std::vector<glm::vec2> _uv0{};
    std::vector<glm::vec2> _uv1{};
    std::vector<index_t> _indices{};
    VkPrimitiveTopology _topology{VK_PRIMITIVE_TOPOLOGY_MAX_ENUM};

    bool _readWriteEnable{false};

    Device* _pDevice;
    Buffer* _attrBuffers[MaxAttribute + 1] {};
    Buffer* _stagingBuffers[MaxAttribute + 1] {};
    size_t _attrCnt[MaxAttribute + 1] {};
    std::bitset<MaxAttribute+1> _attrDirty{0};

    std::array<VkBuffer, MaxAttribute> _attrBindingHandls{};
    std::array<int, MaxAttribute> _attrBindings{};
    size_t _attrBindingCnt{0};

    std::string _name{};

private:
    void _set_attr_dirty(VertexAttribute attr) { _attrDirty.set(attr, true); }
    void _unset_attr_dirty(VertexAttribute attr) { _attrDirty.set(attr, false); }
    bool _is_attr_dirty(VertexAttribute attr) { return _attrDirty.test(attr); }
    size_t _get_attr_byte_size(VertexAttribute attr) const;
    void _gen_buffer(VertexAttribute attr, size_t size);
    void _gen_staging_data(VertexAttribute attr, uint8_t* data, size_t sz);
    void _update_buffer(CommandBuffer* cmd, VertexAttribute attr, uint8_t* data, size_t size);

    void _clear_cpu_data();
    void _clear_gpu_data();
    void _clear_staging_data();
    void _clear_attr_binding();
    void _append_attr_binding(VertexAttribute attr);
public:
    Mesh(Device* pDevice, bool readWriteEnable = false);
    ~Mesh();

    NONE_COPYABLE_NONE_MOVEABLE(Mesh)

    void SetName(const char* name) { _name = name; }
    const char* GetName() const { return _name.c_str(); }

    void SetVertices(const glm::vec3* vertices, size_t cnt);
    void SetNormals(const glm::vec3* normals, size_t cnt);
    void SetTangents(const glm::vec3* tangents, size_t cnt);
    void SetColors(const glm::vec4* colors, size_t cnt);
    void SetUV1s(const glm::vec2* uvs, size_t cnt);
    void SetUV2s(const glm::vec2* uvs, size_t cnt);
    void SetIndices(const index_t* idxs, size_t cnt);
    void SetTopology(VkPrimitiveTopology pt) { _topology = pt; }
    //void SetReadWriteEnable(bool enable) { _readWriteEnable = enable; };

    bool Apply();
    void Release();
    bool IsValid() const { return _attrBindingCnt > 0; }

    bool HasVertices() const { return _attrCnt[Position] > 0; }
    bool HasNormals() const { return _attrCnt[Normal] > 0; }
    bool HasTangents() const { return _attrCnt[Tangent] > 0; }
    bool HasColors() const { return _attrCnt[Color] > 0; }
    bool HasUV1s() const { return _attrCnt[UV0] > 0; }
    bool hasUV2s() const { return _attrCnt[UV1] > 0; }
    bool HasAttribute(VertexAttribute attr) const { return _attrCnt[attr] > 0; } 

    size_t GetVerticesCount() const { return _attrCnt[Position]; }
    size_t GetIndicesCount() const { return _attrCnt[MaxAttribute]; }
    VkPrimitiveTopology GetTopology() const { return _topology; }
    bool GetReadWriteEnable() const {return _readWriteEnable; }
    const std::vector<glm::vec3>& GetVertices() const { return _positions; }
    const std::vector<glm::vec3>& GetNormals() const { return _normals; }
    const std::vector<glm::vec3>& GetTangents() const { return _tangents; }
    const std::vector<glm::vec4>& GetColors() const { return _colors; }
    const std::vector<glm::vec2>& GetUV1s() const { return _uv0; }
    const std::vector<glm::vec2>& GetUV2s() const { return _uv1; }
    const std::vector<uint32_t>& GetIndices() const { return _indices; }

    Buffer* GetAttributeBuffer(VertexAttribute attr) const { return _attrBuffers[attr]; }
    Buffer* GetIndexBuffer() const { return _attrBuffers[MaxAttribute]; }
    size_t GetAttributeStride(VertexAttribute attr) const { return _get_attr_byte_size(attr); }

    size_t GetAttributeCount() const { return _attrBindingCnt; }
    int GetAttributeBinding(VertexAttribute attr) const { return _attrBindings[attr]; }
    const VkBuffer* GetAttributeBindingHandls() const { return _attrBindingHandls.data(); }

    VkFormat GetAttributeFormat(VertexAttribute attr) const;
    VkIndexType GetIndexType() const { return VK_INDEX_TYPE_UINT32; }

    Device* GetDevice() const { return _pDevice; }   
};

