#include"OrbitCamera.h"
#include<input\InputManager.h>
#include<glm\gtc\matrix_transform.hpp>
#include"GameTimer.h"

#define HAFL_PI 1.570796f

OrbitCamera::OrbitCamera(/* args */)
{   
    Reset();
}


void OrbitCamera::Reset()
{
    _xyAngle = 0.f;
    _xzAngle = HAFL_PI;
    _center = glm::vec3(0);
    _radius = 5.f;
    _rotateSpeed = _zoomSpeed = _panSpeed = 10.f;
    _right = glm::vec3(1, 0, 0);
    _lastMousePosX = _lastMousePosY = 0;
    _update_cam_pos();
}


void OrbitCamera::Update()
{
    float dt = GameTimer::GetDeltaTime();
    double mousePosX, mousePosY, detalX, detalY;

    if (InputManager::GetKeyDown(GLFW_KEY_F))
        Reset();

    if (InputManager::GetMouseDown(GLFW_MOUSE_BUTTON_LEFT))
    {    
        _lastMousePosX = InputManager::GetCursorPosX();
        _lastMousePosY = InputManager::GetCursorPosY();
        _rotate = true;
    }
    if (InputManager::GetMouseDown(GLFW_MOUSE_BUTTON_RIGHT))
    {   
        _lastMousePosX = InputManager::GetCursorPosX();
        _lastMousePosY = InputManager::GetCursorPosY(); 
        _zoom = true;
        _dumping = 0;
    }
    if (InputManager::GetMouseDown(GLFW_MOUSE_BUTTON_MIDDLE))
    {
        _lastMousePosX = InputManager::GetCursorPosX();
        _lastMousePosY = InputManager::GetCursorPosY();
        _pan = true; 
    }

    if (InputManager::GetMouseUp(GLFW_MOUSE_BUTTON_LEFT))
        _rotate = false;
    if (InputManager::GetMouseUp(GLFW_MOUSE_BUTTON_RIGHT))
        _zoom = false;
    if (InputManager::GetMouseUp(GLFW_MOUSE_BUTTON_MIDDLE))
        _pan = false;
    if (InputManager::GetKeyUp(GLFW_KEY_W)
        || InputManager::GetKeyUp(GLFW_KEY_S))
        _dumping = 0;

    if (_rotate || _zoom || _pan)
    {
        mousePosX = InputManager::GetCursorPosX();
        mousePosY = InputManager::GetCursorPosY();
        detalX = mousePosX - _lastMousePosX;
        detalY = mousePosY - _lastMousePosY;
        _lastMousePosX = mousePosX;
        _lastMousePosY = mousePosY;
    }

    if (_rotate)
    {
        _xzAngle += detalX * _rotateSpeed * dt;
        _xyAngle += detalY * _rotateSpeed * dt;
    }
    if (_zoom)
    {
        _radius -= detalX * _zoomSpeed * dt;
        // fly
        if (InputManager::GetKeyPress(GLFW_KEY_W))
        {
            _dumping += dt;
            _dumping = std::clamp(_dumping, 0.f, 1.f);
            glm::vec3 forward = glm::normalize(_center - _camPos);
            _center += forward * _dumping * _panSpeed;
        }
        else if (InputManager::GetKeyPress(GLFW_KEY_S))
        {
            _dumping += dt;
            _dumping = std::clamp(_dumping, 0.f, 1.f);
            glm::vec3 forward = glm::normalize(_center - _camPos);
            _center -= forward * _dumping * _panSpeed; 
        }
        else if (InputManager::GetKeyPress(GLFW_KEY_A))
        {
            _dumping += dt;
            _dumping = std::clamp(_dumping, 0.f, 1.f);
            _center += _right * _dumping * _panSpeed;
        }
        else if (InputManager::GetKeyPress(GLFW_KEY_D))
        {
            _dumping += dt;
            _dumping = std::clamp(_dumping, 0.f, 1.f);
            _center -= _right * _dumping * _panSpeed; 
        }
            
    }
    if (_pan)
    {
        _center += _right * (float)detalX * _panSpeed * dt;
        _center += glm::vec3(0, 1, 0) * (float)detalY * _panSpeed * dt;
    }


    if (_rotate || _zoom || _pan)
        _update_cam_pos();

}


glm::vec3 OrbitCamera::_spherical_to_cartesian(float alpha, float beta)
{
    float sinXY = glm::sin(beta);
    float cosXY = glm::cos(beta);
    float sinXZ = glm::sin(alpha);
    float cosXZ = glm::cos(alpha);
    glm::vec3 P{0};
    P.x = cosXZ * cosXY * _radius;
    P.z = sinXZ * cosXY * _radius;
    P.y = sinXY * _radius;
    P += _center;
    return P;
}

void OrbitCamera::_update_cam_pos()
{
    _camPos = _spherical_to_cartesian(_xzAngle, _xyAngle);
    glm::vec3 offsetPos = _spherical_to_cartesian(_xzAngle + 0.1f, _xyAngle);
    _right = glm::normalize(offsetPos - _camPos);
    glm::vec3 forward = glm::normalize(_center - _camPos);
    _viewMatrix = glm::lookAt(_camPos, _center, glm::cross(forward, _right));
}
