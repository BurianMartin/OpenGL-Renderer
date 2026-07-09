#pragma once

#include "Utils.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Forge
{

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
        /// @param aspect_ratio Initial viewport aspect ratio (width/height), used to build the projection matrix.
        Camera(GLfloat aspect_ratio);

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

        /// @return The camera's current world-space position.
        glm::vec3 GetPosition() const;

        /// @return The camera's current vertical field of view, in degrees (clamped to [1°, 45°] by Zoom()).
        GLfloat GetFOV() const;
        /// Integrates every currently-held movement direction by `delta_time`; call once per frame.
        void Update(GLfloat delta_time);

        /// Sets or clears the held-movement flag for one direction; movement itself happens on the next Update().
        void CameraMove(CamMove direction, bool state);

        /// Recomputes the projection matrix for a new viewport aspect ratio (call on window resize).
        void UpdateAspectRatio(float aspect_ratio);

        /// Marks the next ProcessMousePosition call as the first one again, preventing a jump when re-capturing the cursor.
        void ResetMouseTracking();

        /// Sets a speed bonus added on top of the base movement speed for every direction (e.g. an OnEvent handler tying this to a "sprint" key); 0 to disable. Not clamped — a negative value would slow or reverse movement.
        void SetBoost(GLfloat boost);

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

        GLfloat aspect_ratio_;
        GLfloat near_plane_;
        GLfloat far_plane_;

        void UpdateFront(); // rebuilds front vector from yaw and pitch

        void MoveForward(GLfloat delta_time);
        void MoveBackward(GLfloat delta_time);
        void MoveLeft(GLfloat delta_time);
        void MoveRight(GLfloat delta_time);

        void MoveDown(GLfloat delta_time);
        void MoveUp(GLfloat delta_time);
    };
}