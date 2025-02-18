#pragma once

static double mouseX, mouseY;
static double mouseDx, mouseDy;
static std::vector<Direction> buffered_directions{};
static bool keys[1024];
static bool first_mouse_input = true;
static bool pause = false;
static bool menu_on = true;
static bool mouse_updated = false;
static bool reset_scene = false;

inline void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            std::cout << "pressing esc" << std::endl;
            menu_on = !menu_on;
        }

        if (key == GLFW_KEY_R && action == GLFW_PRESS)
        {
            first_mouse_input = true;
            reset_scene = true;
            //camera.setTransform(inverse(lookAt(glm::vec3(0.0f, 0.0f, 30.0f), glm::vec3(10.0f, 0.0f, -7.0f),
            //                                  glm::vec3(0.0f, 1.0f, 0.0f))));
        }

        if (key == GLFW_KEY_P && action == GLFW_PRESS)
        {
            if (pause)
            {
                std::cout << "resuming" << std::endl;
            }
            if (!pause)
            {
                std::cout << "pausing" << std::endl;
            }
            pause = !pause;
        }
    };
}


inline void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (first_mouse_input)
    {
        mouseX = xpos;
        mouseY = ypos;
        first_mouse_input = false;
    }
    mouseDx = xpos - mouseX;
    mouseDy = ypos - mouseY;

    mouseX = xpos;
    mouseY = ypos;
    mouse_updated = true;
}

inline void keypresses_handling()
{
    if (keys[GLFW_KEY_W])
    {
        buffered_directions.emplace_back(Direction::FORWARD);
    }
    if (keys[GLFW_KEY_A])
    {
        buffered_directions.emplace_back(Direction::LEFT);
    }
    if (keys[GLFW_KEY_S])
    {
        buffered_directions.emplace_back(Direction::BACKWARDS);
    }
    if (keys[GLFW_KEY_D])
    {
        buffered_directions.emplace_back(Direction::RIGHT);
    }
    if (keys[GLFW_KEY_SPACE])
    {
        buffered_directions.emplace_back(Direction::UP);
    }
    if (keys[GLFW_KEY_LEFT_SHIFT])
    {
        buffered_directions.emplace_back(Direction::DOWN);
    }
}
