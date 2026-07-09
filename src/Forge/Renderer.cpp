#include "Forge/Renderer.hpp"

namespace Forge
{
	Renderer::Renderer(GLint windowWidth, GLint windowHeight)
	{
		fctx_ = std::make_shared<FrameContext>();
		fctx_->window_height_ = windowHeight;
		fctx_->window_width_ = windowWidth;
	}

	void Renderer::RenderScene(std::shared_ptr<Scene> scene, GLfloat delta_time)
	{
		if (!scene)
		{
			debug_error("Scene is null");
			return;
		}

		fctx_->delta_time_ = delta_time;
		fctx_->time_ += delta_time; // accumulated elapsed time for animated shaders

		scene->Render(); // clears depth and draws the background once per camera, inside its own scissored viewport
	}

	std::shared_ptr<FrameContext> Renderer::GetFrameContext()
	{
		return fctx_;
	}
} // namespace Forge
