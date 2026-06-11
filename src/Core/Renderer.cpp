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

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Reset color and depth for the next frame draw

		scene->FillRenderContext(rctx_);
		auto current_time = glfwGetTime();
		rctx_.delta_time_ = current_time - rctx_.time_;
		rctx_.time_ = current_time;

		scene->DrawBackground();

		for (const auto &layer : scene->GetLayers())
		{
			layer->OnRender(rctx_); // Remember, layer ordering matters !!
		}
	}

} // namespace Core
