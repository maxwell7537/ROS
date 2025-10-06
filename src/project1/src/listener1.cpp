#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "my_msgs/msg/msg1.hpp"  
#include <opencv2/opencv.hpp>

using std::placeholders::_1;
class listener : public rclcpp::Node{
    public:
        listener()
        : Node("listener"){//回调函数
            subscription_ = this->create_subscription<my_msgs::msg::Msg1>(
            "topic", 10, std::bind(&listener::topic_callback, this, _1));
            RCLCPP_INFO(this->get_logger(), "订阅者节点已启动");
        }
    private:
        void topic_callback(const my_msgs::msg::Msg1 & msg) const{
            RCLCPP_INFO(this->get_logger(), "收到: '%s'", msg.s.c_str());
            cv::Mat img(400, 400, CV_8UC3, cv::Scalar(255, 255, 255));
            cv::putText(img, std::to_string(msg.num), cv::Point(50, 200), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 2);
            cv::imshow("Received Message", img);
            cv::waitKey(1);
        }
        rclcpp::Subscription<my_msgs::msg::Msg1>::SharedPtr subscription_;
};

int main(int argc,char*argv[]){
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<listener>());
    rclcpp::shutdown();
    return 0;
}