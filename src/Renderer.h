#ifndef RENDERER_H
#define RENDERER_H

#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "State.h"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>



struct Mesh{
    GLuint vertexBuffer;
    GLuint indexBuffer;
};

class Renderer {
public:
    Renderer() {}
    
    void init();
    void loadShader(const GLchar *vShaderStr, const GLchar *fShaderStr, GLuint *id);
    void loadQuad();
    void loadTriangle();
    void startDraw();
    void drawQuad(float x, float y, float angle, float w, float h);
    void drawTriangle(float x, float y, float angle, float w, float h);
    void drawState(const State &state);
    void endDraw();
    void setShader(GLuint program);
    void setColor(float r, float g, float b, float a);
    void setColor2(float r, float g, float b, float a);
    float getRatio() { return _ratio; }
    void finalize();
    
private:
    void printConfigInfo(int n, EGLDisplay display, EGLConfig *config);

    EGLDisplay _display;
    EGLSurface _surface;
    EGLContext _context;
    EGLConfig _config;
    #ifdef DESKTOP
    GLFWwindow* _window;
    #else
    EGL_DISPMANX_WINDOW_T _window;
    #endif
    
    uint32_t _screenWidth;
    uint32_t _screenHeight;
    float _ratio;
    
    glm::mat4 _projection;
    
    // Meshes
    Mesh _quad;
    Mesh _triangle;

    

    // Shaders
    GLuint _colorShader;
    GLuint _verticalGradientShader;
    GLuint _circleShader;
    GLuint _currentProgram;
 
    
};

#endif
