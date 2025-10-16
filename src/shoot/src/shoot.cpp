#include <string>
#include <algorithm>
#include "rclcpp/rclcpp.hpp"
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/image_encodings.hpp"
#include "geometry_msgs/msg/point.hpp"
#include "std_msgs/msg/float32_multi_array.hpp"
#include "my_msgs/msg/send_data.hpp"
#include <tdt_interface/msg/send_data.hpp>
#include <cmath>

using namespace std;

class Shoot : public rclcpp::Node {
public:
    Shoot(int id = 1) : Node("shoot_node") {
        this->player_id = id;
        
        // 初始化云台角度
        pitch_ = 0.0;
        yaw_ = 90.0;
        
        // 初始化目标位置
        target_pixel_x_ = 0.0f;
        target_pixel_y_ = 0.0f;
        target_world_x_ = 0.0f;
        target_world_y_ = 0.0f;
        target_world_z_ = 0.0f;
        target_distance_ = 0.0f;
        target_confidence_ = 0.0f;
        has_target_ = false;
        
        // 创建发布者
        string topic_name = "target_angles_player_" + to_string(player_id);
        publisher_ = this->create_publisher<tdt_interface::msg::SendData>(topic_name, 10);

        // 创建目标位置订阅者
        detection_result_sub_ = this->create_subscription<std_msgs::msg::Float32MultiArray>(
            "detection_result", 10,
            std::bind(&Shoot::detection_result_callback, this, std::placeholders::_1));

        // 创建定时器
        timer_ = this->create_wall_timer(
            chrono::milliseconds(10),  // 100Hz
            bind(&Shoot::timer_callback, this));
            
        RCLCPP_INFO(this->get_logger(), "自动瞄准云台控制节点已启动，发布到话题: %s", topic_name.c_str());
        RCLCPP_INFO(this->get_logger(), "等待目标检测数据...");
    }
    
    ~Shoot() {
    }

private:
    void timer_callback() {
        auto msg = tdt_interface::msg::SendData();
        
        if (has_target_ && target_confidence_ > 0.3) {
            // 自动瞄准模式：计算目标角度
            calculate_target_angles();
            msg.pitch = pitch_;
            msg.yaw = yaw_;
            msg.if_shoot = true; // 检测到目标就射击
            
            // 减少日志输出频率
            static int log_counter = 0;
            if (log_counter++ % 100 == 0) { // 每1秒输出一次
                RCLCPP_INFO(this->get_logger(), "自动瞄准 - 角度: pitch=%.2f, yaw=%.2f, 距离=%.2f, 置信度=%.2f", 
                            pitch_, yaw_, target_distance_, target_confidence_);
            }
        } else {
            // 没有目标时停止射击
            msg.pitch = pitch_;
            msg.yaw = yaw_;
            msg.if_shoot = false;
            
            static int no_target_counter = 0;
            if (no_target_counter++ % 200 == 0) { // 每2秒输出一次
                RCLCPP_INFO(this->get_logger(), "等待目标...");
            }
        }
        
        // 发布消息
        publisher_->publish(msg);
    }
    
    void detection_result_callback(const std_msgs::msg::Float32MultiArray::SharedPtr msg) {
        if (msg->data.size() >= 11) {
            // 解析完整检测结果
            target_pixel_x_ = msg->data[0];
            target_pixel_y_ = msg->data[1];
            target_world_x_ = msg->data[2];
            target_world_y_ = msg->data[3];
            target_world_z_ = msg->data[4];
            target_distance_ = msg->data[5];
            target_confidence_ = msg->data[6];
            
            has_target_ = true;
            
            // 更新最后一次收到目标的时间
            last_target_time_ = this->now();
            
            // 减少日志输出频率
            static int target_log_counter = 0;
            if (target_log_counter++ % 50 == 0) {
                RCLCPP_DEBUG(this->get_logger(), "收到目标 - 世界坐标: (%.2f, %.2f, %.2f), 距离: %.2f, 置信度: %.2f",
                            target_world_x_, target_world_y_, target_world_z_,
                            target_distance_, target_confidence_);
            }
        } else {
            has_target_ = false;
        }
    }
    
    void calculate_target_angles() {
        // 根据目标在世界坐标系中的位置计算云台角度
        
        // 计算偏航角 (yaw) - 水平方向
        // 使用atan2计算角度，考虑x和z坐标
        yaw_ = atan2(target_world_x_, target_world_z_) * (180.0 / M_PI)+90;
        
        // 计算俯仰角 (pitch) - 垂直方向  
        // 先计算水平距离，然后计算俯仰角
        double horizontal_distance = sqrt(target_world_x_ * target_world_x_ + target_world_z_ * target_world_z_);
        pitch_ = atan2(target_world_y_, horizontal_distance) * (180.0 / M_PI);
        
        // 限制俯仰角范围（根据实际云台机械限制调整）
        if (pitch_ > 60.0) pitch_ = 60.0;
        if (pitch_ < -17.0) pitch_ = -17.0;
        
        // 可以在这里添加更复杂的控制算法，比如：
        // 1. PID控制实现平滑移动
        // 2. 预测目标运动轨迹
        // 3. 根据距离调整射击策略
    }
    
    rclcpp::Publisher<tdt_interface::msg::SendData>::SharedPtr publisher_;
    rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr detection_result_sub_;
    rclcpp::TimerBase::SharedPtr timer_;
    int player_id = 0;
    
    // 云台角度
    double pitch_;  // 俯仰角
    double yaw_;    // 偏航角
    
    // 目标信息
    float target_pixel_x_;
    float target_pixel_y_;
    float target_world_x_;
    float target_world_y_;
    float target_world_z_;
    float target_distance_;
    float target_confidence_;
    bool has_target_;
    rclcpp::Time last_target_time_;
};

int main(int argc, char * argv[]) {
    setenv("ROS_DOMAIN_ID", "0", 1);
    rclcpp::init(argc, argv);
    int id = 1;
    if(argc > 1) {
        try {
            id = stoi(argv[1]);
            if(id != 1 && id != 2) {
                cerr << "玩家ID必须是1或2" << endl;
                return 1;
            }
        } catch(const exception& e) {
            cerr << "玩家ID必须是数字" << endl;
            return 1;
        }
    }
    auto node = make_shared<Shoot>(id);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}