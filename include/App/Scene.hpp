#pragma once
#include "Utils.hpp"
#include "Shader.hpp"
#include "Model.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>
#include <unordered_map>

class Scene
{
private:
    std::vector<std::unique_ptr<Shader>> shaders;
    std::unordered_map<Shader *, std::vector<Model>> model_map;

public:
    Scene();
    ~Scene();

    void Add_Model(Model &&model, std::unique_ptr<Shader> shader);

    std::unordered_map<Shader *, std::vector<Model>> &Get_Model_Map();
    void Clear();
};