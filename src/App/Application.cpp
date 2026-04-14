#include "Application.hpp"

Application::Application() : current_scene(0)
{
    Init();

    scenes.emplace_back(glm::vec4(n_rgb(111.0f), n_rgb(0.0f), n_rgb(158.0f), n_rgb(0)));
    scenes.emplace_back(glm::vec4(n_rgb(48.0f), n_rgb(194.0f), n_rgb(165.0f), n_rgb(0)));

    glfwSetWindowUserPointer(window, this);

    float vertices[] = {
        0.0f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f};

    Model triangle1(vertices, sizeof(vertices), nullptr, 0, GL_TRIANGLES);
    Model triangle2(vertices, sizeof(vertices), nullptr, 0, GL_TRIANGLES);

    scenes[0].Add_Model(std::move(triangle1), std::make_unique<Shader>("/home/buri/OpenGL_Experiments/shaders/vertex.glsl", "/home/buri/OpenGL_Experiments/shaders/time_color.glsl"));
    scenes[1].Add_Model(std::move(triangle2), std::make_unique<Shader>("/home/buri/OpenGL_Experiments/shaders/vertex.glsl", "/home/buri/OpenGL_Experiments/shaders/solid_color.glsl"));

    glfwSetKeyCallback(window, Application::Handle_Key_Events);

    if (scenes.size() <= 0)
    {
        debug_error("No base scene created, nothing to display, aborting");
    }
}

Application::~Application() {}

void Application::Init()
{
    if (!glfwInit())
    {
        debug_error("GLFW init error");
        return;
    }

    window = glfwCreateWindow(1080, 1080, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();

        debug_error("GLFW window creation error");

        return;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        debug_error("Erorr while initializing GLAD");
    }

    renderer.Init();
}

void Application::Handle_Internal_Events()
{
    if (NEXT_SCENE & flags)
    {
        current_scene = (current_scene + 1) % scenes.size();
        debug_info("Setting scene to scene: " << current_scene);
    }
    if (PREV_SCENE & flags)
    {
        current_scene = (current_scene + scenes.size() - 1) % scenes.size();
        debug_info("Setting scene to scene: " << current_scene);
    }

    Clear_Flags();
}

void Application::Clear_Flags()
{
    flags = 0;
}

void Application::Run()
{

    while (!glfwWindowShouldClose(window))
    {
        renderer.Render_Frame(scenes[current_scene]);
        /* Render here */

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        Handle_Internal_Events();
    }
    for (Scene &scene : scenes)
    {
        scene.Clear();
    }
    glfwTerminate();
    return;
}

void Application::Next_Scene()
{
    flags |= NEXT_SCENE;
}

void Application::Prev_Scene()
{
    flags |= PREV_SCENE;
}

void Application::Move_Cam(Direction dir)
{
    switch (dir)
    {
    case Direction::Up:
        scenes[current_scene].Move_Camera_Up();
        break;

    case Direction::Down:
        scenes[current_scene].Move_Camera_Down();
        break;

    case Direction::Left:
        scenes[current_scene].Move_Camera_Left();
        break;

    case Direction::Right:
        scenes[current_scene].Move_Camera_Right();
        break;

    case Direction::Forward:
        scenes[current_scene].Move_Camera_Forward();
        break;

    case Direction::Backward:
        scenes[current_scene].Move_Camera_Backward();
        break;

    default:
        break;
    }
}

void Application::Handle_Key_Events(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Application *app = static_cast<Application *>(glfwGetWindowUserPointer(window));

    switch (action)
    {
    case GLFW_PRESS:
        switch (key)
        {
        case GLFW_KEY_W:
            debug_info("W Key Pressed");
            app->Move_Cam(Application::Direction::Forward);
            break;

        case GLFW_KEY_A:
            debug_info("A Key Pressed");
            app->Move_Cam(Application::Direction::Left);
            break;

        case GLFW_KEY_S:
            debug_info("S Key Pressed");
            app->Move_Cam(Application::Direction::Backward);
            break;

        case GLFW_KEY_D:
            debug_info("D Key Pressed");
            app->Move_Cam(Application::Direction::Right);
            break;

        case GLFW_KEY_SPACE:
            debug_info("SPACE Key Pressed");
            app->Move_Cam(Application::Direction::Up);
            break;

        case GLFW_KEY_LEFT_SHIFT:
            debug_info("SHIFT Key Pressed");
            app->Move_Cam(Application::Direction::Down);
            break;

        case GLFW_KEY_TAB:
            debug_info("Tab Key Pressed");
            app->Next_Scene();
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}
