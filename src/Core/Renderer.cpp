#include "Core/Renderer.hpp"

namespace Core
{
	Renderer::Renderer(GLfloat aspect_ratio)
	{
		rctx_ = std::make_shared<RenderContext>();
		rctx_->aspect_ratio_ = aspect_ratio;
	}

	void Renderer::RenderScene(std::shared_ptr<Scene> scene, GLfloat delta_time)
	{
		if (!scene)
		{
			debug_error("Scene is null");
			return;
		}

		glClear(GL_DEPTH_BUFFER_BIT);

		scene->UpdateRenderContext();
		rctx_->delta_time_ = delta_time;
		rctx_->time_ += delta_time; // accumulated elapsed time for animated shaders

		scene->DrawBackground();

		for (const auto &layer : scene->GetLayers())
		{
			layer->OnRender(rctx_); // Remember, layer ordering matters !!
		}
	}

	std::shared_ptr<RenderContext> Renderer::GetRenderContext()
	{
		return rctx_;
	}
} // namespace Core
