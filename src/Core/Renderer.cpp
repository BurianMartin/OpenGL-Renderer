#include "Renderer.hpp"

void Core::Renderer::RenderScene(std::shared_ptr<Scene> scene)
{
	for (const auto &layer : scene->GetLayers())
	{
		layer.OnRender(); // Remember, layer ordering matters !!
	}
}
