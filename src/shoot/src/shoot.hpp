#ifndef SHOOT_HPP
#define SHOOT_HPP

// ==================== 配置宏 ====================
// 选择一种瞄准方式（只能启用一个）
 //#define USE_PNP_WORLD_COORDINATES    // 使用PNP解算的世界坐标进行弹道计算
#define USE_PIXEL_CENTER_AIM       // 使用像素坐标将准心移动到目标中心

#include <string>
#include <algorithm>
#include "rclcpp/rclcpp.hpp"
#include <cmath>
#include "std_msgs/msg/float32_multi_array.hpp"
#include <tdt_interface/msg/send_data.hpp>
#include <tdt_interface/msg/receive_data.hpp>
#include <std_msgs/msg/int32.hpp>

class Shoot : public rclcpp::Node {
public:
    Shoot(int id = 1);
    ~Shoot() = default;

private:
    // 参数回调
    rcl_interfaces::msg::SetParametersResult parameters_callback(const std::vector<rclcpp::Parameter> &parameters);
    void angles_callback(const tdt_interface::msg::ReceiveData::SharedPtr msg);
    
    void shoot_state_callback(const std_msgs::msg::Int32::SharedPtr msg);
    // 定时器回调
    void timer_callback();
    
    // 目标检测回调
    void detection_result_callback(const std_msgs::msg::Float32MultiArray::SharedPtr msg);
    
    // 工具函数
    double clamp(double value, double min_val, double max_val);
    bool is_world_coordinates_valid();
    
#ifdef USE_PNP_WORLD_COORDINATES
    // PNP世界坐标模式专用函数
    bool calculate_target_angles_from_world();
    double calculate_gravity_compensation(double horizontal_distance, double target_height);
#endif

#ifdef USE_PIXEL_CENTER_AIM
    // 像素中心瞄准模式专用函数
    bool calculate_target_angles_from_pixel();
    bool is_aimed_at_center();
#endif

    // 发布调试信息
    void publish_debug_info(double final_pitch, double final_yaw);
    
    // ROS2相关
    rclcpp::Publisher<tdt_interface::msg::SendData>::SharedPtr publisher_;
    rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr shoot_state_sub_; 
    rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr detection_result_sub_;
    rclcpp::TimerBase::SharedPtr timer_;
    OnSetParametersCallbackHandle::SharedPtr parameters_callback_handle_;
    rclcpp::Subscription<tdt_interface::msg::ReceiveData>::SharedPtr angles_sub_;
    int player_id_;
    
    // 通用参数
    double pitch_offset_;
    double yaw_offset_;
    bool auto_shoot_;
    double confidence_threshold_;
    bool is_aiming_;

    bool angles_initialized_;
    double current_yaw_;    // 当前实际yaw角度
    double current_pitch_;  // 当前实际pitch角度

    double base_yaw_;        // 底盘当前绝对 yaw（来自 angles_callback）
    double turret_yaw_offset_; // 云台相对于底盘的偏移（PID 控制对象）
    double target_turret_yaw_; // 云台期望的绝对角度
    
#ifdef USE_PNP_WORLD_COORDINATES
    // PNP模式专用参数
    double bullet_speed_;
    double gravity_;
    double camera_height_;
#endif

#ifdef USE_PIXEL_CENTER_AIM
    // 像素瞄准模式专用参数
    double pixel_kp_;
    double image_width_;
    double image_height_;
    double fov_x_;
    double fov_y_;
    double aim_tolerance_;
    double max_angle_step_;  // 新增：最大角度步长限制
    double pixel_kd_; 
    double pixel_ki_;
    double max_i_term_;
    double prev_error_x_;
    double prev_error_y_;
    double integral_x_;
    double integral_y_;
#endif

    // 云台角度
    double pitch_;
    double yaw_;
    
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
    
    // 控制模式信息
    std::string control_mode_;
};

#endif // SHOOT_HPP