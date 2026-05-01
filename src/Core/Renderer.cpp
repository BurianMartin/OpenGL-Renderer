#include "Renderer.hpp"

namespace Core
{
	void Renderer::RenderScene(std::shared_ptr<Scene> scene)
	{
		if (!scene)
		{
			debug_error("Scene is null");
			return;
		}

		scene->DrawBackground();

		for (const auto &layer : scene->GetLayers())
		{
			layer->OnRender(); // Remember, layer ordering matters !!
		}
	}

} // namespace Core
