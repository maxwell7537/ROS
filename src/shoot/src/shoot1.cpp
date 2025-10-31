#include <chrono>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <memory>
#include <string>
#include <vector>
#include <random>

#include "rclcpp/rclcpp.hpp"
#include "rcl_interfaces/msg/set_parameters_result.hpp"
#include "std_msgs/msg/float32_multi_array.hpp"
#include "std_msgs/msg/int32.hpp"
#include <tdt_interface/msg/send_data.hpp>
#include <tdt_interface/msg/receive_data.hpp>

using namespace std::chrono_literals;

class Shoot : public rclcpp::Node {
public:
    Shoot(int id = 1);
    ~Shoot() = default;

private:
    // 回调
    void angles_callback(const tdt_interface::msg::ReceiveData::SharedPtr msg);
    void shoot_state_callback(const std_msgs::msg::Int32::SharedPtr msg);
    void detection_result_callback(const std_msgs::msg::Float32MultiArray::SharedPtr msg);
    void timer_callback();

    // 工具
    double clamp(double value, double min_val, double max_val);
    bool is_world_coordinates_valid();
    bool calculate_target_angles_from_world();
    double calculate_gravity_compensation(double horizontal_distance, double target_height);
    bool calculate_target_angles_from_pixel();
    bool is_aimed_at_center();
    
    double apply_spread(double angle);
    double calculate_iterative_ballistic(double horizontal_distance, double target_height);
    bool is_angle_safe(double pitch, double yaw);
    bool is_distance_safe(double distance);

    void executeEnemy1Search();

    // ROS2接口
    rclcpp::Publisher<tdt_interface::msg::SendData>::SharedPtr publisher_;
    rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr shoot_state_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr detection_result_sub_;
    rclcpp::Subscription<tdt_interface::msg::ReceiveData>::SharedPtr angles_sub_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr parameters_callback_handle_;

    rcl_interfaces::msg::SetParametersResult parameters_callback(
        const std::vector<rclcpp::Parameter> &parameters);
    void update_parameters_from_server();

    int player_id_;

    // 通用参数
    double pitch_offset_;
    double yaw_offset_;
    int auto_shoot_;               // 0/1
    double confidence_threshold_;
    bool is_aiming_;
    bool angles_initialized_;
    double current_yaw_;
    double current_pitch_;
    int class_id;

    // 射击控制参数
    double fire_interval_;
    bool enable_fire_rate_control_;
    rclcpp::Time last_fire_time_;
    
    // 弹道散布
    double bullet_spread_angle_;
    bool enable_spread_;
    std::random_device rd_;
    std::mt19937 gen_;
    std::normal_distribution<> spread_dist_;
    
    // 发射点偏移
    double fire_point_offset_x_;
    double fire_point_offset_y_;
    double fire_point_offset_z_;

    // PNP/弹道参数
    double bullet_speed_;
    double gravity_;
    double camera_height_;
    
    // 弹道优化参数
    int ballistic_iterations_;
    double ballistic_tolerance_;
    double air_resistance_factor_;
    double bullet_mass_;

    // 像素瞄准参数（用于开火决策和在无世界坐标时回退）
    double pixel_kp_;
    double pixel_kd_;
    double pixel_ki_;
    double image_width_;
    double image_height_;
    double fov_x_;
    double fov_y_;
    double aim_tolerance_;
    double max_angle_step_;
    double max_i_term_;
    double prev_error_x_;
    double prev_error_y_;
    double integral_x_;
    double integral_y_;
    double center_x[10];
    double center_y[10];
    double kp;
    
    // 安全限制参数
    double max_pitch_angle_;
    double min_pitch_angle_;
    double max_yaw_angle_;
    double min_yaw_angle_;
    double max_effective_range_;
    double min_safe_distance_;

    // 云台角度（控制目标）
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
  

    
};

// --------------------------------- 实现 ---------------------------------

Shoot::Shoot(int id)
: Node("shoot_node"),
  player_id_(id),
  pitch_offset_(0.0),
  yaw_offset_(0.0),
  auto_shoot_(0),
  confidence_threshold_(0.3),
  is_aiming_(false),
  angles_initialized_(false),
  current_yaw_(0.0),
  current_pitch_(0.0),
  class_id(0),
  // 射击控制参数
  fire_interval_(0.2),
  enable_fire_rate_control_(true),
  last_fire_time_(this->now()),
  // 弹道散布
  bullet_spread_angle_(0.3),
  enable_spread_(true),
  gen_(rd_()),
  spread_dist_(0.0, bullet_spread_angle_),
  // 发射点偏移
  fire_point_offset_x_(0.0),
  fire_point_offset_y_(0.0),
  fire_point_offset_z_(0.25),
  // PNP/弹道参数
  bullet_speed_(23.0),
  gravity_(9.8),
  camera_height_(0.5),
  // 弹道优化参数
  ballistic_iterations_(3),
  ballistic_tolerance_(0.01),
  air_resistance_factor_(0.0),
  bullet_mass_(0.003),
  // 像素瞄准参数
  pixel_kp_(0.020),
  pixel_kd_(0.02),
  pixel_ki_(0.0005),
  image_width_(640.0),
  image_height_(480.0),
  fov_x_(60.0),
  fov_y_(60.0),
  aim_tolerance_(90.0),// 容忍度
  max_angle_step_(3.0),
  max_i_term_(5.0),
  prev_error_x_(0.0),
  prev_error_y_(0.0),
  integral_x_(0.0),
  integral_y_(0.0),
//   center_x ({0,image_width_/2.0,image_width_/2.0,image_width_/2.0,image_width_/2.0,image_width_ / 2.0,image_width_/2.0+15}),
//   center_y({0,image_height_/2.0,image_height_/2.0+20,image_height_/2.0+35,image_height_/2.0,image_height_/2.0+30,image_height_/2.0+33}),
    center_x{0, image_width_/2.0, image_width_/2.0, image_width_/2.0, image_width_/2.0, image_width_/2.0, image_width_/2.0+15},
    center_y{0, image_height_/2.0+20, image_height_/2.0+20, image_height_/2.0+35, image_height_/2.0, image_height_/2.0+30, image_height_/2.0+33},
  kp(15.0),
  // 安全限制参数
  max_pitch_angle_(60.0),
  min_pitch_angle_(-17.0),
  max_yaw_angle_(90.0),
  min_yaw_angle_(-90.0),
  max_effective_range_(15.0),
  min_safe_distance_(0.5),
  // 云台角度
  pitch_(0.0),
  yaw_(0.0),
  // 目标信息
  target_pixel_x_(0.0f),
  target_pixel_y_(0.0f),
  target_world_x_(0.0f),
  target_world_y_(0.0f),
  target_world_z_(0.0f),
  target_distance_(0.0f),
  target_confidence_(0.0f),
  has_target_(false)

{// 发布者/订阅者
    this->declare_parameter("kp", 5.0);
    std::string topic_name = "target_angles_player_" + std::to_string(player_id_);
    publisher_ = this->create_publisher<tdt_interface::msg::SendData>(topic_name, 10);

    detection_result_sub_ = this->create_subscription<std_msgs::msg::Float32MultiArray>(
        "detection_result", 10,
        std::bind(&Shoot::detection_result_callback, this, std::placeholders::_1));

    std::string angles_topic = "real_angles_player_" + std::to_string(player_id_);
    angles_sub_ = this->create_subscription<tdt_interface::msg::ReceiveData>(
        angles_topic, 10,
        std::bind(&Shoot::angles_callback, this, std::placeholders::_1));

    shoot_state_sub_ = this->create_subscription<std_msgs::msg::Int32>(
        "shoot_state", 10,
        std::bind(&Shoot::shoot_state_callback, this, std::placeholders::_1));

    timer_ = this->create_wall_timer(10ms, std::bind(&Shoot::timer_callback, this));

    parameters_callback_handle_ = this->add_on_set_parameters_callback(
            std::bind(&Shoot::parameters_callback, this, std::placeholders::_1));
    update_parameters_from_server();
    RCLCPP_INFO(this->get_logger(), "shoot node started (player_id=%d)", player_id_);
    RCLCPP_INFO(this->get_logger(), "射击间隔: %.1f秒, 子弹速度: %.1f米/秒", fire_interval_, bullet_speed_);
    RCLCPP_INFO(this->get_logger(), "弹道散布: ±%.1f度, 发射点偏移: (%.2f, %.2f, %.2f)米", 
                bullet_spread_angle_, fire_point_offset_x_, fire_point_offset_y_, fire_point_offset_z_);
}

rcl_interfaces::msg::SetParametersResult Shoot::parameters_callback(
    const std::vector<rclcpp::Parameter> &parameters) {
    
    rcl_interfaces::msg::SetParametersResult result;
    result.successful = true;
    
    for (const auto &parameter : parameters) {
        const std::string &name = parameter.get_name();
        if (name == "kp") {
            kp = parameter.as_double();
            RCLCPP_INFO(this->get_logger(), "✅ 更新参数 kp = %.3f", kp);
        }
    }
    
    return result;
}
void Shoot::update_parameters_from_server() {
    kp = this->get_parameter("kp").as_double();
    RCLCPP_INFO(this->get_logger(), "从参数服务器加载kp: %.3f", kp);
}

void Shoot::angles_callback(const tdt_interface::msg::ReceiveData::SharedPtr msg){
    current_yaw_ = msg->yaw;
    current_pitch_ = msg->pitch;

    if (!angles_initialized_) {
        yaw_ = current_yaw_;
        pitch_ = 0.0;
        angles_initialized_ = true;
        RCLCPP_INFO(this->get_logger(), "Angles initialized: yaw=%.2f", current_yaw_);
    }
}

void Shoot::shoot_state_callback(const std_msgs::msg::Int32::SharedPtr msg)
{
    if (msg->data == 1||msg->data == 2) {
        is_aiming_ = true;
        if (angles_initialized_) {
            // 同步控制角度到当前实际角度，避免瞬移
            yaw_ = current_yaw_;
            pitch_ = current_pitch_;
            prev_error_x_ = 0.0;
            prev_error_y_ = 0.0;
            integral_x_ = 0.0;
            integral_y_ = 0.0;
            RCLCPP_INFO(this->get_logger(), "Shooting enabled, angles synced (yaw=%.2f)", yaw_);
        }
        RCLCPP_INFO(this->get_logger(), "Received shoot enable");
    }else {
        is_aiming_ = false;
        RCLCPP_INFO(this->get_logger(), "Received shoot disable");
    }
}

void Shoot::detection_result_callback(const std_msgs::msg::Float32MultiArray::SharedPtr msg){
    if (msg->data.size() >= 11) {
        // 解析检测结果（与原数据格式保持一致）
        target_pixel_x_ = msg->data[0];
        target_pixel_y_ = msg->data[1];
        target_world_x_ = msg->data[2];
        target_world_y_ = msg->data[3];
        target_world_z_ = msg->data[4];
        target_distance_ = msg->data[5];
        target_confidence_ = msg->data[6];
        class_id = (int)msg->data[7];

        bool data_valid = (target_pixel_x_ > 0 && target_pixel_y_ > 0 &&
                           target_pixel_x_ < 1000 && target_pixel_y_ < 1000 &&
                           target_confidence_ > 0.1);

        // 在1号敌人专用模式下，只处理1号敌人
        
            // 正常模式下的目标处理
        has_target_ = data_valid;
        if (data_valid) {
            last_target_time_ = this->now();
        }
        }
    else {
        has_target_ = false;
    }
    
    // 原有的class_id特殊处理逻辑保持不变
    if(class_id==4){
        center_y[4] = 240 + 2 * target_distance_ - 4.9 * pow(target_distance_ / 23, 2);
    }
    if(class_id==2){
        auto_shoot_=1;
    }
}



bool Shoot::is_world_coordinates_valid()
{
    // world coordinates valid check
    if (target_distance_ <= 0) return false;
    if (target_world_x_ == 0.0f && target_world_y_ == 0.0f && target_world_z_ == 0.0f) return false;
    return true;
}
bool Shoot::calculate_target_angles_from_world(){
    double target_x = static_cast<double>(target_world_x_) - fire_point_offset_x_;//向右
    double target_z = static_cast<double>(target_world_z_) - fire_point_offset_z_;//向下
    double target_y = static_cast<double>(target_world_y_) - fire_point_offset_y_;//向前
    
    double target_yaw = std::atan2(-target_x, target_z) * (180.0 / M_PI);
    
    double horizontal_distance = std::sqrt(target_x * target_x + target_z * target_z);
    double target_height = target_y - camera_height_;  // 修正高度计算
    
    double target_pitch = calculate_iterative_ballistic(horizontal_distance, target_height);
    
    // 计算角度差，使用增量控制
    double yaw_diff = target_yaw - current_yaw_;
    double pitch_diff = target_pitch - current_pitch_;
    
    // yaw_diff = clamp(yaw_diff, -max_angle_step_, max_angle_step_);
    // pitch_diff = clamp(pitch_diff, -max_angle_step_, max_angle_step_);
    
    yaw_ += yaw_diff;
    pitch_ += pitch_diff;
    
    if (enable_spread_) {
        pitch_ = apply_spread(pitch_);
        yaw_ = apply_spread(yaw_);
    }
    return true;
}

double Shoot::calculate_iterative_ballistic(double horizontal_distance, double target_height)
{
    // 迭代求解弹道，考虑重力
    double pitch_rad = std::atan2(target_height, horizontal_distance);
    
    for (int i = 0; i < ballistic_iterations_; ++i) {
        double vx = bullet_speed_ * std::cos(pitch_rad);
        double vy = bullet_speed_ * std::sin(pitch_rad);
        
        double time_to_target = horizontal_distance / vx;
        double drop = 0.5 * gravity_ * time_to_target * time_to_target;
        
        double new_pitch_rad = std::atan2(target_height + drop, horizontal_distance);
        
        // 检查收敛
        if (std::abs(new_pitch_rad - pitch_rad) < ballistic_tolerance_) {
            break;
        }
        pitch_rad = new_pitch_rad;
    }
    
    return pitch_rad * (180.0 / M_PI);
}

double Shoot::calculate_gravity_compensation(double horizontal_distance, double target_height)
{
    if (bullet_speed_ <= 0.0 || horizontal_distance <= 0.0) {
        return 0.0;
    }

    // 飞行时间=水平距离 / 子弹速度
    double time_to_target = horizontal_distance / bullet_speed_;
    double drop_distance = 0.5 * gravity_ * time_to_target * time_to_target;

    // 需要提升的角度
    double compensation_rad = std::atan2(drop_distance, horizontal_distance);
    return compensation_rad * (180.0 / M_PI);
}

bool Shoot::calculate_target_angles_from_pixel()
{
    // 在无法获得世界坐标时，使用像素 PID 更新角度以跟随目标
    double error_x = static_cast<double>(target_pixel_x_) - center_x[class_id];
    double error_y = static_cast<double>(target_pixel_y_) - center_y[class_id];

    double derivative_x = error_x - prev_error_x_;
    double derivative_y = error_y - prev_error_y_;

    integral_x_ += error_x;
    integral_y_ += error_y;
    integral_x_ = clamp(integral_x_, -max_i_term_, max_i_term_);
    integral_y_ = clamp(integral_y_, -max_i_term_, max_i_term_);

    double angle_adjust_x = pixel_kp_ * error_x + pixel_kd_ * derivative_x + pixel_ki_ * integral_x_;
    double angle_adjust_y = pixel_kp_ * error_y + pixel_kd_ * derivative_y + pixel_ki_ * integral_y_;

    // 归一化为角度
    angle_adjust_x = (angle_adjust_x / image_width_) * fov_x_;
    angle_adjust_y = (angle_adjust_y / image_height_) * fov_y_;

    // 限制步长
    // angle_adjust_x = clamp(angle_adjust_x, -max_angle_step_, max_angle_step_);
    // angle_adjust_y = clamp(angle_adjust_y, -max_angle_step_, max_angle_step_);

    // 像素控制为增量
    yaw_ += angle_adjust_x;
    pitch_ -= angle_adjust_y;

    prev_error_x_ = error_x;
    prev_error_y_ = error_y;
    return true;
}

bool Shoot::is_aimed_at_center(){
    double error_x = std::abs(static_cast<double>(target_pixel_x_) - center_x[class_id]);
    double error_y = std::abs(static_cast<double>(target_pixel_y_) - center_y[class_id]);

    return (error_x <= aim_tolerance_ && error_y <= aim_tolerance_);
}

double Shoot::apply_spread(double angle) {
    if (!enable_spread_) {
        return angle;
    }
    return angle + spread_dist_(gen_);
}

bool Shoot::is_angle_safe(double pitch, double yaw) {
    return (pitch >= min_pitch_angle_ && pitch <= max_pitch_angle_ &&
            yaw >= min_yaw_angle_ && yaw <= max_yaw_angle_);
}

bool Shoot::is_distance_safe(double distance) {
    return (distance >= min_safe_distance_ && distance <= max_effective_range_);
}

double Shoot::clamp(double value, double min_val, double max_val){
    return std::max(min_val, std::min(value, max_val));
}

void Shoot::timer_callback(){
    auto msg = tdt_interface::msg::SendData();
    // 检查目标是否过期（0.5秒无更新认为目标丢失）
    auto now = this->now();
    if (has_target_ && now.seconds()-last_target_time_.seconds() > 0.5) {
        has_target_ = false;
        RCLCPP_DEBUG(this->get_logger(), "目标丢失");
    }

    if (is_aiming_ && angles_initialized_ && has_target_ && target_confidence_ > confidence_threshold_) {
        bool angle_calculated = false;

        calculate_target_angles_from_pixel();
        angle_calculated = true;

        // 优先使用世界坐标计算云台角度（PNP 弹道）
        // if (is_world_coordinates_valid() && is_distance_safe(target_distance_)) {
        //     calculate_target_angles_from_world();
        //     angle_calculated = true;
        // }

        // // 如果世界坐标不可用，使用像素回退（微调）
        // if (!angle_calculated && target_pixel_x_ > 0 && target_pixel_y_ > 0 &&
            // target_pixel_x_ < image_width_ && target_pixel_y_ < image_height_) {
            // calculate_target_angles_from_pixel();
            // angle_calculated = true;
        // }

        if (angle_calculated) {
            double final_pitch = pitch_ ;
            double final_yaw = yaw_ ;
            static double last_published_yaw = final_yaw;
            static double last_published_pitch = final_pitch;
            double yaw_diff = std::fabs(final_yaw - last_published_yaw);
            double pitch_diff = std::fabs(final_pitch - last_published_pitch);

            // 只有当角度变化足够大时才发布（避免频繁小幅发布）
            // if (yaw_diff > 0.5 || pitch_diff > 0.25) {
                msg.pitch = final_pitch;
                msg.yaw = final_yaw;
               msg.if_shoot = auto_shoot_  || is_aimed_at_center();
                static int count=0;
                if(count++%10 == 0){
                    RCLCPP_INFO(this->get_logger(), "Output: yaw=%.2f, pitch=%.2f, conf=%.2f, fire=%s",
                                final_yaw, final_pitch,  target_confidence_,
                                 (msg.if_shoot ? "YES" : "NO"));
                }
                publisher_->publish(msg);
                last_published_yaw = final_yaw;
                last_published_pitch = final_pitch;
            // }
        }
    }
}

int main(int argc, char * argv[]){
    setenv("ROS_DOMAIN_ID", "0", 1);
    rclcpp::init(argc, argv);

    int id = 1;
    if (argc > 1) {
        try {
            id = std::stoi(argv[1]);
            if (id != 1 && id != 2) {
                std::cerr << "玩家ID必须是1或2" << std::endl;
                return 1;
            }
        } catch (const std::exception &e) {
            std::cerr << "玩家ID必须是数字" << std::endl;
            return 1;
        }
    }

    auto node = std::make_shared<Shoot>(id);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}