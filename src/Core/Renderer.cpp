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

	scene.ClearBackground();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	auto VM_Matrix = scene.GetVpMatrix();

	for (auto &[shader, model_vector] : model_map)
	{
		shader->Use();
		float time = (float)glfwGetTime();
		shader->SetFloat("uTime", time);
		for (auto &model : model_vector)
		{
			glm::mat4 M = model->GetModelMatrix();
			glm::mat4 MVP = VM_Matrix * M;

			shader->SetMat4("uMVP", MVP);
			model->Draw();
		}
	}
}
