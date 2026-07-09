#pragma once
#include "Utils.hpp"
#include "Forge/Model.hpp"

#include <cmath>
#include <functional>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Forge
{
    inline GLfloat EaseOutQuad(GLfloat t) { return 1.0f - (1.0f - t) * (1.0f - t); }

    class Vec3Tween
    {
    public:
        /// Linear moves at constant speed for the whole duration; EaseOutQuad decelerates into the end pose.
        enum class Ease
        {
            Linear,
            EaseOutQuad
        };

        /// None plays once and stops (default). Loop restarts from the beginning every cycle
        /// (forth, reset, forth, reset...). PingPong reverses direction every cycle (forth, back,
        /// forth, back...). A repeating tween never reports IsDone() — it keeps running until
        /// whatever owns it (e.g. a Layer's tween list) is torn down.
        enum class Repeat
        {
            None,
            Loop,
            PingPong
        };

        Vec3Tween(std::shared_ptr<Model> model, glm::vec3 start_pos, glm::vec3 end_pos, GLfloat duration, glm::quat start_rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::quat end_rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), Ease ease = Ease::EaseOutQuad)
            : model_(model), start_position_(start_pos), end_position_(end_pos), start_rotation_(start_rot), end_rotation_(end_rot), duration_(duration), ease_(ease) {}

        // Ease has no default here (unlike the constructor above) — a bare 4-arg call
        // (model, start_pos, end_pos, duration) would otherwise match this constructor and
        // the one above equally well, since both default everything past position 4.
        Vec3Tween(std::shared_ptr<Model> model, glm::vec3 start_pos, glm::vec3 end_pos, GLfloat duration, Ease ease)
            : Vec3Tween(model, start_pos, end_pos, duration, glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), ease) {}

        void Update(GLfloat deltaTime)
        {
            if (repeat_ == Repeat::None)
            {
                bool wasDone = IsDone();
                elapsed_ = std::min(elapsed_ + deltaTime, duration_);
                model_->SetPosition(GetValuePosition());
                model_->SetRotation(GetValueRotation());
                if (finish_ && !wasDone && IsDone())
                {
                    finish_();
                }
                return;
            }

            GLfloat period = (repeat_ == Repeat::PingPong) ? duration_ * 2.0f : duration_;
            elapsed_ += deltaTime;
            bool completedCycle = elapsed_ >= period;
            if (completedCycle)
            {
                elapsed_ = std::fmod(elapsed_, period);
            }
            model_->SetPosition(GetValuePosition());
            model_->SetRotation(GetValueRotation());
            if (finish_ && completedCycle)
            {
                finish_(); // fires once per completed lap, not once ever, when repeating
            }
        }
        glm::vec3 GetValuePosition() const { return glm::mix(start_position_, end_position_, ApplyEase(NormalizedT())); }

        /// Under SetSpin, rotation is built directly from a continuously growing angle rather than
        /// slerped between two fixed quaternions — see SetSpin's comment for why that matters.
        glm::quat GetValueRotation() const
        {
            if (useSpin_)
            {
                return glm::angleAxis(spinTotalAngle_ * ApplyEase(NormalizedT()), spinAxis_) * start_rotation_;
            }
            return glm::slerp(start_rotation_, end_rotation_, ApplyEase(NormalizedT()));
        }
        bool IsDone() const { return repeat_ == Repeat::None && elapsed_ >= duration_; }
        void SetFinishFunction(std::function<void()> finish)
        {
            finish_ = finish;
        }
        void SetRepeat(Repeat repeat)
        {
            repeat_ = repeat;
        }

        /// Rotates continuously about `axis` by `total_angle_radians`, computed directly from
        /// elapsed time instead of slerped between two fixed endpoint quaternions. Use this for
        /// any spin of a full turn or more (e.g. glm::two_pi<float>() for one continuous 360°
        /// revolution) — start_rot/end_rot can't represent that: a full turn and "no rotation"
        /// are literally the same orientation, so there's no interpolation path between them for
        /// glm::slerp to take (it collapses to no visible rotation at all). Overrides end_rot;
        /// start_rot (defaulted to identity, or whatever was passed to the constructor) still
        /// applies as the base pose spinning happens on top of.
        void SetSpin(glm::vec3 axis, GLfloat total_angle_radians)
        {
            useSpin_ = true;
            spinAxis_ = axis;
            spinTotalAngle_ = total_angle_radians;
        }

    private:
        /// 0..1 for None/Loop; 0..1..0 (triangle wave) for PingPong.
        GLfloat NormalizedT() const
        {
            if (repeat_ == Repeat::PingPong)
            {
                GLfloat raw = elapsed_ <= duration_ ? elapsed_ : (2.0f * duration_ - elapsed_);
                return raw / duration_;
            }
            return elapsed_ / duration_;
        }

        GLfloat ApplyEase(GLfloat t) const
        {
            switch (ease_)
            {
            case Ease::Linear:
                return t;
            case Ease::EaseOutQuad:
            default:
                return EaseOutQuad(t);
            }
        }

        std::function<void()> finish_;
        glm::vec3 start_position_, end_position_;
        glm::quat start_rotation_, end_rotation_;
        GLfloat duration_, elapsed_ = 0.0f;
        std::shared_ptr<Model> model_;
        Ease ease_;
        Repeat repeat_ = Repeat::None;
        bool useSpin_ = false;
        glm::vec3 spinAxis_ = glm::vec3(0.0f, 1.0f, 0.0f);
        GLfloat spinTotalAngle_ = 0.0f;
    };
} // namespace Forge
