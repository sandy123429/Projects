#pragma once

// glow BEFORE glfw (due to ogl conflicts)
#include <glow/gl.hh>
#include <GLFW/glfw3.h>
#include <AntTweakBar.h>

/// Translates a GLFW mouse button to an AntTweakBar one
inline TwMouseButtonID glfw2atbMouseButton(int btn)
{
    switch (btn)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        return TW_MOUSE_LEFT;
    case GLFW_MOUSE_BUTTON_RIGHT:
        return TW_MOUSE_RIGHT;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        return TW_MOUSE_MIDDLE;
    default:
        return (TwMouseButtonID)-1;
    }
}

/// Translates a GLFW mouse action to an AntTweakBar one
inline TwMouseAction glfw2atbMouseAction(int action)
{
    switch (action)
    {
    case GLFW_PRESS:
        return TW_MOUSE_PRESSED;
    case GLFW_RELEASE:
        return TW_MOUSE_RELEASED;
    default:
        return (TwMouseAction)-1;
    }
}

/// Translates GLFW key modifiers to AntTweakBar ones
inline TwKeyModifier glfw2atbKeyMod(int mods)
{
    int m = TW_KMOD_NONE;
    if (mods & GLFW_MOD_ALT)
        m |= TW_KMOD_ALT;
    if (mods & GLFW_MOD_CONTROL)
        m |= TW_KMOD_CTRL;
    if (mods & GLFW_MOD_SHIFT)
        m |= TW_KMOD_SHIFT;
    if (mods & GLFW_MOD_SUPER)
        m |= TW_KMOD_META;
    return (TwKeyModifier)m;
}

/// Translates GLFW keys to AntTweakBar ones
inline int glfw2atbKey(int key, int scancode)
{
    switch (key)
    {
    case GLFW_KEY_BACKSPACE:
        return TW_KEY_BACKSPACE;
    case GLFW_KEY_TAB:
        return TW_KEY_TAB;
    case GLFW_KEY_NUM_LOCK:
        return TW_KEY_CLEAR;
    case GLFW_KEY_ENTER:
        return TW_KEY_RETURN;
    case GLFW_KEY_PAUSE:
        return TW_KEY_PAUSE;
    case GLFW_KEY_ESCAPE:
        return TW_KEY_ESCAPE;
    case GLFW_KEY_SPACE:
        return TW_KEY_SPACE;
    case GLFW_KEY_DELETE:
        return TW_KEY_DELETE;
    case GLFW_KEY_UP:
        return TW_KEY_UP;
    case GLFW_KEY_DOWN:
        return TW_KEY_DOWN;
    case GLFW_KEY_RIGHT:
        return TW_KEY_RIGHT;
    case GLFW_KEY_LEFT:
        return TW_KEY_LEFT;
    case GLFW_KEY_INSERT:
        return TW_KEY_INSERT;
    case GLFW_KEY_HOME:
        return TW_KEY_HOME;
    case GLFW_KEY_END:
        return TW_KEY_END;
    case GLFW_KEY_PAGE_UP:
        return TW_KEY_PAGE_UP;
    case GLFW_KEY_PAGE_DOWN:
        return TW_KEY_PAGE_DOWN;
    case GLFW_KEY_F1:
        return TW_KEY_F1;
    case GLFW_KEY_F2:
        return TW_KEY_F2;
    case GLFW_KEY_F3:
        return TW_KEY_F3;
    case GLFW_KEY_F4:
        return TW_KEY_F4;
    case GLFW_KEY_F5:
        return TW_KEY_F5;
    case GLFW_KEY_F6:
        return TW_KEY_F6;
    case GLFW_KEY_F7:
        return TW_KEY_F7;
    case GLFW_KEY_F8:
        return TW_KEY_F8;
    case GLFW_KEY_F9:
        return TW_KEY_F9;
    case GLFW_KEY_F10:
        return TW_KEY_F10;
    case GLFW_KEY_F11:
        return TW_KEY_F11;
    case GLFW_KEY_F12:
        return TW_KEY_F12;
    case GLFW_KEY_F13:
        return TW_KEY_F13;
    case GLFW_KEY_F14:
        return TW_KEY_F14;
    case GLFW_KEY_F15:
        return TW_KEY_F15;

    default:
        return scancode; // everything else
    }
}
