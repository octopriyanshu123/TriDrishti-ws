#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>

// ─────────────────────────────────────────────
//  Geometry parameters
// ─────────────────────────────────────────────
static const float OUTER_RADIUS = 5.0f;
static const float THICKNESS    = 0.3f;
static const float INNER_RADIUS = OUTER_RADIUS - THICKNESS;
static const float HEIGHT       = 15.0f;
static const int   SEGMENTS     = 120;

// ─────────────────────────────────────────────
//  Camera  (spherical orbit + roll)
// ─────────────────────────────────────────────
struct Camera
{
    float yaw    =  90.0f;
    float pitch  = 30.0f;
    float radius =  35.0f;
    float roll   =   0.0f;
    glm::vec3 target{0.0f, 0.0f, 0.0f};

    glm::vec3 position() const
    {
        float ry = glm::radians(yaw);
        float rp = glm::radians(pitch);
        return target + glm::vec3(
            radius * cosf(rp) * cosf(ry),
            radius * sinf(rp),
            radius * cosf(rp) * sinf(ry));
    }

    glm::mat4 view() const
    {
        glm::vec3 pos     = position();
        glm::vec3 forward = glm::normalize(target - pos);
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 rollMat = glm::rotate(glm::mat4(1.0f),
                                        glm::radians(roll), forward);
        glm::vec3 up = glm::vec3(rollMat * glm::vec4(worldUp, 0.0f));
        return glm::lookAt(pos, target, up);
    }
} cam;

// ─────────────────────────────────────────────
//  Object rotation state  (model matrix)
// ─────────────────────────────────────────────
static float g_rotX = 0.0f;
static float g_rotY = 0.0f;
static float g_rotZ = 0.0f;

// ─────────────────────────────────────────────
//  Mouse state
// ─────────────────────────────────────────────
static bool   g_dragging  = false;   // left  → orbit camera
static bool   g_rolling   = false;   // right → roll camera
static double g_lastX     = 0.0;
static double g_lastY     = 0.0;

// ═════════════════════════════════════════════
//  SHADER 1 – Phong (cylinder)
// ═════════════════════════════════════════════
static const char* VS_PHONG = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
out vec3 FragPos;
out vec3 Normal;
uniform mat4 model, view, projection;
void main(){
    vec4 wp = model * vec4(aPos,1.0);
    FragPos  = wp.xyz;
    Normal   = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * wp;
}
)";

static const char* FS_PHONG = R"(
#version 330 core
in  vec3 FragPos;
in  vec3 Normal;
out vec4 FragColor;
uniform vec3 lightPos, viewPos, objectColor;
void main(){
    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir  = normalize(viewPos  - FragPos);
    vec3 ambient  = 0.25 * vec3(1.0);
    vec3 diffuse  = max(dot(norm,lightDir),0.0) * vec3(1.0);
    vec3 spec     = 0.7 * pow(max(dot(viewDir,reflect(-lightDir,norm)),0.0),64.0) * vec3(1.0);
    FragColor = vec4((ambient+diffuse+spec)*objectColor, 1.0);
}
)";

// ═════════════════════════════════════════════
//  SHADER 2 – Flat colour (grid + axes)
// ═════════════════════════════════════════════
static const char* VS_FLAT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;
out vec3 vColor;
uniform mat4 view, projection;
void main(){
    vColor = aColor;
    gl_Position = projection * view * vec4(aPos,1.0);
}
)";

static const char* FS_FLAT = R"(
#version 330 core
in  vec3 vColor;
out vec4 FragColor;
void main(){ FragColor = vec4(vColor,1.0); }
)";

// ─────────────────────────────────────────────
//  Shader helpers
// ─────────────────────────────────────────────
static GLuint compileShader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok){ char log[512]; glGetShaderInfoLog(s,512,nullptr,log);
              std::cerr << "Shader error:\n" << log << "\n"; }
    return s;
}

static GLuint buildProgram(const char* vs, const char* fs)
{
    GLuint p = glCreateProgram();
    GLuint v = compileShader(GL_VERTEX_SHADER,   vs);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
    glAttachShader(p,v); glAttachShader(p,f);
    glLinkProgram(p);
    GLint ok; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok){ char log[512]; glGetProgramInfoLog(p,512,nullptr,log);
              std::cerr << "Link error:\n" << log << "\n"; }
    glDeleteShader(v); glDeleteShader(f);
    return p;
}

// ─────────────────────────────────────────────
//  Vertex types
// ─────────────────────────────────────────────
struct Vertex   { float x,y,z, nx,ny,nz; };
struct FlatVert { float x,y,z, r,g,b;    };

// ─────────────────────────────────────────────
//  GPU line buffer
// ─────────────────────────────────────────────
struct LineBuf { GLuint vao, vbo; GLsizei n; };

static LineBuf uploadLines(const std::vector<FlatVert>& v)
{
    LineBuf b; b.n = (GLsizei)v.size();
    glGenVertexArrays(1,&b.vao); glGenBuffers(1,&b.vbo);
    glBindVertexArray(b.vao);
    glBindBuffer(GL_ARRAY_BUFFER, b.vbo);
    glBufferData(GL_ARRAY_BUFFER, v.size()*sizeof(FlatVert), v.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(FlatVert),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(FlatVert),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return b;
}

// ─────────────────────────────────────────────
//  Build hollow cylinder
// ─────────────────────────────────────────────
static void buildHollowCylinder(std::vector<Vertex>& verts,
                                 std::vector<unsigned int>& idx)
{
    verts.clear(); idx.clear();
    const float yBot   = -HEIGHT / 2.0f;
    const float yTop   =  HEIGHT / 2.0f;
    const float TWO_PI = 2.0f * 3.14159265358979f;

    auto addQuad = [&](Vertex a, Vertex b, Vertex c, Vertex d){
        unsigned int base = (unsigned int)verts.size();
        verts.push_back(a); verts.push_back(b);
        verts.push_back(c); verts.push_back(d);
        idx.push_back(base+0); idx.push_back(base+1); idx.push_back(base+2);
        idx.push_back(base+0); idx.push_back(base+2); idx.push_back(base+3);
    };

    for (int i = 0; i < SEGMENTS; ++i)
    {
        float a0 = (float)i       / SEGMENTS * TWO_PI;
        float a1 = (float)(i + 1) / SEGMENTS * TWO_PI;
        float c0=cosf(a0), s0=sinf(a0), c1=cosf(a1), s1=sinf(a1);

        // Outer wall
        addQuad({OUTER_RADIUS*c0,yBot,OUTER_RADIUS*s0, c0,0,s0},
                {OUTER_RADIUS*c1,yBot,OUTER_RADIUS*s1, c1,0,s1},
                {OUTER_RADIUS*c1,yTop,OUTER_RADIUS*s1, c1,0,s1},
                {OUTER_RADIUS*c0,yTop,OUTER_RADIUS*s0, c0,0,s0});

        // Inner wall (reversed winding)
        addQuad({INNER_RADIUS*c0,yTop,INNER_RADIUS*s0,-c0,0,-s0},
                {INNER_RADIUS*c1,yTop,INNER_RADIUS*s1,-c1,0,-s1},
                {INNER_RADIUS*c1,yBot,INNER_RADIUS*s1,-c1,0,-s1},
                {INNER_RADIUS*c0,yBot,INNER_RADIUS*s0,-c0,0,-s0});

        // Top annulus
        addQuad({INNER_RADIUS*c0,yTop,INNER_RADIUS*s0, 0,1,0},
                {OUTER_RADIUS*c0,yTop,OUTER_RADIUS*s0, 0,1,0},
                {OUTER_RADIUS*c1,yTop,OUTER_RADIUS*s1, 0,1,0},
                {INNER_RADIUS*c1,yTop,INNER_RADIUS*s1, 0,1,0});

        // Bottom annulus (reversed winding)
        addQuad({INNER_RADIUS*c1,yBot,INNER_RADIUS*s1, 0,-1,0},
                {OUTER_RADIUS*c1,yBot,OUTER_RADIUS*s1, 0,-1,0},
                {OUTER_RADIUS*c0,yBot,OUTER_RADIUS*s0, 0,-1,0},
                {INNER_RADIUS*c0,yBot,INNER_RADIUS*s0, 0,-1,0});
    }
}

// ─────────────────────────────────────────────
//  Build XZ grid  (Y = -HEIGHT/2, world floor)
// ─────────────────────────────────────────────
static std::vector<FlatVert> buildGrid()
{
    std::vector<FlatVert> v;
    const float Y     = -HEIGHT / 2.0f;
    const float RANGE = 50.0f;
    const float STEP  = 5.0f;
    const float r=0.22f, g=0.22f, b=0.30f;

    for (float t = -RANGE; t <= RANGE; t += STEP)
    {
        v.push_back({t, Y, -RANGE, r,g,b});
        v.push_back({t, Y,  RANGE, r,g,b});
        v.push_back({-RANGE, Y, t, r,g,b});
        v.push_back({ RANGE, Y, t, r,g,b});
    }
    return v;
}

// ─────────────────────────────────────────────
//  Build XYZ axis lines
// ─────────────────────────────────────────────
static std::vector<FlatVert> buildAxes()
{
    const float L = 12.0f;
    return {
        { 0,0,0,  1.0f,0.15f,0.15f }, { L,0,0,  1.0f,0.15f,0.15f },  // X red
        { 0,0,0,  0.15f,1.0f,0.15f }, { 0,L,0,  0.15f,1.0f,0.15f },  // Y green
        { 0,0,0,  0.15f,0.40f,1.0f }, { 0,0,L,  0.15f,0.40f,1.0f },  // Z blue
    };
}

// ─────────────────────────────────────────────
//  GLFW callbacks
// ─────────────────────────────────────────────
static void mouseButtonCallback(GLFWwindow* win, int button, int action, int)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        g_dragging = (action == GLFW_PRESS);
        glfwGetCursorPos(win, &g_lastX, &g_lastY);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        g_rolling  = (action == GLFW_PRESS);
        glfwGetCursorPos(win, &g_lastX, &g_lastY);
    }
}

static void cursorPosCallback(GLFWwindow*, double xpos, double ypos)
{
    float dx = (float)(xpos - g_lastX);
    float dy = (float)(ypos - g_lastY);
    g_lastX = xpos;
    g_lastY = ypos;

    if (g_dragging) {                          // Left-click  → orbit yaw + pitch
        cam.yaw   += dx * 0.4f;
        cam.pitch  = glm::clamp(cam.pitch - dy * 0.4f, -89.0f, 89.0f);
    }
    if (g_rolling) {                           // Right-click → roll
        cam.roll  += dx * 0.4f;
    }
}

static void scrollCallback(GLFWwindow*, double, double yoffset)
{
    cam.radius -= (float)yoffset * 1.0f;       // scroll = zoom
    cam.radius  = glm::clamp(cam.radius, 3.0f, 200.0f);
}

static void framebufferSizeCallback(GLFWwindow*, int w, int h)
{
    glViewport(0, 0, w, h);
}

// ─────────────────────────────────────────────
//  Main
// ─────────────────────────────────────────────
int main()
{
    if (!glfwInit()) { std::cerr<<"glfwInit failed\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(900, 700,
        "Hollow Cylinder  |  LMB=Orbit  RMB=Roll  Scroll=Zoom  Q/E=RotY  Z/X=RotZ",
        nullptr, nullptr);
    if (!window) { std::cerr<<"glfwCreateWindow failed\n"; glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetMouseButtonCallback(window,     mouseButtonCallback);
    glfwSetCursorPosCallback(window,       cursorPosCallback);
    glfwSetScrollCallback(window,          scrollCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cerr<<"glewInit failed\n"; return -1; }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.10f, 0.12f, 0.15f, 1.0f);

    // ── Shader programs ────────────────────────
    GLuint phongProg = buildProgram(VS_PHONG, FS_PHONG);
    GLuint flatProg  = buildProgram(VS_FLAT,  FS_FLAT);

    // ── Cylinder GPU buffers ───────────────────
    std::vector<Vertex>       verts;
    std::vector<unsigned int> indices;
    buildHollowCylinder(verts, indices);

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,nx));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // ── Grid + Axes GPU buffers ────────────────
    LineBuf gridBuf = uploadLines(buildGrid());
    LineBuf axesBuf = uploadLines(buildAxes());

    // ── Uniform locations ──────────────────────
    GLint uP_model = glGetUniformLocation(phongProg,"model");
    GLint uP_view  = glGetUniformLocation(phongProg,"view");
    GLint uP_proj  = glGetUniformLocation(phongProg,"projection");
    GLint uP_light = glGetUniformLocation(phongProg,"lightPos");
    GLint uP_vpos  = glGetUniformLocation(phongProg,"viewPos");
    GLint uP_color = glGetUniformLocation(phongProg,"objectColor");
    GLint uF_view  = glGetUniformLocation(flatProg,"view");
    GLint uF_proj  = glGetUniformLocation(flatProg,"projection");

    // ── Render loop ────────────────────────────
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Keyboard: rotate object on Y and Z axes
        const float rotSpeed = 0.5f;
        if (glfwGetKey(window, GLFW_KEY_Q)      == GLFW_PRESS) g_rotY -= rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_E)      == GLFW_PRESS) g_rotY += rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_Z)      == GLFW_PRESS) g_rotZ -= rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_X)      == GLFW_PRESS) g_rotZ += rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ── Projection (recalculated every frame for correct resize) ──
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        float aspect = (h > 0) ? (float)w / (float)h : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 300.0f);

        // ── Camera (spherical orbit + roll) ───────
        glm::mat4 viewMat    = cam.view();
        glm::vec3 cameraPos  = cam.position();

        // ── Model (object rotation on all 3 axes) ─
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(g_rotX), glm::vec3(1,0,0));
        model = glm::rotate(model, glm::radians(g_rotY), glm::vec3(0,1,0));
        model = glm::rotate(model, glm::radians(g_rotZ), glm::vec3(0,0,1));

        glm::vec3 lightPos(15.0f, 20.0f, 20.0f);

        // ── Draw grid + axes (flat shader) ────────
        glUseProgram(flatProg);
        glUniformMatrix4fv(uF_view, 1, GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(uF_proj, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(gridBuf.vao);
        glDrawArrays(GL_LINES, 0, gridBuf.n);

        glBindVertexArray(axesBuf.vao);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, axesBuf.n);
        glLineWidth(1.0f);
        glBindVertexArray(0);

        // ── Draw cylinder (Phong shader) ──────────
        glUseProgram(phongProg);
        glUniformMatrix4fv(uP_model, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(uP_view,  1, GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(uP_proj,  1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(uP_light, 1, glm::value_ptr(lightPos));
        glUniform3fv(uP_vpos,  1, glm::value_ptr(cameraPos));
        glUniform3f (uP_color, 0.20f, 0.55f, 0.90f);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    // ── Cleanup ────────────────────────────────
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &gridBuf.vao);
    glDeleteBuffers(1,     &gridBuf.vbo);
    glDeleteVertexArrays(1, &axesBuf.vao);
    glDeleteBuffers(1,     &axesBuf.vbo);
    glDeleteProgram(phongProg);
    glDeleteProgram(flatProg);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}