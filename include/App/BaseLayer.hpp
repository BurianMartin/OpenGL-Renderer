#include "Core/Layer.hpp"

#include <glad/gl.h>

namespace Solitare
{
    class BaseLayer : public Core::Layer
    {
    public:
        BaseLayer();
        ~BaseLayer();

        void OnEvent() override;
        void OnUpdate() override;
        void Transition() override;
        void Suspend() const override;
        void OnRender() const override;
    };

} // namespace Solitare
