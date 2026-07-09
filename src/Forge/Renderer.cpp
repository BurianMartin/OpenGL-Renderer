#include "Forge/Renderer.hpp"

namespace Forge
{
	Renderer::Renderer(GLfloat aspect_ratio)
	{
		rctx_ = std::make_shared<FrameContext>();
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

		rctx_->delta_time_ = delta_time;
		rctx_->time_ += delta_time; // accumulated elapsed time for animated shaders

		scene->DrawBackground();
		scene->Render();
	}

	std::shared_ptr<FrameContext> Renderer::GetFrameContext()
	{
		return rctx_;
	}
} // namespace Forge
