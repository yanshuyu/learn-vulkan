#include"InputManager.h"


int InputManager::s_keyActions[GLFW_KEY_LAST]{};
int InputManager::s_MouseActions[GLFW_MOUSE_BUTTON_LAST]{};
double InputManager::s_CursorPosX = 0;
double InputManager::s_CursorPosY = 0;
double InputManager::s_mouseSrcoll = 0;

void InputManager::Reset()
{
    for (size_t i = 0; i < GLFW_KEY_LAST; i++)
    {   
        s_keyActions[i] = -1;
    }
    
    for (size_t i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
    {
        s_MouseActions[i] = -1;
    }
    
}