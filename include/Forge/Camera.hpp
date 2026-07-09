#pragma once

#include "Utils.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Forge
{

    /**
     * @brief A camera's on-screen region, expressed as a fraction of the window rather
     * than absolute pixels.
     *
     * `{0,0,1,1}` is the whole window; `{0.68f, 0.68f, 0.30f, 0.30f}` is a
     * picture-in-picture inset in the top-right 30%x30%. Because the rect is
     * normalized, it stays correct across a window resize on its own —
     * `SetWindowSize` only ever updates the window size it's measured
     * against, never the fractions themselves.
     */
    struct Viewport
    {
    private:
        GLfloat x_ = 0.0f;      // normalized [0,1] fraction of the window's width
        GLfloat y_ = 0.0f;      // normalized [0,1] fraction of the window's height
        GLfloat width_ = 1.0f;  // normalized [0,1] fraction of the window's width
        GLfloat height_ = 1.0f; // normalized [0,1] fraction of the window's height

        GLint window_width_ = 0;
        GLint window_height_ = 0;

        GLfloat aspect_ratio_ = 1.0f;

        void RecomputeAspectRatio()
        {
            aspect_ratio_ = (static_cast<GLfloat>(window_width_) * width_) / (static_cast<GLfloat>(window_height_) * height_);
        }

    public:
        /**
         * @param x,y,width,height Normalized [0,1] fractions of the window this viewport occupies.
         * @param windowWidth,windowHeight The window's current pixel size, used to derive this viewport's pixel rect/aspect ratio.
         */
        Viewport(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLint windowWidth, GLint windowHeight)
            : x_(x), y_(y), width_(width), height_(height), window_width_(windowWidth), window_height_(windowHeight)
        {
            RecomputeAspectRatio();
        }

        /// Call when the window itself resizes — this viewport's normalized fractions don't change, only the window size they're measured against.
        void SetWindowSize(GLint windowWidth, GLint windowHeight)
        {
            window_width_ = windowWidth;
            window_height_ = windowHeight;
            RecomputeAspectRatio();
        }

        /// Moves this viewport's normalized position without changing its size.
        void SetViewportPos(GLfloat x, GLfloat y)
        {
            x_ = x;
            y_ = y;
        }

        /// Resizes this viewport's normalized fraction without changing its position.
        void SetSize(GLfloat width, GLfloat height)
        {
            width_ = width;
            height_ = height;
            RecomputeAspectRatio();
        }

        /// @return This viewport's aspect ratio (pixel width / pixel height), recomputed whenever the fraction or window size changes.
        GLfloat GetAspectRatio() const
        {
            return aspect_ratio_;
        }

        /// Converts this viewport's normalized rect to pixels against the window's current size and applies it as both the GL viewport and scissor rect.
        void Apply() const
        {
            GLint x = static_cast<GLint>(x_ * window_width_);
            GLint y = static_cast<GLint>(y_ * window_height_);
            GLint width = static_cast<GLint>(width_ * window_width_);
            GLint height = static_cast<GLint>(height_ * window_height_);
            glViewport(x, y, width, height);
            glScissor(x, y, width, height);
        }
    };

    /// Movement directions understood by Camera::CameraMove; see `key_map` for the WASD/Space/Shift binding.
    enum class CamMove
    {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        FORWARD,
        BACKWARD
    };

    /**
     * @brief Perspective camera with WASD-style movement and mouse-look.
     *
     * Movement is edge-triggered: `CameraMove` just flips a boolean flag per
     * direction, and `Update(delta_time)` integrates every held direction
     * each frame — so multiple directions (e.g. forward + right) combine
     * naturally. Mouse look accumulates into yaw/pitch via
     * `ProcessMousePosition`/`Rotate`, and `Zoom` narrows/widens FOV rather
     * than moving the camera.
     */
    class Camera
    {
    public:
        /// @param viewport Initial `Viewport` (position/size fraction + window size) — its aspect ratio is used to build the projection matrix.
        Camera(Viewport viewport);

        /// @return The view matrix for the camera's current position/orientation.
        glm::mat4 GetViewMatrix() const;

        /// @return The perspective projection matrix for the camera's current FOV/aspect ratio.
        glm::mat4 GetProjectionMatrix() const;

        // Mouse and scroll

        /**
         * @brief Feeds a raw cursor position to the mouse-look system.
         * @param xpos Cursor X in screen space (pixels).
         * @param ypos Cursor Y in screen space (pixels).
         * @note The first call after construction or after ResetMouseTracking()
         * only primes `last_x_`/`last_y_` — it does not rotate the camera, to
         * avoid a jump from an unset previous position.
         */
        void ProcessMousePosition(float xpos, float ypos);

        /// Applies a mouse-look delta (already scaled by sensitivity upstream) to yaw/pitch and rebuilds the front vector.
        void Rotate(GLfloat x_offset, GLfloat y_offset);

        /// Adjusts FOV by a scroll delta (narrows on scroll up), clamped to [1°, 45°] — 45° is also the default, so this only ever zooms in, never wider than the initial view.
        void Zoom(GLfloat y_offset);

        /// Applies this camera's `Viewport` as both the GL viewport and scissor rect (forwards to `Viewport::Apply()`).
        void ApplyViewport();

        /// @return The camera's current world-space position.
        glm::vec3 GetPosition() const;

        /// @return The camera's current vertical field of view, in degrees (clamped to [1°, 45°] by Zoom()).
        GLfloat GetFOV() const;
        /// Integrates every currently-held movement direction by `delta_time`; call once per frame.
        void Update(GLfloat delta_time);

        /// Sets or clears the held-movement flag for one direction; movement itself happens on the next Update().
        void CameraMove(CamMove direction, bool state);

        /// Tells this camera's `Viewport` the window's new pixel size (see `Viewport::SetWindowSize`) — call on every window resize, for every camera, not just the active one.
        void UpdateViewportSize(GLint width, GLint height);

        /// Moves this camera's `Viewport` (normalized position) without changing its size.
        void UpdateViewportPosition(GLfloat x, GLfloat y);

        /// Replaces this camera's `Viewport` outright.
        void UpdateViewport(Viewport viewport);

        /// Repositions and resizes this camera's `Viewport` (normalized fractions) in one call, preserving whatever window size it already knew about.
        void UpdateViewport(GLfloat x, GLfloat y, GLfloat width, GLfloat height);

        /// Marks the next ProcessMousePosition call as the first one again, preventing a jump when re-capturing the cursor.
        void ResetMouseTracking();

        /// Sets a speed bonus added on top of the base movement speed for every direction (e.g. an OnEvent handler tying this to a "sprint" key); 0 to disable. Not clamped — a negative value would slow or reverse movement.
        void SetBoost(GLfloat boost);

        /// Directly sets world-space position, bypassing CameraMove/Update's edge-triggered movement — for a fixed/scripted camera (e.g. a picture-in-picture overview) that never receives WASD input.
        void SetPosition(const glm::vec3 &position);

        /// Directly sets yaw/pitch (degrees) and rebuilds the front vector, bypassing Rotate()'s mouse-look path — for a fixed/scripted camera. Pitch is not clamped here (unlike Rotate()), since a scripted camera may deliberately look straight down/up.
        void SetYawPitch(GLfloat yaw, GLfloat pitch);

    private:
        bool move_up_ = false;
        bool move_down_ = false;
        bool move_left_ = false;
        bool move_right_ = false;
        bool move_forward_ = false;
        bool move_backward_ = false;

        float last_x_ = 0.0f;
        float last_y_ = 0.0f;
        bool first_mouse_ = true;

        glm::vec3 position_;
        glm::vec3 front_;
        glm::vec3 up_;

        GLfloat yaw_;
        GLfloat pitch_;
        GLfloat fov_;

        GLfloat speed_;
        GLfloat boost_;
        GLfloat sensitivity_;

        GLfloat near_plane_;
        GLfloat far_plane_;

        Viewport viewport_;

        void UpdateFront(); // rebuilds front vector from yaw and pitch

        void MoveForward(GLfloat delta_time);
        void MoveBackward(GLfloat delta_time);
        void MoveLeft(GLfloat delta_time);
        void MoveRight(GLfloat delta_time);

        void MoveDown(GLfloat delta_time);
        void MoveUp(GLfloat delta_time);
    };
}