#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadCubemap(vector<std::string> faces);
unsigned int loadTexture(char const * path);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
bool blinn = false;
bool blinnKeyPressed = false;
bool flashLight = false;
bool flashLightKeyPressed = false;
bool faceCullingKeyPressed = false;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 30.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float cubeMoveLR = 0.0f;
float cubeMoveUD = 0.0f;
float cubeRotate = 0.0f;

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Star Wars", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
//    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // build and compile shaders
    // -------------------------
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader sceneLight("resources/shaders/scene_light.vs", "resources/shaders/scene_light.fs");
    Shader swCube("resources/shaders/cube_discard.vs", "resources/shaders/cube_discard.fs");

    // star wars cube coordinates
    float vertices[] = {
            // positions          // normals           // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    // skybox coordinates
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    // model positions
    // imperial bomber positions
    glm::vec3 bomberPositions[] = {
            glm::vec3( 0.0f,  0.0f,  12.0f),
            glm::vec3( 3.0f,  0.0f, 6.0f),
            glm::vec3(0.0f, 3.0f, 0.0f),
            glm::vec3(-3.0f, 0.0f, 5.0f),
            glm::vec3( 0.0f, -3.0f, 8.0f),
            glm::vec3(3.0f,  3.0f, 10.0f),
            glm::vec3( -3.0f, -3.0f, 9.0f),
            glm::vec3(3.0f,  -3.0f, 15.0f),
            glm::vec3( -3.0f, 3.0f, 14.0f),
            glm::vec3(1.5f, 0.0f,20.0f),
            glm::vec3(-1.5f, 0.0f,22.0f)
    };
    // imperial destroyer positions
    glm::vec3 destroyerPositions[] = {
            glm::vec3(0.0f, 30.0f, -140.0f),
            glm::vec3(180.0f, 0.0f, -450.0f),
            glm::vec3(-250.0f, -10.0f, -400.0f),
    };

    // imperial fighter positions
    glm::vec3 fighterPositions[] = {
            glm::vec3(0.0f, 0.0f, 5.5f),
            glm::vec3(5.0f, 5.0f, 8.0f),
            glm::vec3(-4.0f, -6.0f, 10.0f),
            glm::vec3(0.0f, 0.0f, 25.0f),
            glm::vec3(0.0f, -3.0f, -3.0f)
    };

    // rebel x-wing starfighter
    glm::vec3 xWingPositions[] = {
            glm::vec3(25.0f, -20.0f, 140.0f),
            glm::vec3(-25.0f, -15.0f, 130.f),
            glm::vec3(0.0f, -5.0f, 135.0f)
    };

    // star wars cube setup
    unsigned int swcubeVAO, swcubeVBO;
    glGenVertexArrays(1, &swcubeVAO);
    glGenBuffers(1, &swcubeVBO);

    glBindVertexArray(swcubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, swcubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // skybox setup
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // star wars cube texture
    unsigned int dartVader = loadTexture(FileSystem::getPath("resources/textures/darth_vader.png").c_str());

    //skybox textures
    vector<std::string> spaceSkybox = {
            FileSystem::getPath("resources/textures/space_skybox/space_right.png"),
            FileSystem::getPath("resources/textures/space_skybox/space_left.png"),
            FileSystem::getPath("resources/textures/space_skybox/space_top.png"),
            FileSystem::getPath("resources/textures/space_skybox/space_bottom.png"),
            FileSystem::getPath("resources/textures/space_skybox/space_back.png"),
            FileSystem::getPath("resources/textures/space_skybox/space_front.png")
    };

    unsigned int cubemapTexture = loadCubemap(spaceSkybox);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // load models
    // -----------
    Model bomber("resources/objects/sw_bomber/tieBomber.obj");
    bomber.SetShaderTextureNamePrefix("material.");

    Model milleniumFalcon("resources/objects/sw_millenium_falcon/Halcon_Milenario.obj");
    milleniumFalcon.SetShaderTextureNamePrefix("material.");

    Model tieFighter("resources/objects/sw_fighter/tie_fighter.obj");
    tieFighter.SetShaderTextureNamePrefix("material.");

    Model deathStar("resources/objects/sw_death_star/DeathStar.obj");
    deathStar.SetShaderTextureNamePrefix("material.");

    Model starDestroyer("resources/objects/sw_star_destroyer/star_destroyer.obj");
    starDestroyer.SetShaderTextureNamePrefix("material.");

    Model xWingStarFighter("resources/objects/sw_x_wing/x-wing-flyingv1.obj");
    xWingStarFighter.SetShaderTextureNamePrefix("material.");

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame + 0.2f;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        sceneLight.use();
        sceneLight.setVec3("viewPos", camera.Position);
        sceneLight.setFloat("material.shininess", 16.0f);
        sceneLight.setInt("blinn", blinn);
        sceneLight.setInt("flashLight", flashLight);

        // directional light
        sceneLight.setVec3("dirLight.direction", glm::vec3(100.0f, -250.0f, -50.0f));
        sceneLight.setVec3("dirLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
        sceneLight.setVec3("dirLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
        sceneLight.setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        
        // point light
        sceneLight.setVec3("pointLight.position", glm::vec3(0.0f, 0.0f, 10.0f));
        sceneLight.setVec3("pointLight.ambient", glm::vec3(0.5, 0.5, 0.5));
        sceneLight.setVec3("pointLight.diffuse", glm::vec3(0.6, 0.6, 0.6));
        sceneLight.setVec3("pointLight.specular", glm::vec3(1.0, 1.0, 1.0));
        sceneLight.setFloat("pointLight.constant", 1.0f);
        sceneLight.setFloat("pointLight.linear", 0.09f);
        sceneLight.setFloat("pointLight.quadratic", 0.032f);
        
        // spot light
        sceneLight.setVec3("spotLight.position", camera.Position);
        sceneLight.setVec3("spotLight.direction", camera.Front);
        sceneLight.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        sceneLight.setVec3("spotLight.diffuse", 0.7f, 0.7f, 0.7f);
        sceneLight.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        sceneLight.setFloat("spotLight.constant", 1.0f);
        sceneLight.setFloat("spotLight.linear", 0.05);
        sceneLight.setFloat("spotLight.quadratic", 0.012);
        sceneLight.setFloat("spotLight.cutOff", glm::cos(glm::radians(10.5f)));
        sceneLight.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(13.0f)));
        
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 1300.0f);
        glm::mat4 view = camera.GetViewMatrix();
        sceneLight.setMat4("projection", projection);
        sceneLight.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        sceneLight.setMat4("model", model);

        // render imperial bombers
        for (unsigned int i = 0 ; i < 11 ; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model,
                                   bomberPositions[i]);
            model = glm::rotate(model, glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, (float) glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(0.85f));
            sceneLight.setMat4("model", model);
            bomber.Draw(sceneLight);
        }

        // render millenium falcon
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, cos(glfwGetTime())+(-20.0f), sin(glfwGetTime())+140.0f));
        model = glm::rotate(model, (float)sin(glfwGetTime()), glm::vec3(0.0f, 0.0f, 0.5f));
        model = glm::rotate(model, glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.025f));
        sceneLight.setMat4("model", model);
        milleniumFalcon.Draw(sceneLight);

        glDisable(GL_CULL_FACE);
        // render imperial fighter
        for (unsigned int i = 0 ; i < 5 ; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, fighterPositions[i]);
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-25.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(0.05f));
            sceneLight.setMat4("model", model);
            tieFighter.Draw(sceneLight);
        }
        glEnable(GL_CULL_FACE);

        // render death star
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1300.0f));
        model = glm::rotate(model, (float)glfwGetTime()/50, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.4f));
        sceneLight.setMat4("model", model);
        deathStar.Draw(sceneLight);

        // render star destroyer
        for (unsigned int i = 0 ; i < 3 ; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, destroyerPositions[i]);
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            if (i == 1) {
                model = glm::rotate(model, glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            if (i == 2) {
                model = glm::rotate(model, glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            model = glm::scale(model, glm::vec3(0.6f));
            sceneLight.setMat4("model", model);
            starDestroyer.Draw(sceneLight);
        }

        // render x wing star fighter
        // x wing star fighter objekti su kompleksniji i kada se ukljuce zahtevaju vise vremena pokretanje tj
        // iskace prozor (You may choose to wait a short while to continue or force the application to quit)
        for (unsigned int i = 0 ; i < 3 ; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, xWingPositions[i]);
            model = glm::rotate(model, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.35f));
            sceneLight.setMat4("model", model);
            xWingStarFighter.Draw(sceneLight);
        }

        // star wars cube
        glDisable(GL_CULL_FACE);
        glm::mat4 cube = glm::mat4(1.0f);
        swCube.use();
        swCube.setMat4("projection", projection);
        swCube.setMat4("view", view);
        swCube.setMat4("model", cube);
        glBindVertexArray(swcubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, dartVader);
        cube = glm::mat4(1.0f);
//        cube = glm::translate(cube, glm::vec3(0.0f + cubeMoveLR, 0.0f + cubeMoveUD, -15.0f));
        cube = glm::translate(cube, camera.Position + glm::vec3(0.0f));
        cube = glm::rotate(cube, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        cube = glm::rotate(cube, glm::radians(cubeRotate), glm::vec3(0.0f, 0.0f, 1.0f));
        cube = glm::scale(cube, glm::vec3(2.0f));
        swCube.setMat4("model", cube);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glEnable(GL_CULL_FACE);

        // skybox setup
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        // render skybox
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &swcubeVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &swcubeVBO);
    glDeleteBuffers(1, &skyboxVBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        cubeMoveLR += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        cubeMoveLR -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        cubeMoveUD += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        cubeMoveUD -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cubeRotate += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        cubeRotate -= 1.0f;

    // Blinn-Phong light key
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinnKeyPressed)
    {
        blinn = !blinn;
        blinnKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        blinnKeyPressed = false;
    }

    // Flash light key
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !flashLightKeyPressed)
    {
        flashLight = !flashLight;
        flashLightKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
    {
        flashLightKeyPressed = false;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RED;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}