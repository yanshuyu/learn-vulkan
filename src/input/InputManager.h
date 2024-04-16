#pragma once
#include<GLFW\glfw3.h>


class InputManager
{
private:
    static int s_keyActions[GLFW_KEY_LAST];
    static int s_MouseActions[GLFW_MOUSE_BUTTON_LAST];
    static double s_CursorPosX;
    static double s_CursorPosY;
    static double s_mouseSrcoll;

public:
    InputManager() = delete;
    ~InputManager() = delete;
    
    static void Reset();
    
    static bool GetKeyDown(int key) { return s_keyActions[key] == GLFW_PRESS; }
    static bool GetKeyUp(int key) { return s_keyActions[key] == GLFW_RELEASE; }
    static bool GetKeyPress(int key) { return s_keyActions[key] == GLFW_REPEAT; }
    static void EnjectKey(int key, int action) { if (key != GLFW_KEY_UNKNOWN) s_keyActions[key] = action; }

    static bool GetMouseDown(int mouseBtn) { return s_MouseActions[mouseBtn] == GLFW_PRESS; }
    static bool GetMouseUp(int mouseBtn) { return s_MouseActions[mouseBtn] == GLFW_RELEASE; }
    static bool GetMousePress(int mouseBtn) { return s_MouseActions[mouseBtn] == GLFW_REPEAT; }
    static void EnjectMouse(int mouseBtn, int action) { s_MouseActions[mouseBtn] = action; }
    
    static void EnJectCursorPos(double x, double y) { s_CursorPosX = x; s_CursorPosY = y; }
    static double GetCursorPosX() { return s_CursorPosX; }
    static double GetCursorPosY() { return s_CursorPosY; } 
    static void ResetCursorPos() { s_CursorPosX = s_CursorPosY = 0; }

    static void EnjectMouseScroll(double offset) { s_mouseSrcoll = offset; }
    static double GetMouseScroll() { return s_mouseSrcoll; }
};


