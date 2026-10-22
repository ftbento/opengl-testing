#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "shader.h"

// --- CALLBACKS ----------------------------------------------------------

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to init GLFW\n";
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(800, 600, "OpenGL Visualizer", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to init GLEW\n";
    return -1;
  }

  glEnable(GL_DEPTH_TEST);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  // --- GEOMETRY DATA (A single unit square blueprint) ---
  float verts[] = {
      // Front face (Z = 0.5)
      -0.5f, 0.5f, 0.5f,  // 0: Top-Left
      0.5f, 0.5f, 0.5f,   // 1: Top-Right
      0.5f, -0.5f, 0.5f,  // 2: Bottom-Right
      -0.5f, -0.5f, 0.5f, // 3: Bottom-Left
      // Back face (Z = -0.5)
      -0.5f, 0.5f, -0.5f, // 4: Top-Left-Back
      0.5f, 0.5f, -0.5f,  // 5: Top-Right-Back
      0.5f, -0.5f, -0.5f, // 6: Bottom-Right-Back
      -0.5f, -0.5f, -0.5f // 7: Bottom-Left-Back
  };

  unsigned int indices[] = {// Front face
                            0, 1, 2, 0, 2, 3,
                            // Back face
                            5, 4, 7, 5, 7, 6,
                            // Left face
                            4, 0, 3, 4, 3, 7,
                            // Right face
                            1, 5, 6, 1, 6, 2,
                            // Top face
                            4, 5, 1, 4, 1, 0,
                            // Bottom face
                            3, 2, 6, 3, 6, 7};

  unsigned int VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
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

  // --- RENDER LOOP ---
  while (!glfwWindowShouldClose(window)) {
    glfwGetFramebufferSize(window, &width, &height);
    float aspect = (float)width / (float)height;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();

    glm::mat4 projection =
        glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);
    glm::mat4 view =
        glm::lookAt(glm::vec3(0.0f, 1.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f));

    glUniform4f(colorLoc, 0.0f, 0.6f, 0.9f, 1.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    glBindVertexArray(VAO);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
    model =
        glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    float time = (float)glfwGetTime();

    glm::mat4 model2 = glm::mat4(1.0f);
    model2 = glm::translate(model2, glm::vec3(1.0f, 0.0f, 0.0f));
    model2 =
        glm::rotate(model2, time * 1.0f, glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch
    model2 =
        glm::rotate(model2, time * 1.3f, glm::vec3(0.0f, 1.0f, 0.0f)); // Yaw
    model2 =
        glm::rotate(model2, time * 0.7f, glm::vec3(0.0f, 0.0f, 1.0f)); // Roll

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}
