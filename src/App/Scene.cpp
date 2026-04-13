#include "Scene.hpp"

Scene::Scene()
{
    debug_info("Test Scene created");
}

Scene::~Scene()
{
}

void Scene::Add_Model(Model &&model, std::unique_ptr<Shader> shader)
{
    shaders.push_back(std::move(shader));
    Shader *key = shaders.back().get();

    model_map[key].push_back(std::move(model));
}

std::unordered_map<Shader *, std::vector<Model>> &Scene::Get_Model_Map()
{
    return model_map;
}

void Scene::Clear()
{
    model_map.clear();
    shaders.clear();
}