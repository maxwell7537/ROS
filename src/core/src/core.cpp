#include <string>
#include <algorithm>
#include "rclcpp/rclcpp.hpp"
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/image_encodings.hpp"

class core:public rclcpp::Node{
public:
    core():Node("core_node"){
        subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
            "unity_camera_image",10,
            std::bind(&core::imageCallback, this, std::placeholders::_1)  // 回调函数
        );
        RCLCPP_INFO(this->get_logger(), "core node started, subscribing to 'unity_camera_image'");
        cv::namedWindow("Received Image", cv::WINDOW_AUTOSIZE);
    }

private:
    void imageCallback(const sensor_msgs::msg::Image::SharedPtr msg)
    {
        try {
            // 将 ROS 2 的 sensor_msgs::msg::Image 转换为 OpenCV 的 cv::Mat
            cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
            // 显示图像
            cv::imshow("Received Image", cv_ptr->image);
            cv::waitKey(1);  // 必须调用 waitKey 来刷新显示

            RCLCPP_DEBUG(this->get_logger(), "Received and displayed image: %dx%d", 
                        msg->width, msg->height);
        } catch (cv_bridge::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
            return;
        } catch (cv::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "OpenCV exception: %s", e.what());
            return;
        }
    }
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<core>());
    cv::destroyAllWindows(); // 关闭 OpenCV 窗口
    rclcpp::shutdown();
    return 0;
}



