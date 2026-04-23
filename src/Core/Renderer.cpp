#include "Renderer.hpp"

void Core::Renderer::RenderScene(std::shared_ptr<Scene> scene)
{
	if (!scene)
	{
		debug_error("Scene is null");
		return;
	}

	for (const auto &layer : scene->GetLayers())
	{
		layer->OnRender(); // Remember, layer ordering matters !!
	}
}
