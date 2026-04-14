#include "Scene.hpp"

Scene::Scene(glm::vec4 background) : background_color(background)
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

    model_map[key].push_back(std::make_unique<Model>(std::move(model)));
}

std::unordered_map<Shader *, std::vector<std::unique_ptr<Model>>> &Scene::Get_Model_Map()
{
    return model_map;
}

void Scene::Clear()
{
    model_map.clear();
    shaders.clear();
}

void Scene::ClearBackground()
{
    glClearColor(background_color.r, background_color.g, background_color.b, background_color.a);
}

void Scene::Move_Camera_Forward()
{
    glm::vec3 direction = glm::normalize(cam.front);
    cam.position += direction * cam.speed;
}

void Scene::Move_Camera_Backward()
{
    glm::vec3 direction = glm::normalize(cam.front);
    cam.position -= direction * cam.speed;
}

void Scene::Move_Camera_Up()
{
    cam.position += glm::vec3(0.0f, 1.0f, 0.0f) * cam.speed;
}

void Scene::Move_Camera_Down()
{
    cam.position -= glm::vec3(0.0f, 1.0f, 0.0f) * cam.speed;
}

void Scene::Move_Camera_Left()
{
    glm::vec3 right = glm::normalize(glm::cross(cam.front, cam.up));
    cam.position -= right * cam.speed;
}

void Scene::Move_Camera_Right()
{
    glm::vec3 right = glm::normalize(glm::cross(cam.front, cam.up));
    cam.position += right * cam.speed;
}

glm::mat4 Scene::GetVpMatrix()
{
    return cam.GetProjectionMatrix() * cam.GetViewMatrix();
}
