#pragma once
#include "Utils.hpp"
#include "Shader.hpp"
#include "Scene.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

class Renderer
{
private:
    ;

public:
    Renderer();
    ~Renderer();

    void Init();

    void Render_Frame(Scene &scene);
};