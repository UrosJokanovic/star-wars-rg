#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 12.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

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
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Star Wars: Episode V - The Empire Strikes Back", NULL, NULL);
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

    // build and compile shaders
    // -------------------------
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");

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

    glm::vec3 destroyerPositions[] = {
            glm::vec3(0.0f, 30.0f, -120.0f),
            glm::vec3(180.0f, 0.0f, -450.0f),
            glm::vec3(-250.0f, -10.0f, -400.0f),
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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

    PointLight pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;



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
        ourShader.use();
        pointLight.position = glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));
        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.constant", pointLight.constant);
        ourShader.setFloat("pointLight.linear", pointLight.linear);
        ourShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition", camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 1500.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render imperial bombers
        for (unsigned int i = 0 ; i < 11 ; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model,
                                   bomberPositions[i]); // translate it down so it's at the center of the scene
            model = glm::rotate(model, glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, (float) glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(0.5f));    // it's a bit too big for our scene, so scale it down
            ourShader.setMat4("model", model);
            bomber.Draw(ourShader);
        }

        // render millenium falcon
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -20.0f, 140.0f));
        model = glm::rotate(model, (float)sin(glfwGetTime()), glm::vec3(0.0f, 0.0f, 0.5f));
        model = glm::rotate(model, glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.025f));
        ourShader.setMat4("model", model);
        milleniumFalcon.Draw(ourShader);

        // render imperial fighter
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 5.5f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-25.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.05f));
        ourShader.setMat4("model", model);
        tieFighter.Draw(ourShader);

        // render death star
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1300.0f));
        model = glm::rotate(model, (float)glfwGetTime()/20, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.4f));
        ourShader.setMat4("model", model);
        deathStar.Draw(ourShader);

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
            model = glm::scale(model, glm::vec3(0.5f));
            ourShader.setMat4("model", model);
            starDestroyer.Draw(ourShader);
        }

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