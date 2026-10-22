#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <glm/ext/matrix_float2x2_precision.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <ctime>

#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// --- CALLBACKS ----------------------------------------------------------

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
}

void glfw_error_callback(int error, const char *description)
{
  std::cerr << "GLFW Error (" << error << "): " << description << "\n";
}

// Helper structure to hold generated mesh payloads
struct MeshData
{
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
};

// Example function that generates a customized plane/grid at runtime
MeshData generatePlane(int gridWidth, int gridHeight)
{
  MeshData data;
  std::srand(std::time(0));

  int width, height, channels;

  // Force 4 channels (RGBA) by passing 4 as the last argument
  unsigned char *img = stbi_load("noise.png", &width, &height, &channels, 4);

  if (img == nullptr)
  {
    std::cerr << "Error loading image" << std::endl;
  }

  // 1. Generate Vertices dynamically
  for (int z = 0; z <= gridHeight; ++z)
  {
    for (int x = 0; x <= gridWidth; ++x)
    {

      // Each pixel takes up 4 bytes: Red, Green, Blue, Alpha
      int pixel_index = (z * width + x) * 4;

      int r = img[pixel_index + 0];
      int g = img[pixel_index + 1];
      int b = img[pixel_index + 2];
      int a = img[pixel_index + 3]; // This is your alpha value (0-255)

      // X position, Y position, Z position
      data.vertices.push_back((float)x / gridWidth - 0.5f);
      float y = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
      data.vertices.push_back(y * 2 * (static_cast<float>(a) / 255.0f) / gridWidth);
      data.vertices.push_back((float)z / gridHeight - 0.5f);

      data.vertices.push_back(r);
      data.vertices.push_back(g);
      data.vertices.push_back(b);
    }
  }
  stbi_image_free(img);

  // 2. Generate Indices dynamically (with strict CCW winding order)
  for (int y = 0; y < gridHeight; ++y)
  {
    for (int x = 0; x < gridWidth; ++x)
    {
      int row1 = y * (gridWidth + 1);
      int row2 = (y + 1) * (gridWidth + 1);

      // Triangle 1 (CCW)
      data.indices.push_back(row1 + x);
      data.indices.push_back(row1 + x + 1);
      data.indices.push_back(row2 + x);

      // Triangle 2 (CCW)
      data.indices.push_back(row1 + x + 1);
      data.indices.push_back(row2 + x + 1);
      data.indices.push_back(row2 + x);
    }
  }

  return data;
}

int main()
{
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
  {
    std::cerr << "Failed to init GLFW\n";
    return -1;
  }

  // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwDefaultWindowHints();

  GLFWwindow *window =
      glfwCreateWindow(800, 600, "OpenGL Visualizer", nullptr, nullptr);
  if (!window)
  {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  GLenum err = glewInit();

  // Bypass GLEW's false-positive "Unknown error" quirk on WSL/Mesa
  if (err != GLEW_OK && std::string((const char *)glewGetErrorString(err)) != "Unknown error")
  {
    std::cerr << "Failed to init GLEW: " << glewGetErrorString(err) << "\n";
    return -1;
  }

  std::cout << "GLEW initialized successfully (or bypassed false positive)!\n";

  glEnable(GL_DEPTH_TEST);
  // glEnable(GL_CULL_FACE);
  // glCullFace(GL_BACK);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  MeshData terrain = generatePlane(64, 64);

  // --- GEOMETRY DATA (A single unit square blueprint) ---
  // float verts[] = {
  //     // Front face (Z = 0.5)
  //     -0.5f, 0.5f, 0.5f,  // 0: Top-Left
  //     0.5f, 0.5f, 0.5f,   // 1: Top-Right
  //     0.5f, -0.5f, 0.5f,  // 2: Bottom-Right
  //     -0.5f, -0.5f, 0.5f, // 3: Bottom-Left
  //     // Back face (Z = -0.5)
  //     -0.5f, 0.5f, -0.5f, // 4: Top-Left-Back
  //     0.5f, 0.5f, -0.5f,  // 5: Top-Right-Back
  //     0.5f, -0.5f, -0.5f, // 6: Bottom-Right-Back
  //     -0.5f, -0.5f, -0.5f // 7: Bottom-Left-Back
  // };

  // thanks ai for fixing my winding order
  // unsigned int indices[] = {
  //     // Front face (Z = 0.5) - Looking straight at it
  //     0, 3, 2, // Top-Left -> Bottom-Left -> Bottom-Right
  //     0, 2, 1, // Top-Left -> Bottom-Right -> Top-Right

  //     // Back face (Z = -0.5) - Looking from behind
  //     5, 6, 7, // Top-Right-Back -> Bottom-Right-Back -> Bottom-Left-Back
  //     5, 7, 4, // Top-Right-Back -> Bottom-Left-Back -> Top-Left-Back

  //     // Left face (X = -0.5) - Looking from the left side
  //     4, 7, 3, // Top-Left-Back -> Bottom-Left-Back -> Bottom-Left
  //     4, 3, 0, // Top-Left-Back -> Bottom-Left -> Top-Left

  //     // Right face (X = 0.5) - Looking from the right side
  //     1, 2, 6, // Top-Right -> Bottom-Right -> Bottom-Right-Back
  //     1, 6, 5, // Top-Right -> Bottom-Right-Back -> Top-Right-Back

  //     // Top face (Y = 0.5) - Looking from above
  //     4, 0, 1, // Top-Left-Back -> Top-Left -> Top-Right
  //     4, 1, 5, // Top-Left-Back -> Top-Right -> Top-Right-Back

  //     // Bottom face (Y = -0.5) - Looking from below
  //     3, 7, 6, // Bottom-Left -> Bottom-Left-Back -> Bottom-Right-Back
  //     3, 6, 2  // Bottom-Left -> Bottom-Right-Back -> Bottom-Right
  // };

  unsigned int VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER,
               terrain.vertices.size() * sizeof(float), // Dynamic sizing check
               terrain.vertices.data(),                 // Pointer to underlying array
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               terrain.indices.size() * sizeof(unsigned int),
               terrain.indices.data(),
               GL_STATIC_DRAW);

  // glBindBuffer(GL_ARRAY_BUFFER, VBO);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
  //              GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  Shader shader("shaders/vertex.glsl", "shaders/frag.glsl");

  // --- CACHE UNIFORM LOCATIONS (Performance optimization) ---
  const int projLoc = glGetUniformLocation(shader.ID, "projection");
  const int viewLoc = glGetUniformLocation(shader.ID, "view");
  const int modelLoc = glGetUniformLocation(shader.ID, "model");
  const int colorLoc =
      glGetUniformLocation(shader.ID, "uColor"); // Cache color uniform

  float cameraAngle = 0.0f;
  float cameraYAngle = glm::radians(-25.0f);
  float cameraDistance = 3.0f;
  float cameraSpeed = 2.0f;

  float lastTime = glfwGetTime();
  // --- RENDER LOOP ---
  while (!glfwWindowShouldClose(window))
  {
    glfwGetFramebufferSize(window, &width, &height);
    float aspect = (float)width / (float)height;
    double currentTime = glfwGetTime();
    float dt = float(currentTime - lastTime);
    lastTime = currentTime;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();

    glm::mat4 projection =
        glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);

    float hemisphere =
        (glm::cos(cameraYAngle) >= 0.0f) ? 1.0f : -1.0f; // 1 = north, 1 = south
    if (glfwGetKey(window, GLFW_KEY_A))
      cameraAngle -= cameraSpeed * dt * hemisphere;
    else if (glfwGetKey(window, GLFW_KEY_D))
      cameraAngle += cameraSpeed * dt * hemisphere;

    if (glfwGetKey(window, GLFW_KEY_W))
      cameraYAngle -= cameraSpeed * dt;
    else if (glfwGetKey(window, GLFW_KEY_S))
      cameraYAngle += cameraSpeed * dt;

    if (glfwGetKey(window, GLFW_KEY_UP))
    {
      cameraDistance -= cameraSpeed * dt;
    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN))
    {
      cameraDistance += cameraSpeed * dt;
    }

    cameraAngle = glm::mod(cameraAngle, glm::two_pi<float>());
    cameraYAngle = glm::mod(cameraYAngle, glm::two_pi<float>());

    glm::mat4 rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, cameraAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation = glm::rotate(rotation, cameraYAngle, glm::vec3(1.0f, 0.0f, 0.0f));

    glm::vec3 camPos = glm::vec3(
        rotation *
        glm::vec4(
            0.0f, 0.0f, cameraDistance,
            1.0f)); // w means position/point , while w = 0.0f would be a vector

    glm::vec3 up = glm::vec3(rotation * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));

    glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f, 0.0f, 0.0f), up);
    glUniform4f(colorLoc, 0.0f, 0.6f, 0.9f, 1.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    glBindVertexArray(VAO);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe

    // CUBE
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, terrain.indices.size(), GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}
