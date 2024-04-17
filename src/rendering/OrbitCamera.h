#pragma once
#include<glm\glm.hpp>
#include<rendering\RenderData.h>



class OrbitCamera
{
private:
    float _xyAngle{0.f}; 
    float _xzAngle{4.712385f};

    glm::vec3 _center{0};
    glm::vec3 _right{1.f, 0.f, 0.f};
    float _radius{5};

    float _rotateSpeed{10.f};
    float _zoomSpeed{10.f};
    float _panSpeed{10.f};
    float _dumping{0.f};

    glm::mat4 _viewMatrix{1.f};

    double _lastMousePosX{0};
    double _lastMousePosY{0};
    glm::vec3 _camPos{0.f};

    bool _rotate{false};
    bool _zoom{false};
    bool _pan{false};


private:
    glm::vec3 _spherical_to_cartesian(float alpha, float beta);
    void _update_cam_pos();

public:
    OrbitCamera();
    ~OrbitCamera() = default;

    glm::mat4 GetViewMatrix() const { return _viewMatrix; }
    void Reset();
    void Update(float dt);

};


