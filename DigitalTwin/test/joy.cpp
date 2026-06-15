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

int main()
{
    int fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);

    if (fd < 0)
    {
        std::cerr << "Failed to open /dev/input/js0\n";
        return 1;
    }

    Pose pose;

    float axis[8] = {0};

    constexpr double MAX_LINEAR_VEL  = 1.0; // m/s
    constexpr double MAX_ANGULAR_VEL = 2.0; // rad/s

    auto last_time = std::chrono::steady_clock::now();

    while (true)
    {
        js_event event;

        while (read(fd, &event, sizeof(event)) > 0)
        {
            event.type &= ~JS_EVENT_INIT;

            if (event.type == JS_EVENT_AXIS)
            {
                axis[event.number] =
                    static_cast<float>(event.value) / 32767.0f;
            }
        }

        auto now = std::chrono::steady_clock::now();
        double dt =
            std::chrono::duration<double>(now - last_time).count();
        last_time = now;

        // Common gamepad mapping:
        // axis[1] -> left stick up/down
        // axis[0] -> left stick left/right

        double linear_vel  = -axis[1] ;
        double angular_vel =  axis[2] ;

        pose.x += linear_vel * std::cos(pose.yaw) * dt;
        pose.y += linear_vel * std::sin(pose.yaw) * dt;
        pose.yaw += angular_vel * dt;

        std::cout
            << "\rX: " << pose.x
            << "  Y: " << pose.y
            << "  Yaw: " << pose.yaw
            << "  V: " << linear_vel
            << "  W: " << angular_vel
            << "      "
            << std::flush;

        usleep(20000); // 20 ms
    }

    close(fd);
    return 0;
}