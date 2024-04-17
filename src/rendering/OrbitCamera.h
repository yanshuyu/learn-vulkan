#pragma once
#include<glm\glm.hpp>
#include<rendering\RenderData.h>



class OrbitCamera
{
private:
    float _xyAngle; 
    float _xzAngle;

    glm::vec3 _center;
    glm::vec3 _right;
    float _radius;

    float _rotateSpeed;
    float _zoomSpeed;
    float _panSpeed;
    float _dumping;

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


