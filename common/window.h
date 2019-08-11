#pragma once
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#define RECORD_STATISTICS
#if defined(RECORD_STATISTICS)
#include "circular_buffer.h"
#endif

#define ENABLE_OGL_DEBUG_OUTPUT

namespace common
{

class Window
{
public:
    Window();
    virtual ~Window();

    Window(const Window&) = delete;
    Window& operator = (const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator = (Window&&) = delete;

    bool Initialize(int width, int height);
    void MainLoop();

protected:
    GLFWwindow* GetWindow() { return window; }
    double GetFPS() { return fps; }
    double GetUPS() { return ups; }
    virtual void Setup() = 0;
    virtual void Cleanup() = 0;

    virtual void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods);
    virtual void OnMouseMove(GLFWwindow* window, double xpos, double ypos);
    virtual void OnMouseButton(GLFWwindow* window, int button, int action, int mods);
    virtual void OnMouseWheel(GLFWwindow* window, double xoffset, double yoffset);
    virtual void OnResizeFramebuffer(GLFWwindow* window, int width, int height);
    virtual void OnResizeWindow(GLFWwindow* window, int width, int height);
    virtual void OnIconifyWindow(GLFWwindow* window, int iconified);

    bool HasIconified() { return has_iconified; }

#if defined(RECORD_STATISTICS)
    const simple_circular_buffer<double>& GetFPSRecord() const noexcept { return fps_record; };
    const simple_circular_buffer<double>& GetUPSRecord() const noexcept { return ups_record; };
#endif

private:
    static void error_callback(int error, const char* description);
#ifdef ENABLE_OGL_DEBUG_OUTPUT
    static void APIENTRY opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *param);
#endif
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void resize_framebuffer_callback(GLFWwindow* window, int width, int height);
    static void resize_window_callback(GLFWwindow* window, int width, int height);
    static void iconify_window_callback(GLFWwindow* window, int iconified);

    virtual void OnUpdate(double dt) = 0;
    virtual void OnRender() = 0;
    virtual void OnGUI();

private:
    GLFWwindow* window = nullptr;
    double fps = 0.0;
    double ups = 0.0;

    bool has_iconified;

#if defined(RECORD_STATISTICS)
    simple_circular_buffer<double> fps_record;
    simple_circular_buffer<double> ups_record;
#endif
};

}   // namespace common