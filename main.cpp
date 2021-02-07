#include <iostream>
#include <string>
#include <vector>
#include <mach-o/dyld.h>

#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "opengl_utilities.h"
#include "extras.h"

#define GLFW_KEY_RIGHT 262
#define GLFW_KEY_LEFT 263
#define GLFW_KEY_DOWN 264
#define GLFW_KEY_UP 265

#include "Camera.h"

/* Keep the global state inside this struct */
static struct {
    glm::dvec2 mouse_position;
    glm::ivec2 screen_dimensions = glm::ivec2(960, 960);
    int key_pressed;
    
    glm::vec3 cameraPos = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f,0.0f,60.606);
    glm::vec3 cameraUp = glm::vec3(0.0f,1.0f,0.0f);
    
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    
    float lastX = 400;
    float lastY= 300;
    
    bool firstMouse = true;
    
} Globals;

float player_scale = 0.001f;
float player_rotation = -90.f;
glm::vec3 player_pos(0, 1.f, 0);
//Camera camera(glm::vec3(0,0,0);
Camera camera(glm::vec3(player_scale, 1.f + player_scale *3, player_scale*15),player_pos);
glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 Right = glm::normalize(glm::cross(Front, glm::vec3(player_scale, 1.f + player_scale*3, player_scale*9)));

/* GLFW Callback functions */
static void ErrorCallback(int error, const char* description)
{
    std::cerr << "Error: " << description << std::endl;
}

static void CursorPositionCallback_(GLFWwindow* window, double x, double y)
{
    Globals.mouse_position.x = x;
    Globals.mouse_position.y = y;
}

static void CursorPositionCallback(GLFWwindow* window, double x, double y)
{
    Globals.mouse_position.x = x;
    Globals.mouse_position.y = y;
    
    if (Globals.firstMouse)
    {
        Globals.lastX = x;
        Globals.lastY = y;
        Globals.firstMouse = false;
    }

    float xoffset = x - Globals.lastX;
    float yoffset = Globals.lastY - y; // reversed since y-coordinates go from bottom to top

    Globals.lastX = x;
    Globals.lastY = y;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

static void WindowSizeCallback(GLFWwindow* window, int width, int height)
{
    Globals.screen_dimensions.x = width;
    Globals.screen_dimensions.y = height;

    glViewport(0, 0, width, height);
}

static void KeyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Globals.key_pressed = key;
}

void ScrollCallBack(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

bool CheckCollision(glm::vec3 &player_pos, glm::vec3 &enemy_pos) // AABB - AABB collision
{
    // collision x-axis?
    bool collisionX = player_pos.x + player_scale >= enemy_pos.x &&
        enemy_pos.x + player_scale >= player_pos.x;
    // collision y-axis?
    bool collisionZ = player_pos.z + player_scale >= enemy_pos.z &&
        enemy_pos.z + player_scale >= player_pos.z;
    // collision only if on both axes
    return collisionX && collisionZ;
}

int main(void)
{
    
//    std::cout << fs::current_path()<<std::endl;
    /* Set GLFW error callback */
    glfwSetErrorCallback(ErrorCallback);

    /* Initialize the library */
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    /* Create a windowed mode window and its OpenGL context */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(
        Globals.screen_dimensions.x, Globals.screen_dimensions.y,
        "Deniz Cangı", NULL, NULL
    );
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    /* Move window to a certain position [do not change] */
    glfwSetWindowPos(window, 10, 50);
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    /* Enable VSync */
//    glfwSwapInterval(1);

    /* Load OpenGL extensions with GLAD */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Set GLFW Callbacks */
    glfwSetCursorPosCallback(window, CursorPositionCallback_);
    glfwSetWindowSizeCallback(window, WindowSizeCallback);
    glfwSetKeyCallback(window, KeyCallBack);
    glfwSetScrollCallback(window, ScrollCallBack);


    /* Configure OpenGL */
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);

    /* Creating OpenGL objects */
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<GLuint> indices;

    GenerateParametricShapeFrom2D(positions, normals, uvs, indices, ParametricHalfCircle, 256, 256);
    VAO sphereVAO(positions, normals, uvs, indices);
    
    positions.clear();
    normals.clear();
    uvs.clear();
    indices.clear();
    
    GenerateParametricShapeFrom2D(positions, normals, uvs, indices, ParametricCircle, 16, 16);
    VAO torusVAO(positions, normals, uvs, indices);
    
    VAO cubeVAO(
    {
        { -0.5f, -0.5f, -0.5f },
        { 0.5f, -0.5f, -0.5f },
        { 0.5f,  0.5f, -0.5f },
        { 0.5f,  0.5f, -0.5f },
        { -0.5f,  0.5f, -0.5f },
        { -0.5f, -0.5f, -0.5f },
        { -0.5f, -0.5f,  0.5f },
        { 0.5f, -0.5f,  0.5f },
        { 0.5f,  0.5f,  0.5f },
        { 0.5f,  0.5f,  0.5f },
        { -0.5f,  0.5f,  0.5f },
        { -0.5f, -0.5f,  0.5f },
        { -0.5f,  0.5f,  0.5f },
        { -0.5f,  0.5f, -0.5f },
        { -0.5f, -0.5f, -0.5f },
        { -0.5f, -0.5f, -0.5f },
        { -0.5f, -0.5f,  0.5f },
        { -0.5f,  0.5f,  0.5f },
        { 0.5f,  0.5f,  0.5f },
        { 0.5f,  0.5f, -0.5f },
        { 0.5f, -0.5f, -0.5f },
        { 0.5f, -0.5f, -0.5f },
        { 0.5f, -0.5f,  0.5f },
        { 0.5f,  0.5f,  0.5f },
        { -0.5f, -0.5f, -0.5f },
        { 0.5f, -0.5f, -0.5f },
        { 0.5f, -0.5f,  0.5f },
        { 0.5f, -0.5f,  0.5f },
        { -0.5f, -0.5f,  0.5f },
        { -0.5f, -0.5f, -0.5f },
        { -0.5f,  0.5f, -0.5f },
        { 0.5f,  0.5f, -0.5f },
        { 0.5f,  0.5f,  0.5f },
        { 0.5f,  0.5f,  0.5f },
        { -0.5f,  0.5f,  0.5f },
        { -0.5f,  0.5f, -0.5f }
    },
    {
        { 0.0f,  0.0f, -1.0f },
        { 0.0f,  0.0f, -1.0f },
        { 0.0f,  0.0f, -1.0f },
        { 0.0f,  0.0f, -1.0f },
        { 0.0f,  0.0f, -1.0f },
        { 0.0f,  0.0f, -1.0f },
        { 0.0f,  0.0f,  1.0f },
        { 0.0f,  0.0f,  1.0f },
        { 0.0f,  0.0f,  1.0f },
        { 0.0f,  0.0f,  1.0f },
        { 0.0f,  0.0f,  1.0f },
        { 0.0f,  0.0f,  1.0f },
        { -1.0f,  0.0f,  0.0f },
        { -1.0f,  0.0f,  0.0f },
        { -1.0f,  0.0f,  0.0f },
        { -1.0f,  0.0f,  0.0f },
        { -1.0f,  0.0f,  0.0f },
        { -1.0f,  0.0f,  0.0f },
        { 1.0f,  0.0f,  0.0f },
        { 1.0f,  0.0f,  0.0f },
        { 1.0f,  0.0f,  0.0f },
        { 1.0f,  0.0f,  0.0f },
        { 1.0f,  0.0f,  0.0f },
        { 1.0f,  0.0f,  0.0f },
        { 0.0f, -1.0f,  0.0f },
        { 0.0f, -1.0f,  0.0f },
        { 0.0f, -1.0f,  0.0f },
        { 0.0f, -1.0f,  0.0f },
        { 0.0f, -1.0f,  0.0f },
        { 0.0f, -1.0f,  0.0f },
        { 0.0f,  1.0f,  0.0f },
        { 0.0f,  1.0f,  0.0f },
        { 0.0f,  1.0f,  0.0f },
        { 0.0f,  1.0f,  0.0f },
        { 0.0f,  1.0f,  0.0f },
        { 0.0f,  1.0f,  0.0f }
    },
    {
    },
    {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
        9, 10, 11,
        12, 13, 14,
        15, 16, 17,
        18, 19, 20,
        21, 22, 23,
        24, 25, 26,
        27, 28, 29,
        30, 31, 32,
        33, 34, 35
    }
    );
    
    char path[2048];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0)
        printf("executable path is %s\n", path);
    else
        printf("buffer too small; need size %u\n", size);
    
    stbi_set_flip_vertically_on_load(true);
    /*
    const std::string filename = fs::absolute(fs::current_path()).c_str();
    std::cout << filename << std::endl;
    
    char buff[FILENAME_MAX]; //create string buffer to hold path
    GetCurrentDir(buff, FILENAME_MAX);
    std::string current_working_dir(buff);*/
    /*
    std::string path_s(path);
//    std::cout << path_s.rfind("/") << std::endl;
    path_s = path_s.substr(0,path_s.rfind("/"));
    std::string filename = path_s + "/denizcangi_mars_texture.jpg";
//    std::cout << filename << std::endl;
//    std::cout << current_working_dir << std::endl;
    char* char_array;
    char_array = &filename[0];*/
    
    auto filename = "denizcangi_mars_texture.jpg";
    
    int x, y, n;
    unsigned char *texture_data_1 = stbi_load(filename, &x, &y, &n, 0);
    if (texture_data_1 == NULL)
    {
        std::cout << "Texture " << filename << " failed to load." << std::endl;
        std::cout << "Error: " << stbi_failure_reason() << std::endl;
        if(stbi_failure_reason())
            std::cout << stbi_failure_reason();
    }
    else
    {
        std::cout << "Texture " << filename << " is loaded, X:" << x << " Y:" << y << " N:" << n << std::endl;
    }

    GLuint texture_1;
    glGenTextures(1, &texture_1);
    
    if (x * n % 4 != 0){
        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    }

    glBindTexture(GL_TEXTURE_2D, texture_1);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data_1
    ); //texture

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(texture_data_1);


    GLuint program = CreateProgramFromSources(
        R"VERTEX(
#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_projection_view;
                                              
out vec4 world_space_position;
out vec3 world_space_normal;
out vec2 vertex_uv;
                                              
void main()
{
    world_space_position = u_model * vec4(a_position, 1);
    world_space_normal = vec3(u_model * vec4(a_normal, 0));
    vertex_uv = a_uv;
    
    gl_Position = u_projection_view * world_space_position;
}
        )VERTEX",

        R"FRAGMENT(
#version 330 core

uniform vec2 u_mouse_position;
uniform float u_material;
uniform sampler2D u_texture;

in vec4 world_space_position;
in vec3 world_space_normal;
in vec2 vertex_uv;
                                              
out vec4 out_color;

void main()
{
    vec3 color = vec3(0);

    vec3 surface_position = world_space_position.xyz;
    vec3 surface_normal = normalize(world_space_normal);
    vec2 surface_uv = vertex_uv;
    vec3 surface_color;
    if (u_material == 1)
    {
        surface_color = vec3(255/255.f, 241/255.f, 38/255.f);
    }
    else if (u_material == 2)
    {
        surface_color = texture(u_texture, surface_uv).rgb;
    }
    else if (u_material == 3)
    {
        surface_color = vec3(0);
    }
    else if (u_material == 4)
    {
        surface_color = vec3(150/255.f, 30/255.f, 30/255.f);
    }
    else if (u_material == 5)
    {
        surface_color = vec3(0/255.f, 153/255.f, 0/255.f);
    }
    vec3 ambient_color = vec3(0.5);
                                    
    vec3 light_direction = normalize(vec3(1,1,-1));
    vec3 light_color = vec3(0.35);
                                    
    float diffuse_intensity = max(0,dot(light_direction, surface_normal));

    vec3 view_dir = vec3(0,0,-1);
    vec3 halfway_dir = normalize(view_dir + light_direction);
    float shininess =  64;
    float specular_intensity = max(0, dot(halfway_dir, surface_normal));
                                        
    color += ambient_color * surface_color + diffuse_intensity * light_color * surface_color + pow(specular_intensity, shininess) * light_color;
                                 
    out_color = vec4(color, 1);
}
        )FRAGMENT");
    if (program == NULL)
    {
        glfwTerminate();
        return -1;
    }
    glUseProgram(program);

    auto texture_location = glGetUniformLocation(program, "u_texture"); //texture
    glUniform1i(texture_location, 0);
    
    glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, texture_1);
    
    auto mouse_location = glGetUniformLocation(program, "u_mouse_position");
    auto model_location = glGetUniformLocation(program, "u_model");
    auto projection_view_location = glGetUniformLocation(program, "u_projection_view");
    
    auto material_location = glGetUniformLocation(program, "u_material");
    
    //Camera parameters
    
    float aspect = 1.f, near = 0.000001f, far = 100000.f;
    
    glm::vec3 sphere_pos(0,0,0);
    float sphere_scale = 1.f;
    
    glm::vec3 player_pos(player_scale, 1.f + player_scale, 0);
    
    auto player_translate = glm::translate(player_pos);
    auto player_rotation = glm::rotate(glm::radians(0.f), glm::vec3(1,1.f,1.f));
    auto player_scaling = glm::scale(glm::vec3(player_scale));
    auto player_transform = player_translate * player_scaling * player_rotation;
    
    auto enemy_1_pos = glm::vec3(player_pos + glm::vec3(10,0,-20) * player_scale);
    auto enemy_1_translate = glm::translate(enemy_1_pos);
    auto enemy_1_rotation = glm::rotate(glm::radians(0.f), glm::vec3(1,1.f,1.f));
    auto enemy_1_scaling = glm::scale(glm::vec3(player_scale));
    
    auto enemy_2_pos = glm::vec3(player_pos + glm::vec3(-10,0,-20) * player_scale);
    auto enemy_2_translate = glm::translate(enemy_2_pos);
    auto enemy_2_rotation = glm::rotate(glm::radians(0.f), glm::vec3(1,1.f,1.f));
    auto enemy_2_scaling = glm::scale(glm::vec3(player_scale));
    
    Camera savedCamera;
    bool goOn = true;
    bool moveForward = true;
    bool action = false;
    bool collision = false;
    
    auto enemy1_saved_pos = enemy_1_pos;
    auto enemy2_saved_pos = enemy_2_pos;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        if(goOn == true){
            if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
                camera.ProcessKeyboard(FORWARD, Globals.deltaTime);
                auto z = Globals.deltaTime * SPEED;
                player_pos = player_pos + Front * z;
                player_translate = glm::translate(player_pos);
                player_transform = player_translate * player_scaling * player_rotation;
                moveForward= true;
                action= true;
            }
            if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
                camera.ProcessKeyboard(BACKWARD, Globals.deltaTime);
                auto z = Globals.deltaTime * SPEED;
                player_pos = player_pos - Front * z;
                player_translate = glm::translate(player_pos);
                player_transform = player_translate * player_scaling * player_rotation;
                moveForward= false;
                action= true;
            }
            if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
                camera.ProcessKeyboard(RIGHT, Globals.deltaTime);
                auto z = Globals.deltaTime * SPEED;
                player_pos = player_pos + Right * z;
                player_translate = glm::translate(player_pos);
                player_transform = player_translate * player_scaling * player_rotation;
            }
            if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
                camera.ProcessKeyboard(LEFT, Globals.deltaTime);
                auto z = Globals.deltaTime * SPEED;
                player_pos = player_pos - Right * z;
                player_translate = glm::translate(player_pos);
                player_transform = player_translate * player_scaling * player_rotation;
            }
            if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
                savedCamera = camera;
                goOn = false;
                glfwSetCursorPosCallback(window, CursorPositionCallback);
            }

        }
        
        if(goOn == false){
            
            if(glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS){
                goOn = true;
                camera = savedCamera;
                glfwSetCursorPosCallback(window, CursorPositionCallback_);
            }
            if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
                camera.ProcessKeyboard(FORWARD, Globals.deltaTime);
            }
            if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
                camera.ProcessKeyboard(BACKWARD, Globals.deltaTime);
            }
            if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
                camera.ProcessKeyboard(RIGHT, Globals.deltaTime);
            }
            if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
                camera.ProcessKeyboard(LEFT, Globals.deltaTime);
            }

        }

        float currentFrame = glfwGetTime();
        Globals.deltaTime = currentFrame - Globals.lastFrame;
        Globals.lastFrame = currentFrame;
        
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        

        // Calculate mouse position
        auto mouse_position = Globals.mouse_position;
        mouse_position /= glm::dvec2(Globals.screen_dimensions);
        mouse_position.y = 1. - mouse_position.y;
        mouse_position = mouse_position * 2. - 1.;

        glUniform2fv(mouse_location, 1, glm::value_ptr(glm::vec2(mouse_position)));
        
//        auto camera_transform = glm::translate(glm::vec3(mouse_position,0));
//        camera_transform = glm::inverse(camera_transform);
        
        auto view = camera.GetViewMatrix();
    
//        auto projection = glm::ortho(-5.f,5.f,-1.f,1.f,-1.f,1.f);

        
        auto projection = glm::perspective(glm::radians(camera.Zoom), aspect, near, far);
        
        auto view_projection = projection * view;//        glm::perspective(1,1,1,1);

        // Draw Mars
        glBindVertexArray(sphereVAO.id);

        auto mars_scale = glm::scale(glm::vec3(sphere_scale));
        auto mars_translate = glm::translate(sphere_pos);
        auto mars_rotate = glm::rotate(glm::radians(90.f), glm::vec3(1, 0.f, 0.f));
        auto mars_transform = mars_translate * mars_scale * mars_rotate;
        
        glUniformMatrix4fv(projection_view_location, 1, GL_FALSE, glm::value_ptr(view_projection));
        glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(mars_transform));
        glUniform1f(material_location,2);
        glDrawElements(GL_TRIANGLES, sphereVAO.element_array_count, GL_UNSIGNED_INT, NULL);
        
        //Draw Rover
        
        glBindVertexArray(cubeVAO.id);
        
        glUniformMatrix4fv(projection_view_location, 1, GL_FALSE, glm::value_ptr(view_projection));
        glUniformMatrix4fv(model_location,1,GL_FALSE, glm::value_ptr(player_transform));
        
        if(CheckCollision(player_pos, enemy_1_pos) || CheckCollision(player_pos, enemy_2_pos)){
            glUniform1f(material_location, 3);
            glfwSetCursorPosCallback(window, CursorPositionCallback);
            goOn = false;
            collision = true;
        }
        else
        {
            glUniform1f(material_location, 1);
        }
        glDrawElements(GL_TRIANGLES, cubeVAO.element_array_count, GL_UNSIGNED_INT, NULL);
        
        //Draw tiers
//
        glBindVertexArray(torusVAO.id);

        const auto draw_tire = [&](glm::vec3 position)
        {
            auto tire_scaling = glm::scale(glm::vec3(0.3));
            auto tire_translate = glm::translate(glm::vec3(position));
            auto tire_transform = player_transform * tire_translate * tire_scaling * glm::rotate(glm::radians(90.f), glm::vec3(0,0,1));

            if (moveForward && action) {
                tire_transform *= glm::rotate(glm::radians(float(glfwGetTime()) *1000.f), glm::vec3(0,1,0));
            }
            if (!moveForward && action) {
                tire_transform *= glm::rotate(glm::radians(float(glfwGetTime()) *1000.f), glm::vec3(0,-1,0));
            }
            
            glUniformMatrix4fv(model_location,1,GL_FALSE, glm::value_ptr(tire_transform));
            glUniform1f(material_location, 3);
            glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);
            
        };
        
        std::vector<glm::vec3> tire_positions{
            glm::vec3(0.58,-0.5,0.5),
            glm::vec3(0.58,-0.5,-0.5),
            glm::vec3(-0.58,-0.5,0.5),
            glm::vec3(-0.58,-0.5,-0.5)
        };
        
        for (const auto& p : tire_positions){
            draw_tire(p);
        }
        

        glBindVertexArray(cubeVAO.id);
        
        if (!collision){
            glm::dvec2 chasing_pos;
            chasing_pos = glm::mix(glm::dvec2(player_pos.x, player_pos.z), glm::dvec2(enemy_1_pos.x, enemy_1_pos.z), 0.999);
            enemy_1_pos = glm::vec3(chasing_pos.x, enemy_1_pos.y, chasing_pos.y);
            enemy1_saved_pos = enemy_1_pos;
        }
        
        else if (collision){
            enemy_1_pos = enemy1_saved_pos;
        }
        enemy_1_translate = glm::translate(enemy_1_pos);
        auto transform_1 = enemy_1_translate * enemy_1_scaling * enemy_1_rotation;
        
                    
        glUniformMatrix4fv(projection_view_location, 1, GL_FALSE, glm::value_ptr(view_projection));
        glUniformMatrix4fv(model_location,1,GL_FALSE, glm::value_ptr(transform_1));
        if (!collision){
            glUniform1f(material_location, 4);
        }
        else if(collision){
            glUniform1f(material_location, 5);
        }
        glDrawElements(GL_TRIANGLES, cubeVAO.element_array_count, GL_UNSIGNED_INT, NULL);
                    
                    //Draw tiers
            //
        glBindVertexArray(torusVAO.id);

        const auto draw_tire_enemy = [&](glm::vec3 position)
        {
            auto tire_scaling = glm::scale(glm::vec3(0.3));
            auto tire_translate = glm::translate(glm::vec3(position));
            auto tire_transform = transform_1 * tire_translate * tire_scaling * glm::rotate(glm::radians(90.f), glm::vec3(0,0,1));

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                tire_transform *= glm::rotate(glm::radians(float(glfwGetTime()) *1000.f), glm::vec3(0,1,0));
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                tire_transform *= glm::rotate(glm::radians(float(glfwGetTime()) *1000.f), glm::vec3(0,-1,0));
            }
                        
            glUniformMatrix4fv(model_location,1,GL_FALSE, glm::value_ptr(tire_transform));
            glUniform1f(material_location, 3);
            glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);
                        
        };
                    
        std::vector<glm::vec3> enemy_tire_positions_2{
            glm::vec3(0.58,-0.5,0.5),
            glm::vec3(0.58,-0.5,-0.5),
            glm::vec3(-0.58,-0.5,0.5),
            glm::vec3(-0.58,-0.5,-0.5)
        };
                    
        for (const auto& p : enemy_tire_positions_2){
            draw_tire_enemy(p);
        }
        
        glBindVertexArray(cubeVAO.id);
        
        if (!collision){
            glm::dvec2 chasing_pos_2;
            chasing_pos_2 = glm::mix(glm::dvec2(player_pos.x, player_pos.z), glm::dvec2(enemy_2_pos.x, enemy_2_pos.z), 0.999);
            enemy_2_pos = glm::vec3(chasing_pos_2.x, enemy_2_pos.y, chasing_pos_2.y);
            enemy2_saved_pos = enemy_2_pos;
        }
        
        else if (collision){
            enemy_2_pos = enemy2_saved_pos;
        }
        
        enemy_2_translate = glm::translate(enemy_2_pos);
        auto transform_2 = enemy_2_translate * enemy_2_scaling * enemy_2_rotation;
                    
        glUniformMatrix4fv(projection_view_location, 1, GL_FALSE, glm::value_ptr(view_projection));
        glUniformMatrix4fv(model_location,1,GL_FALSE, glm::value_ptr(transform_2));
        if (!collision){
            glUniform1f(material_location, 4);
        }
        else if(collision){
            glUniform1f(material_location, 5);
        }
        glDrawElements(GL_TRIANGLES, cubeVAO.element_array_count, GL_UNSIGNED_INT, NULL);
                    
                    //Draw tiers
            //
        glBindVertexArray(torusVAO.id);

        const auto draw_tire_enemy2 = [&](glm::vec3 position)
        {
            auto tire_scaling = glm::scale(glm::vec3(0.3));
            auto tire_translate = glm::translate(glm::vec3(position));
            auto tire_transform = transform_2 * tire_translate * tire_scaling * glm::rotate(glm::radians(90.f), glm::vec3(0,0,1));

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                tire_transform *= glm::rotate(glm::radians(float(glfwGetTime()) *1000.f), glm::vec3(0,1,0));
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                tire_transform *= glm::rotate(glm::radians(float(glfwGetTime()) *1000.f), glm::vec3(0,-1,0));
            }
                        
            glUniformMatrix4fv(model_location,1,GL_FALSE, glm::value_ptr(tire_transform));
            glUniform1f(material_location, 3);
            glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);
                        
        };
                    
        std::vector<glm::vec3> enemy_tire_positions_3{
            glm::vec3(0.58,-0.5,0.5),
            glm::vec3(0.58,-0.5,-0.5),
            glm::vec3(-0.58,-0.5,0.5),
            glm::vec3(-0.58,-0.5,-0.5)
        };
                    
        for (const auto& p : enemy_tire_positions_3){
            draw_tire_enemy2(p);
        }
        
        moveForward = false;
        action= false;

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
