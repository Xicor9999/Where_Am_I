#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ROS_INFO_STREAM("Drive the robot to the new location");

    // Request the robot to run with the new linear and angular speed
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the command_robot service and pass the requested speeds
    if (!client.call(srv))
        ROS_ERROR("Failed to call service command_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    bool white_ball_found = false;
    int left_white_pixel_count = 0;
    int right_white_pixel_count = 0;

    // Loop through each pixel in the image
    for (int i = 0; i < img.height; i++)
    {
        for (int j = 0; j < img.width; j++)
        {
            // Calculate the pixel index
            int index = i * img.step + j*3;

            // Check if the pixel is white
            if (img.data[index] == white_pixel &&
                img.data[index + 1] == white_pixel &&
                img.data[index + 2] == white_pixel)
            {
                white_ball_found = true;

                // Count white pixels in the left and right thirds of the image
                if (j < img.width / 3)
                {
                    left_white_pixel_count++;
                }
                else if (j > 2 * img.width / 3)
                {
                    right_white_pixel_count++;
                }
            }
        }
    }

    // Decide on the robot's movement based on the white pixel count
    if (white_ball_found)
    {
        float lin_x = 0.2; // Move forward
        float ang_z = 0.0; // No rotation

        if (left_white_pixel_count > right_white_pixel_count)
        {
            ang_z = 0.2; // Turn left
        }
        else if (right_white_pixel_count > left_white_pixel_count)
        {
            ang_z = -0.2; // Turn right
        }

        drive_robot(lin_x, ang_z);
    }
    else
    {
        // Stop the robot if no white ball is detected
        drive_robot(0.0, 0.0);
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}