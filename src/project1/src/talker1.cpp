#include <string>
#include <algorithm>
#include "rclcpp/rclcpp.hpp"
#include "my_msgs/msg/msg1.hpp"
using namespace std;
class talker : public rclcpp::Node{
    public:
        talker(): Node("talker"), count_(0){
          this->declare_parameter<double>("publish_frequency", 10.0);
          publisher_ = this->create_publisher<my_msgs::msg::Msg1>("topic", 10);
          timer_ =this->create_wall_timer(chrono::milliseconds(100),bind(&talker::timer_callback,this));
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
              double new_frequency =param.as_double();
              update_timer_frequency();
              RCLCPP_INFO(this->get_logger(), "发布频率已更新为: %.2f Hz", new_frequency);
            }
        return result;
      }
      void update_timer_frequency() {
        double frequency = this->get_parameter("publish_frequency").as_double();
        timer_->cancel();// 取消现有定时器
        // auto period = chrono::duration_cast<chrono::milliseconds>(chrono::duration<double>(1.0 / frequency));
        auto period = chrono::duration_cast<chrono::milliseconds>(1.0s / frequency);
        timer_ = this->create_wall_timer(period, bind(&talker::timer_callback, this));// 创建新的定时器
      }
      void timer_callback(){
        auto message = my_msgs::msg::Msg1();
        message.num = count_;
        message.s = "Hello, ROS2: "+ to_string(count_++);
        RCLCPP_INFO(this->get_logger(),"发布: '%s'", message.s.c_str());
        publisher_->publish(message);
      }
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<my_msgs::msg::Msg1>::SharedPtr publisher_;
    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_callback_handle_;
    size_t count_;
};

int main(int argc, char * argv[]){
  rclcpp::init(argc, argv);
  rclcpp::spin(make_shared<talker>());
  rclcpp::shutdown();
  return 0;
}

//ros2 param set /talker publish_frequency 2.0