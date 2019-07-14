﻿#include <cassert>
#include <iostream>
#include <chrono>
#include <thread>
#include <GL/glew.h>
#include "logger.h"
#include "window.h"

#define STRINGIFY(value) #value
#define TOKEN_TO_STRING(value) STRINGIFY(value)

Window::Window()
{
    has_iconified = false;
}

Window::~Window()
{
    if(window)
        glfwDestroyWindow(window);
}

bool Window::Initialize(int width, int height)
{
    LOG_I("Compiled against GLFW " << GLFW_VERSION_MAJOR << "." << GLFW_VERSION_MINOR << "." << GLFW_VERSION_REVISION);
    int major, minor, revision;
    glfwGetVersion(&major, &minor, &revision);
    LOG_I("Running against GLFW " << major << "." << minor << "." << revision);

    glfwSetErrorCallback(error_callback);
    if(!glfwInit()){
        LOG_E("Could not initialize GLFW.");
        return false;
    }
    // cf. http://www.glfw.org/docs/latest/window_guide.html#window_hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
#if 0   // TODO: font 周りの都合で互換性を維持.
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#else
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    glfwWindowHint(GLFW_SAMPLES, 4);
    window = glfwCreateWindow(width, height, TOKEN_TO_STRING(PRODUCT_NAME), nullptr, nullptr);
    if(!window){
        LOG_E("Could not open GLFW window.");
        return false;
    }
    glfwSetWindowUserPointer(window, this);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE; // cf. https://www.opengl.org/wiki/OpenGL_Loading_Library
    GLenum glew_error = glewInit();
    if(glew_error != GLEW_OK){
        LOG_E(glewGetErrorString(glew_error));
        return false;
    }
    LOG_I("OpenGL version: " << glGetString(GL_VERSION));
    LOG_I("GLEW version: " << glewGetString(GLEW_VERSION));

    glfwSwapInterval(0);    // 0 で vsync 無視 ... 1秒に n 回
    // callback
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_move_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, mouse_wheel_callback);
    glfwSetFramebufferSizeCallback(window, resize_framebuffer_callback);
    glfwSetWindowSizeCallback(window, resize_window_callback);
    glfwSetWindowIconifyCallback(window, iconify_window_callback);
#ifdef ENABLE_OGL_DEBUG_OUTPUT
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    glDebugMessageCallback(opengl_debug_callback, NULL);
#endif
    return true;
}

void Window::MainLoop()
{
    assert(window);
    Setup();

    constexpr std::int64_t one = std::chrono::duration<std::int64_t, std::nano>(std::chrono::seconds(1)).count();
    constexpr std::int64_t frame_period = one / 60LL;
    constexpr std::int64_t update_period = one / 120LL;

    std::int64_t lag = 0LL;
    std::int64_t sleep_error = 0LL;
    std::int64_t frame_count = 0LL;
    std::int64_t update_count = 0LL;

    auto previous = std::chrono::high_resolution_clock::now();
    auto measure_start = previous;
    while(!glfwWindowShouldClose(window)){
        auto current = std::chrono::high_resolution_clock::now();
        lag += std::chrono::duration<std::int64_t, std::nano>(current - previous).count();
        previous = current;
        // fps の算出
        auto elapsed = std::chrono::duration<std::int64_t, std::nano>(std::chrono::high_resolution_clock::now() - measure_start).count();
        if(elapsed >= one){
            fps = static_cast<double>(frame_count * one) / elapsed;
            ups = static_cast<double>(update_count * one) / elapsed;
            frame_count = update_count = 0LL;
            //std::cout << "fps:" << fps << ", ups:" << ups << std::endl;
            measure_start = std::chrono::high_resolution_clock::now();
        }

        glfwPollEvents();

        while(lag >= update_period){
            OnUpdate(static_cast<double>(update_period) / one);
            lag -= update_period;
            update_count++;
        }
        if(!has_iconified){
            OnRender();
            glfwSwapBuffers(window);
            frame_count++;
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto sleep = frame_period - std::chrono::duration<std::int64_t, std::nano>(end - previous).count() - sleep_error;
        if(sleep > 0){
            std::this_thread::sleep_for(std::chrono::duration<std::int64_t, std::nano>(sleep));
            // sleep の誤差は次フレームで吸収
            sleep_error = std::chrono::duration<std::int64_t, std::nano>(std::chrono::high_resolution_clock::now() - end).count() - sleep;
        }
        else{
            sleep_error = 0LL;
        }
    }
    Cleanup();
}

void Window::error_callback(int error, const char* description)
{
    LOG_E(error << ": " << description);
    assert(!"glfw error");
}

#ifdef ENABLE_OGL_DEBUG_OUTPUT
static const char* source_to_string(GLenum source)
{
    switch (source){
    case GL_DEBUG_SOURCE_API:               // Calls to the OpenGL API
        return "OpenGL";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     // Calls to a window - system API
        return "Window system";
    case GL_DEBUG_SOURCE_SHADER_COMPILER:   // A compiler for a shading language
        return "Shader compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY:       // An application associated with OpenGL
        return "Third party";
    case GL_DEBUG_SOURCE_APPLICATION:       // Generated by the user of this application
        return "Application";
    case GL_DEBUG_SOURCE_OTHER:             // Some source that isn't one of these 
        return "Other";
    default:
        return "Unknown";
    }
}

static const char* type_to_string(GLenum source)
{
    switch(source){
    case GL_DEBUG_TYPE_ERROR:               // An error, typically from the API
        return "Error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: // Some behavior marked deprecated has been used
        return "Deprecated behavior";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  // Something has invoked undefined behavior
        return "Undefined behavior";
    case GL_DEBUG_TYPE_PORTABILITY:         // Some functionality the user relies upon is not portable
        return "Portability";
    case GL_DEBUG_TYPE_PERFORMANCE:         // Code has triggered possible performance issues
        return "Performance";
    case GL_DEBUG_TYPE_MARKER:              // Command stream annotation
        return "Marker";
    case GL_DEBUG_TYPE_PUSH_GROUP:          // Group pushing
        return "Push group";
    case GL_DEBUG_TYPE_POP_GROUP:           // foo
        return "Pop group";
    case GL_DEBUG_TYPE_OTHER:               // Some type that isn't one of these 
        return "Other";
    default:
        return "Unknown";
    }
}

static const char* severity_to_string(GLenum severity)
{
    switch(severity){
    case GL_DEBUG_SEVERITY_HIGH:        // All OpenGL Errors, shader compilation / linking errors, or highly - dangerous undefined behavior
        return "High";
    case GL_DEBUG_SEVERITY_MEDIUM:      // Major performance warnings, shader compilation / linking warnings, or the use of deprecated functionality
        return "Medium";
    case GL_DEBUG_SEVERITY_LOW:         // Redundant state change performance warning, or unimportant undefined behavior
        return "Low";
    case GL_DEBUG_SEVERITY_NOTIFICATION:// Anything that isn't an error or performance issue. 
        return "Notification";
    default:
        return "Unknown";
    }
}

void APIENTRY Window::opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *param)
{
    LOG_D("source: " << source_to_string(source) << " type: " << type_to_string(type) << " id: " << id << " severity: " << severity_to_string(severity) << " message: " << message);
}
#endif

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    w->OnKey(window, key, scancode, action, mods);
}

void Window::mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
    Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    w->OnMouseMove(window, xpos, ypos);
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    w->OnMouseButton(window, button, action, mods);
}

void Window::mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    w->OnMouseWheel(window, xoffset, yoffset);
}

void Window::resize_framebuffer_callback(GLFWwindow* window, int width, int height)
{
    Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    w->OnResizeFramebuffer(window, width, height);
}

void Window::resize_window_callback(GLFWwindow* window, int width, int height)
{
    Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    w->OnResizeWindow(window, width, height);
}

void Window::iconify_window_callback(GLFWwindow* window, int iconified)
{
    Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    w->OnIconifyWindow(window, iconified);
}

void Window::OnKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //std::cout << "Window::OnKey{key=" << key << ", scancode=" << scancode << ", action=" << action << ", mods=" << mods << std::endl;
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void Window::OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
    //std::cout << "Window::OnMouseMove{" << xpos << ", " << ypos << "}" << std::endl;
}

void Window::OnMouseButton(GLFWwindow* window, int button, int action, int mods)
{
    //std::cout << "Window::OnMouseButton{button=" << button << ", action=" << action << ", mods=" << mods << std::endl;
}

void Window::OnMouseWheel(GLFWwindow* window, double xoffset, double yoffset)
{
    //std::cout << "Window::OnMouseWheel{" << xoffset << ", " << yoffset << "}" << std::endl;
}

void Window::OnResizeFramebuffer(GLFWwindow* window, int width, int height)
{
    //std::cout << "Window::OnResizeFramebuffer{" << width << ", " << height << "}" << std::endl;
}

void Window::OnResizeWindow(GLFWwindow* window, int width, int height)
{
    //std::cout << "Window::OnResizeWindow{" << width << ", " << height << "}" << std::endl;
}

void Window::OnIconifyWindow(GLFWwindow* window, int iconified)
{
    //std::cout << "Window::OnIconifyWindow{iconified=" << iconified << "}" << std::endl;
    has_iconified = (iconified == GL_TRUE);
}