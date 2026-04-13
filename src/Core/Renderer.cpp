#include "Renderer.hpp"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::Init()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::Render_Frame(Scene &scene)
{

	auto &model_map = scene.Get_Model_Map();

	glClearColor(n_rgb(111.0f), n_rgb(0), n_rgb(158.0f), n_rgb(0));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	for (auto &[shader, model_vector] : model_map)
	{
		shader->Use();
		float time = (float)glfwGetTime();
		shader->SetFloat("uTime", time);
		for (Model &model : model_vector)
		{
			model.Draw();
		}
	}
}
