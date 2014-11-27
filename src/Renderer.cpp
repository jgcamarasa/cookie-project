#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifndef DESKTOP
#include <bcm_host.h>
#endif

#include <glm/gtc/matrix_transform.hpp>
#include <math.h>


#include "Renderer.h"

#define RADIANS_TO_DEGREES(__ANGLE__) ((__ANGLE__) / M_PI * 180.0) 

const GLchar *vColorShaderStr = "\n"
"attribute vec4 vPosition;\n"
"uniform mat4 MVP;\n"
"void main() {\n"
"   gl_Position = MVP*vPosition;\n"
"}\0";

const GLchar *fColorShaderStr = "\n"
"uniform vec4 color;\n"
"void main() {\n"
"   gl_FragColor = color;\n"
"}\0";


// vertical gradient
const GLchar *vVerticalGradientShaderStr = "\n"
"attribute vec4 vPosition;\n"
"uniform mat4 MVP;\n"
"varying vec2 texCoord;\n"
"void main() {\n"
"   gl_Position = MVP*vPosition;\n"
"   texCoord = vec2(vPosition.x+0.5, vPosition.y+0.5);\n"
"}\0";

const GLchar *fVerticalGradientShaderStr = "\n"
"uniform vec4 color;\n"
"uniform vec4 color2;\n"
"varying vec2 texCoord;\n"
"void main() {\n"
"   gl_FragColor = mix(color, color2, 1.0-texCoord.y);\n"
"}\0";

// circle
const GLchar *vCircleShaderStr = "\n"
"attribute vec4 vPosition;\n"
"uniform mat4 MVP;\n"
"varying vec2 texCoord;\n"
"void main() {\n"
"   gl_Position = MVP*vPosition;\n"
"   texCoord = vec2(vPosition.x, vPosition.y);\n"
"}\0";

const GLchar *fCircleShaderStr = "\n"
"uniform vec4 color;\n"
"uniform vec4 color2;\n"
"varying vec2 texCoord;\n"
"void main() {\n"
"   float dist = sqrt(texCoord.x*texCoord.x+texCoord.y*texCoord.y);\n"
"   if(dist < 0.5)\n"
"      gl_FragColor = color;\n"
"   else\n"
"      gl_FragColor = color2;\n"
"}\0";


using namespace glm;


void error_callback(int error, const char* description)
{
fputs(description, stderr);
}


void getThrustPositions(int numPlayers, float xPos[3], float yPos[3]);

#ifdef DESKTOP
void Renderer::init(){
    glfwSetErrorCallback(error_callback);
    /* Initialize the library */
    if (!glfwInit()){
        return;
    }

    
    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    
    _screenWidth = mode->width;
    _screenHeight = mode->height;
    
    printf("%dx%d\n", _screenWidth, _screenHeight);
    
    _window = glfwCreateWindow(_screenWidth, _screenHeight, "My Title", glfwGetPrimaryMonitor(), NULL);
    
    if (!_window)
    {
       
    }

    glfwMakeContextCurrent(_window);

    
    loadTriangle();
    loadQuad();
    
    loadShader(vColorShaderStr, fColorShaderStr, &_colorShader);
    loadShader(vVerticalGradientShaderStr, fVerticalGradientShaderStr, &_verticalGradientShader);
    loadShader(vCircleShaderStr, fCircleShaderStr, &_circleShader);
    
    _ratio = _screenWidth/(float)_screenHeight;
    _projection = glm::ortho(-10.0f*_ratio, 10.0f*_ratio, -10.0f, 10.0f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}
#else
void Renderer::init(){
    static const EGLint attribute_list[] =
   {
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_NONE
   };

    static const EGLint context_attributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    _display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    
    EGLBoolean result = eglInitialize(_display, NULL, NULL);
    
    if (result == EGL_FALSE) {
        fprintf(stderr, "Can't initialise EGL\n");
        exit(1);
    }

    EGLint num_configs;
    EGLConfig *configs;
    
    eglGetConfigs(_display, NULL, 0, &num_configs);
    printf("EGL has %d configs\n", num_configs);

    configs = (EGLConfig*)calloc(num_configs, sizeof *configs);
    eglGetConfigs(_display, configs, num_configs, &num_configs);
    
    // get an appropriate EGL configuration - just use the first available
    result = eglChooseConfig(_display, attribute_list, 
			     &_config, 1, &num_configs);
    assert(EGL_FALSE != result);

    // Choose the OpenGL ES API
    result = eglBindAPI(EGL_OPENGL_ES_API);
    assert(EGL_FALSE != result);

    // create an EGL rendering context
    _context = eglCreateContext(_display, 
				      _config, EGL_NO_CONTEXT, 
				      context_attributes);
    assert(_context!=EGL_NO_CONTEXT);
    printf("Got an EGL context\n");
    
    int32_t success = 0;   

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;


    // create an EGL window surface
    success = graphics_get_display_size(0 /* LCD */, 
					&_screenWidth, 
					&_screenHeight);
    assert( success >= 0 );

    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = _screenWidth;
    dst_rect.height = _screenHeight;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = _screenWidth << 16;
    src_rect.height = _screenHeight << 16;        

    dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
    dispman_update = vc_dispmanx_update_start( 0 );

    dispman_element = 
	vc_dispmanx_element_add(dispman_update, dispman_display,
				0/*layer*/, &dst_rect, 0/*src*/,
				&src_rect, DISPMANX_PROTECTION_NONE, 
				0 /*alpha*/, 0/*clamp*/, (DISPMANX_TRANSFORM_T)0/*transform*/);

    // Build an EGL_DISPMANX_WINDOW_T from the Dispmanx window
    _window.element = dispman_element;
    _window.width = _screenWidth;
    _window.height = _screenHeight;
    vc_dispmanx_update_submit_sync(dispman_update);

    printf("Got a Dispmanx window\n");
    
    

    _surface = eglCreateWindowSurface(_display, 
					    _config, 
					    &_window, NULL );
    assert(_surface != EGL_NO_SURFACE);

    // connect the context to the surface
    result = eglMakeCurrent(_display, _surface, _surface, _context);
    assert(EGL_FALSE != result);
    
    assert(glGetError() == GL_NO_ERROR);
    
    loadTriangle();
    loadQuad();
    
    loadShader(vColorShaderStr, fColorShaderStr, &_colorShader);
    loadShader(vVerticalGradientShaderStr, fVerticalGradientShaderStr, &_verticalGradientShader);
    
    _ratio = _screenWidth/(float)_screenHeight;
    _projection = glm::ortho(-10.0f*_ratio, 10.0f*_ratio, -10.0f, 10.0f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
}
#endif


void Renderer::loadShader(const GLchar *vShaderStr, const GLchar *fShaderStr, GLuint *id){
    char log[10000];

    // Vertex Shader
    int vShader = glCreateShader(GL_VERTEX_SHADER);
    int nShaders = strlen(vShaderStr);
    glShaderSource(vShader, 1, &vShaderStr, &nShaders);
    
    glCompileShader(vShader);
    

    GLint isCompiled = 0;
    //glGetShaderiv(_vShader, GL_COMPILE_STATUS, &isCompiled);
    glGetShaderInfoLog(vShader, 10000, NULL, log);
    printf("Vertex shader status: %s\n", log);
    
    // Fragment Shader
    int fShader = glCreateShader(GL_FRAGMENT_SHADER);

    nShaders = strlen(fShaderStr);
    glShaderSource(fShader, 1, &fShaderStr, &nShaders);

    glCompileShader(fShader);
    glGetShaderInfoLog(fShader, 10000, NULL, log);
    printf("Fragment shader status: %s\n", log);
    
    
    // Shader Program
    *id = glCreateProgram();
    glAttachShader(*id, vShader);
    glAttachShader(*id, fShader);
    glLinkProgram(*id);
    glGetProgramInfoLog(*id, 10000, NULL, log);
    printf("Linking shader status:%s\n", log);
    
    glUseProgram(*id);
    
    //assert(glGetError() == GL_NO_ERROR);
    
    GLint vPositionIndex = glGetAttribLocation(*id, "vPosition");
    
        
    //assert(glGetError() == GL_NO_ERROR);
    
    
    _currentProgram = _colorShader;
}

void Renderer::loadQuad(){
    GLfloat verts[] = {
        -0.5f, +0.5f, +0.0f,
        +0.5f, +0.5f, +0.0f,
        +0.5f, -0.5f, +0.0f,
        -0.5f, -0.5f, +0.0f };
        
    GLfloat uvs[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,};
    
    GLushort indices[] = {
        0, 1, 2, 0, 3, 2
    };
    
    
    // Vertex Buffer 
    glGenBuffers(1, &_quad.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _quad.vertexBuffer);
    glBufferData(
        GL_ARRAY_BUFFER, 
        4*3*sizeof(GLfloat),
        verts,
        GL_STATIC_DRAW);
    

    
    // Indices
    glGenBuffers(1, &_quad.indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quad.indexBuffer);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, 
        3*2*sizeof(GLushort),
        indices,
        GL_STATIC_DRAW);
    
    assert(glGetError() == GL_NO_ERROR);
}

void Renderer::loadTriangle(){
    GLfloat verts[] = {
        +0.0f, +1.0f, +0.0f,
        +0.866f, -0.5f, +0.0f,
        -0.866f, -0.5f, +0.0f
        };
        

    GLushort indices[] = {
        0, 1, 2
    };
    
    
    // Vertex Buffer 
    glGenBuffers(1, &_triangle.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _triangle.vertexBuffer);
    glBufferData(
        GL_ARRAY_BUFFER, 
        3*3*sizeof(GLfloat),
        verts,
        GL_STATIC_DRAW);
    

    
    // Indices
    glGenBuffers(1, &_triangle.indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _triangle.indexBuffer);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, 
        3*sizeof(GLushort),
        indices,
        GL_STATIC_DRAW);
    
    assert(glGetError() == GL_NO_ERROR);
    
}


void Renderer::startDraw(){
    glClearColor(0.36f, 0.40f, 0.38f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
}

void getColorForPlayer(int player, float color[4]){
    color[0] = color[1] = color[2] = 0.0f;
    color[3] = 1.0f;
    switch(player){
        case 0:
            color[0]=1.0f;
            break;
        case 1:
            color[1]=1.0f;
            break;
        case 2:
            color[2]=1.0f;
            break;
        case 3:
            color[0]=1.0f;
            color[2]=1.0f;
            break;
        case 4:
            color[0]=1.0f;
            color[1]=1.0f;
            break;
        
    }
}

b2Vec2 getBoundingBox(b2Body *body){
    b2Fixture* fixture = body->GetFixtureList();
    b2PolygonShape* shape = (b2PolygonShape*)fixture->GetShape();
    
    float maxX, minX, maxY, minY;
    maxX = maxY = -1000.0f;
    minX = minY = +1000.0f;
    
    for(int i = 0; i < shape->GetVertexCount(); i++){
        b2Vec2 v = shape->GetVertex(i);
        if(v.x > maxX){
            maxX = v.x;
        }else if(v.x < minX){
            minX = v.x;
        }
        if(v.y > maxY){
            maxY = v.y;
        }else if(v.y < minY){
            minY = v.y;
        }
    }
    
    
    return b2Vec2(maxX-minX, maxY-minY);
}

void Renderer::drawState(const State &state){
    //draw sky
    setShader(_verticalGradientShader);
    setColor(0.04f, 0.39f, 0.76f, 1.0f);
    setColor2(0.3f, 0.7f, 1.0f, 1.0f);
    drawQuad(0.0f, 0.0f, 0.0f, 20.0f*_ratio, 20.0f);
    
    setShader(_colorShader);
    b2Vec2 pos;
    // draw ground
    for(int i = 0; i < state.ground.size(); i++){
        setColor(0.0f, 0.0f, 0.0f, 1.0f);
        b2Body *ground = state.ground[i];
        pos = ground->GetPosition();
        b2Vec2 size = getBoundingBox(ground);
        float angle = RADIANS_TO_DEGREES(ground->GetAngle());
        drawQuad(pos.x, pos.y, angle, size.x, size.y);
    }


    // player
    b2Body *player = state.player;
    pos = player->GetPosition();
    float angle = RADIANS_TO_DEGREES(player->GetAngle());
    float radAngle = player->GetAngle();
    // draw thrusts
    setShader(_verticalGradientShader);
    setColor2(0.0f, 0.0f, 0.0f, 0.0f);
    float xPos[3], yPos[3];
    getThrustPositions(state.numPlayers, xPos, yPos);
    b2Vec2 thrust0 = player->GetWorldPoint(b2Vec2(xPos[0],yPos[0]));
    b2Vec2 thrust1 = player->GetWorldPoint(b2Vec2(xPos[1],yPos[1]));
    b2Vec2 thrust2 = player->GetWorldPoint(b2Vec2(xPos[2],yPos[2]));
    b2Vec2* thrustVec[3] = {&thrust0, &thrust1, &thrust2};

    float angs[3] = {-30, 30, 0};
    for(int i = 0; i < MAX_PLAYERS; i++){
        if(state.playerStates[i]){
            float color[4];
            getColorForPlayer(i, color);
            setColor(color[0], color[1], color[2], color[3]);
            
            drawQuad(
                thrustVec[i]->x, 
                thrustVec[i]->y, 
                angle+240*i/*angs[i]*/, 0.2f, 2.5f);
        }
    }

    
    // draw player
    setShader(_colorShader);
    setColor(0.07f, 0.20f, 0.32f, 1.0f);
    drawTriangle(pos.x, pos.y, angle, 1.0f, 1.0f);

    
    setShader(_circleShader);
    setColor(1.0f, 0.0f, 0.0f, 1.0f);
    setColor2(0.0f, 0.0f, 0.0f, 0.0f);
    drawQuad(
        //pos.x+sin(radAngle)*yPos[0]+cos(radAngle)*xPos[0], 
        //pos.y-cos(radAngle)*yPos[0]+sin(radAngle)*xPos[0], 
        thrust0.x,
        thrust0.y,
        angle, 0.35f, 0.35f);
        
    setShader(_circleShader);
    setColor(0.0f, 1.0f, 0.0f, 1.0f);
    setColor2(0.0f, 0.0f, 0.0f, 0.0f);
    drawQuad(
        //pos.x+sin(radAngle)*yPos[1]+cos(radAngle)*xPos[1], 
        //pos.y-cos(radAngle)*yPos[1]+sin(radAngle)*xPos[1], 
        thrust1.x,
        thrust1.y,
        angle, 0.35f, 0.35f);
    
    setShader(_circleShader);
    setColor(0.0f, 0.0f, 1.0f, 1.0f);
    setColor2(0.0f, 0.0f, 0.0f, 0.0f);
    drawQuad(
        //pos.x+sin(radAngle)*yPos[2]+cos(radAngle)*xPos[2], 
        //pos.y-cos(radAngle)*yPos[2]+sin(radAngle)*xPos[2], 
        thrust2.x,
        thrust2.y,
        angle, 0.35f, 0.35f);
    
    
    
    
    
}

void Renderer::drawQuad(float x, float y, float angle, float w, float h){
    glBindBuffer(GL_ARRAY_BUFFER, _quad.vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quad.indexBuffer);
    
    GLint vPositionIndex = glGetAttribLocation(_currentProgram, "vPosition");
    glVertexAttribPointer(
        vPositionIndex, 
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        0);
    glEnableVertexAttribArray(vPositionIndex);
    
    GLuint MVPIndex = glGetUniformLocation(_currentProgram, "MVP");
    //printf("MVPINDEX %d\n", MVPIndex);

    glm::mat4x4 model;
    model = glm::translate(model, glm::vec3(x, y, 1.0f));
    model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(w, h, 0.0f));
    glm::mat4x4 MVP = _projection * model;

    glUniformMatrix4fv(MVPIndex, 1, GL_FALSE, &MVP[0][0]);
    
    /*for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            printf("%f,",_projection[i][j]);
        }
        printf("\n");
    }*/
    

    glDrawElements(
        GL_TRIANGLES,
        6,
        GL_UNSIGNED_SHORT,
        (void*)0);
    
    glDisableVertexAttribArray(vPositionIndex);
    
}

void Renderer::drawTriangle(float x, float y, float angle, float w, float h){
    glBindBuffer(GL_ARRAY_BUFFER, _triangle.vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _triangle.indexBuffer);
    
    GLint vPositionIndex = glGetAttribLocation(_currentProgram, "vPosition");
    glVertexAttribPointer(
        vPositionIndex, 
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        0);
    glEnableVertexAttribArray(vPositionIndex);
    
    GLuint MVPIndex = glGetUniformLocation(_currentProgram, "MVP");
    //printf("MVPINDEX %d\n", MVPIndex);

    glm::mat4x4 model;
    model = glm::translate(model, glm::vec3(x, y, 1.0f));
    model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(w, h, 0.0f));
    glm::mat4x4 MVP = _projection * model;

    glUniformMatrix4fv(MVPIndex, 1, GL_FALSE, &MVP[0][0]);
    
    /*for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            printf("%f,",_projection[i][j]);
        }
        printf("\n");
    }*/
    

    glDrawElements(
        GL_TRIANGLES,
        3,
        GL_UNSIGNED_SHORT,
        (void*)0);
        
    glDisableVertexAttribArray(vPositionIndex);
}

void Renderer::endDraw(){
    
    glFlush();
    #ifdef DESKTOP
    glfwSwapBuffers(_window);
    #else
    eglSwapBuffers(_display, _surface);
    #endif
    
    
    
}

void Renderer::setShader(GLuint program){
    _currentProgram = program;
    glUseProgram(program);
}

void Renderer::setColor(float r, float g, float b, float a){
    GLuint colorIndex = glGetUniformLocation(_currentProgram, "color");
    glUniform4f(colorIndex, r, g, b, a);
}

void Renderer::setColor2(float r, float g, float b, float a){
    GLuint colorIndex = glGetUniformLocation(_currentProgram, "color2");
    glUniform4f(colorIndex, r, g, b, a);
}

void Renderer::finalize(){
     // clear screen
   glClear( GL_COLOR_BUFFER_BIT );
   eglSwapBuffers(_display, _surface);

   // Release OpenGL resources
   eglMakeCurrent( _display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
   eglDestroySurface(_display, _surface);
   eglDestroyContext( _display, _context);
   eglTerminate( _display );
}

void Renderer::printConfigInfo(int n, EGLDisplay display, EGLConfig *config) {
    int size;

    printf("Configuration %d is\n", n);

    eglGetConfigAttrib(display,
		       *config, EGL_RED_SIZE, &size);
    printf("  Red size is %d\n", size);
    eglGetConfigAttrib(display,
		       *config, EGL_BLUE_SIZE, &size);
    printf("  Blue size is %d\n", size);
    eglGetConfigAttrib(display,
		       *config, EGL_GREEN_SIZE, &size);
    printf("  Green size is %d\n", size);
    eglGetConfigAttrib(display,
		       *config, EGL_BUFFER_SIZE, &size);
    printf("  Buffer size is %d\n", size);

   eglGetConfigAttrib(display,
		       *config,  EGL_BIND_TO_TEXTURE_RGB , &size);
   if (size == EGL_TRUE)
       printf("  Can be bound to RGB texture\n");
   else
       printf("  Can't be bound to RGB texture\n");

   eglGetConfigAttrib(display,
		       *config,  EGL_BIND_TO_TEXTURE_RGBA , &size);
   if (size == EGL_TRUE)
       printf("  Can be bound to RGBA texture\n");
   else
       printf("  Can't be bound to RGBA texture\n");
}
