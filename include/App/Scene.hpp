#pragma once
#include "Utils.hpp"
#include "Shader.hpp"
#include "Model.hpp"
#include "Camera.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <unordered_map>

class Scene
{
private:
    std::vector<std::unique_ptr<Shader>> shaders;
    std::unordered_map<Shader *, std::vector<std::unique_ptr<Model>>> model_map;
    glm::vec4 background_color;

    Camera cam;

public:
    Scene(glm::vec4 background = glm::vec4(n_rgb(111.0f), n_rgb(0), n_rgb(158.0f), n_rgb(0)));
    ~Scene();

    Scene(const Scene &) = delete;
    Scene &operator=(const Scene &) = delete;

    Scene(Scene &&) noexcept = default;
    Scene &operator=(Scene &&) noexcept = default;

    void Add_Model(Model &&model, std::unique_ptr<Shader> shader);

    std::unordered_map<Shader *, std::vector<std::unique_ptr<Model>>> &Get_Model_Map();
    void Clear();

    void ClearBackground();

    void Move_Camera_Forward();
    void Move_Camera_Backward();

    void Move_Camera_Down();
    void Move_Camera_Up();

    void Move_Camera_Left();
    void Move_Camera_Right();

    glm::mat4 GetVpMatrix();
};