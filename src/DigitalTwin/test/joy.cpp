#include <iostream>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <chrono>

struct Pose
{
    double x = 0.0;
    double y = 0.0;
    double yaw = 0.0;
};

void printJoystickState(float axis[], bool button[])
{
    std::cout << "\n\n========== JOYSTICK STATE ==========\n";

    std::cout << "Axes:\n";
    for (int i = 0; i < 8; i++)
    {
        std::cout
            << "  Axis[" << i << "] = "
            << axis[i] << '\n';
    }

    std::cout << "\nButtons:\n";
    for (int i = 0; i < 16; i++)
    {
        std::cout
            << "  Button[" << i << "] = "
            << (button[i] ? "Pressed" : "Released")
            << '\n';
    }

    std::cout << "====================================\n";
}

int main()
{
    const char *device = "/dev/input/js0";

    int fd = open(device, O_RDONLY | O_NONBLOCK);

    if (fd < 0)
    {
        std::cerr << "Failed to open " << device << std::endl;
        return 1;
    }

    std::cout << "Opened joystick: " << device << std::endl;

    Pose pose;

    float axis[8] = {0.0f};
    bool button[16] = {false};

    auto last_time = std::chrono::steady_clock::now();
    auto print_time = std::chrono::steady_clock::now();

    while (true)
    {
        js_event event;

        while (read(fd, &event, sizeof(event)) > 0)
        {
            event.type &= ~JS_EVENT_INIT;

            if (event.type == JS_EVENT_AXIS)
            {
                if (event.number < 8)
                {
                    axis[event.number] =
                        static_cast<float>(event.value) / 32767.0f;

                    // std::cout
                    //     << "\nAXIS["
                    //     << (int)event.number
                    //     << "] = "
                    //     << axis[event.number];
                }
            }
            else if (event.type == JS_EVENT_BUTTON)
            {
                // if (event.number < 16)
                // {
                //     button[event.number] = event.value;

                //     std::cout
                //         << "\nBUTTON["
                //         << (int)event.number
                //         << "] = "
                //         << (button[event.number]
                //                 ? "PRESSED"
                //                 : "RELEASED");
                // }
            }
        }

        auto now = std::chrono::steady_clock::now();

        double dt =
            std::chrono::duration<double>(
                now - last_time)
                .count();

        last_time = now;

        //------------------------------------------------------------------
        // Example mapping:
        // Axis 1 = Forward / Backward
        // Axis 2 = Turn Left / Right
        //------------------------------------------------------------------

        // double linear_vel  = -axis[1];
        // double angular_vel =  axis[2];

        // pose.x += linear_vel * std::cos(pose.yaw) * dt;
        // pose.y += linear_vel * std::sin(pose.yaw) * dt;
        // pose.yaw += angular_vel * dt;

        // std::cout
        //     << "\r"
        //     << "X: " << pose.x
        //     << "  Y: " << pose.y
        //     << "  Yaw: " << pose.yaw
        //     << "  V: " << linear_vel
        //     << "  W: " << angular_vel
        //     << "        "
        //     << std::flush;

        //------------------------------------------------------------------
        // Print full joystick state every second
        //------------------------------------------------------------------

                    printJoystickState(axis, button);


        // if (std::chrono::duration_cast<std::chrono::seconds>(
        //         now - print_time)
        //         .count() >= 1)
        // {
        //     printJoystickState(axis, button);
        //     print_time = now;
        // }

        usleep(20000); // 20 ms
    }

    close(fd);
    return 0;
}