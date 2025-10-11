#include <string>
#include <algorithm>
#include "rclcpp/rclcpp.hpp"
#include "my_msgs/msg/msg1.hpp"
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include "sensor_msgs/image_encodings.hpp"

using namespace std;

class talker:public rclcpp::Node{
public:
    talker():Node("talker"),count_(0){
        this->declare_parameter<double>("publish_frequency", 2.0);
        publisher_ = this->create_publisher<my_msgs::msg::Msg1>("topic", 10);
        image_ = cv::Mat(1080, 1440, CV_8UC3, cv::Scalar(255, 255, 255));
        update_timer_frequency();
        param_callback_handle_ = this->add_on_set_parameters_callback(
            bind(&talker::parameters_callback, this, placeholders::_1));
        RCLCPP_INFO(this->get_logger(), "发布者节点已启动");
    }

private:
    rcl_interfaces::msg::SetParametersResult parameters_callback(
        const vector<rclcpp::Parameter> &parameters){
        rcl_interfaces::msg::SetParametersResult result;
        result.successful = true;
        for (const auto &param : parameters){
            if (param.get_name() == "publish_frequency") {
                double new_frequency = param.as_double();
                update_timer_frequency();
                RCLCPP_INFO(this->get_logger(), "发布频率已更新为: %.2f Hz", new_frequency);
            }
        }
        return result;
    }
    
    void update_timer_frequency() {
        double frequency = this->get_parameter("publish_frequency").as_double();
        if (timer_ && !timer_->is_canceled()) {
            timer_->cancel();
        }
        auto period = chrono::duration_cast<chrono::milliseconds>(chrono::duration<double>(1.0 / frequency));
        timer_ = this->create_wall_timer(period, bind(&talker::timer_callback, this));
    }
    
    void timer_callback(){
        auto message = my_msgs::msg::Msg1();
        message.num = count_;
        message.s = "Hello, ROS2: " + to_string(count_);
        image_.setTo(cv::Scalar(255, 255, 255));
        string text = "Count: " + to_string(count_);
        cv::putText(image_, text, cv::Point(500, 720),
                   cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        try {
            // 使用 cv_bridge::CvImage 对象进行转换
            auto cv_image = std::make_shared<cv_bridge::CvImage>();
            cv_image->header.stamp = this->now();
            cv_image->header.frame_id = "camera_frame";
            cv_image->encoding = "bgr8";
            cv_image->image = image_.clone();  // 使用clone确保数据独立
            message.image = *cv_image->toImageMsg();
            // RCLCPP_INFO(this->get_logger(), "发布图像尺寸: %dx%d", image_.cols, image_.rows);
        } catch (const exception& e) {
            RCLCPP_ERROR(this->get_logger(), "图像转换失败: %s", e.what());
            return;
        }
        
        RCLCPP_INFO(this->get_logger(), "发布: '%s'", message.s.c_str());
        publisher_->publish(message);
        count_++;
    }
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<my_msgs::msg::Msg1>::SharedPtr publisher_;
    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_callback_handle_;
    size_t count_;
    cv::Mat image_;
};

int main(int argc, char * argv[]){
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<talker>());
    rclcpp::shutdown();
    return 0;
}