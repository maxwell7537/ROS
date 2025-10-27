#include "shoot.hpp"
#include <std_msgs/msg/int32.hpp>

using namespace std;

Shoot::Shoot(int id) : Node("shoot_node"), player_id_(id) {
    // 初始化云台角度
    pitch_ = 0.0;
    yaw_ = 0.0;
    
    // 初始化目标位置
    target_pixel_x_ = 0.0f;
    target_pixel_y_ = 0.0f;
    target_world_x_ = 0.0f;
    target_world_y_ = 0.0f;
    target_world_z_ = 0.0f;
    target_distance_ = 0.0f;
    target_confidence_ = 0.0f;
    has_target_ = false;
    
    pitch_offset_ =0.0;
    yaw_offset_ = 0.0;
    auto_shoot_ =0;   //全程射击
    confidence_threshold_ =0.3;
    
    prev_error_x_ = 0.0;
    prev_error_y_ = 0.0;
    integral_x_ = 0.0;
    integral_y_ = 0.0;
    angles_initialized_ = false;
    base_yaw_ = 0.0;
    turret_yaw_offset_ = 0.0;
    target_turret_yaw_ = 0.0;
    // 射击状态发布
    shoot_state_sub_ = this->create_subscription<std_msgs::msg::Int32>(
            "shoot_state", 10,
            std::bind(&Shoot::shoot_state_callback, this, std::placeholders::_1));
    is_aiming_ = false;
    
#ifdef USE_PNP_WORLD_COORDINATES
    // PNP模式专用参数声明
    this->declare_parameter("bullet_speed", 25.0);
    this->declare_parameter("gravity", 9.8);
    this->declare_parameter("camera_height", 0.5);
    
    // 获取PNP模式参数
    bullet_speed_ = this->get_parameter("bullet_speed").as_double();
    gravity_ = this->get_parameter("gravity").as_double();
    camera_height_ = this->get_parameter("camera_height").as_double();
    
    control_mode_ = "PNP_WORLD";
#endif

#ifdef USE_PIXEL_CENTER_AIM
    // 调整后的参数
    pixel_kp_ = 0.03;          // 比例系数
    pixel_kd_ = 0.05;          // 微分系数，抑制振荡
    pixel_ki_ = 0.001;         // 积分系数，消除静差
    image_width_ = 640.0;
    image_height_ = 480.0;
    fov_x_ = 60.0;
    fov_y_ = 45.0;
    aim_tolerance_ = 35.0;     // 容忍度，避免频繁开关
    max_angle_step_ = 3.0;     // 最大步长，运动更平滑
    max_i_term_ = 5.0;         // 积分限幅
    control_mode_ = "PIXEL_CENTER";
#endif

    string topic_name = "target_angles_player_" + to_string(player_id_);
    publisher_ = this->create_publisher<tdt_interface::msg::SendData>(topic_name, 10);

    detection_result_sub_ = this->create_subscription<std_msgs::msg::Float32MultiArray>(
        "detection_result", 10,
        bind(&Shoot::detection_result_callback, this, placeholders::_1));

    string angles_topic = "real_angles_player_" + to_string(player_id_);
    angles_sub_ = this->create_subscription<tdt_interface::msg::ReceiveData>(
        angles_topic, 10,
        std::bind(&Shoot::angles_callback, this, std::placeholders::_1));
    #ifdef USE_PNP_WORLD_COORDINATES
        parameters_callback_handle_ = this->add_on_set_parameters_callback(
            std::bind(&Shoot::parameters_callback, this, std::placeholders::_1));
    #endif

    timer_ = this->create_wall_timer(
        chrono::milliseconds(20),  // 频率100Hz
        bind(&Shoot::timer_callback, this));
    RCLCPP_INFO(this->get_logger(), "shoot start");
}

#ifdef USE_PNP_WORLD_COORDINATES
rcl_interfaces::msg::SetParametersResult Shoot::parameters_callback(const std::vector<rclcpp::Parameter> &parameters) {
    rcl_interfaces::msg::SetParametersResult result;
    result.successful = true;
    for (const auto &param : parameters) {
    if(param.get_name() == "bullet_speed") {
            bullet_speed_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新子弹初速度: %.1f m/s", bullet_speed_);
        } else if (param.get_name() == "gravity") {
            gravity_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新重力加速度: %.1f m/s²", gravity_);
        } else if (param.get_name() == "camera_height") {
            camera_height_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新相机高度: %.2f m", camera_height_);
        }
    }
}
#endif

void Shoot::angles_callback(const tdt_interface::msg::ReceiveData::SharedPtr msg) {
    // 更新当前底盘角度
    base_yaw_ = msg->yaw;
    
    if (!angles_initialized_) {
        // 第一次收到角度信息时初始化控制角度
        yaw_ = base_yaw_;
        pitch_ = 0.0;
        angles_initialized_ = true;
        RCLCPP_INFO(this->get_logger(), "云台角度初始化完成: base_yaw=%.2f, control_yaw=%.2f", base_yaw_, yaw_);
    }
    
    // 实时更新当前角度用于显示
    current_yaw_ = msg->yaw;
    current_pitch_ = msg->pitch;
}
void Shoot::shoot_state_callback(const std_msgs::msg::Int32::SharedPtr msg) {
    if (msg->data == 1) {
        is_aiming_ = true;
        
        // 重要修改：在获得射击授权时，将控制角度同步到当前实际角度
        if (angles_initialized_) {
            yaw_ = base_yaw_;  // 使用底盘角度作为初始控制角度
            pitch_ = 0.0;      // 重置俯仰角
            
            #ifdef USE_PIXEL_CENTER_AIM
            prev_error_x_ = 0.0;
            prev_error_y_ = 0.0;
            integral_x_ = 0.0;
            integral_y_ = 0.0;
            #endif
            
            RCLCPP_INFO(this->get_logger(), "射击授权激活，角度已同步: yaw=%.2f", yaw_);
        }
        
        RCLCPP_INFO(this->get_logger(), "收到射击授权，开始控制云台并射击");
    } else {
        is_aiming_ = false;
        RCLCPP_INFO(this->get_logger(), "射击授权收回，停止射击");
    }
}
void Shoot::timer_callback(){
    auto msg = tdt_interface::msg::SendData();
    
    if (is_aiming_ && angles_initialized_ && has_target_ && target_confidence_ > confidence_threshold_) {
        string used_mode = "NONE";
        bool angle_calculated = false;
        
#ifdef USE_PNP_WORLD_COORDINATES
        if (is_world_coordinates_valid()) {
            calculate_target_angles_from_world();
            used_mode = "PNP_WORLD";
            angle_calculated = true;
        }
#endif

#ifdef USE_PIXEL_CENTER_AIM
        if (!angle_calculated && target_pixel_x_ > 0 && target_pixel_y_ > 0 && 
            target_pixel_x_ < image_width_ && target_pixel_y_ < image_height_) {
            calculate_target_angles_from_pixel();
            used_mode = "PIXEL_CENTER";
            angle_calculated = true;
        }
#endif
        
        if (angle_calculated) {
            // 重要修改：直接使用计算出的角度，不添加偏移量
            double final_pitch = pitch_;
            double final_yaw = yaw_;
            
            static double last_published_yaw = final_yaw;
            static double last_published_pitch = final_pitch;
            double yaw_diff = fabs(final_yaw - last_published_yaw);
            double pitch_diff = fabs(final_pitch - last_published_pitch);

            // 只有当角度变化足够大时才发布
            if (yaw_diff > 0.5 || pitch_diff > 0.5) {
                msg.pitch = final_pitch;
                msg.yaw = final_yaw;
                msg.if_shoot = auto_shoot_ || is_aimed_at_center();

                RCLCPP_INFO(this->get_logger(), "控制输出: yaw=%.2f, pitch=%.2f, 模式=%s", 
                           final_yaw, final_pitch, used_mode.c_str());
                
                publisher_->publish(msg);
                last_published_yaw = final_yaw;
                last_published_pitch = final_pitch;
            }
        }
    }
}

void Shoot::detection_result_callback(const std_msgs::msg::Float32MultiArray::SharedPtr msg) {
    if (msg->data.size() >= 11) {
        // 解析完整检测结果
        target_pixel_x_ = msg->data[0];
        target_pixel_y_ = msg->data[1];
        target_world_x_ = msg->data[2];
        target_world_y_ = msg->data[3];
        target_world_z_ = msg->data[4];
        target_distance_ = msg->data[5];
        target_confidence_ = msg->data[6];
        bool data_valid = (target_pixel_x_ > 0 && target_pixel_y_ > 0 && 
                          target_pixel_x_ < 2000 && target_pixel_y_ < 2000 && // 合理的图像尺寸范围
                          target_confidence_ > 0.1);
        
        has_target_ = data_valid;
        if (data_valid) {
            last_target_time_ = this->now();
        }
    } else {
        has_target_ = false;
    }
}

bool Shoot::is_world_coordinates_valid() {
#ifdef USE_PNP_WORLD_COORDINATES
    // 检查世界坐标是否有效
    if (target_distance_ <= 0) return false;
    
    // 检查坐标是否为异常值（全零）
    if (target_world_x_ == 0.0f && target_world_y_ == 0.0f && target_world_z_ == 0.0f) {
        return false;
    }
    
    return true;
#else
    return false;
#endif
}

#ifdef USE_PNP_WORLD_COORDINATES
bool Shoot::calculate_target_angles_from_world() {
    // 根据目标在世界坐标系中的位置计算云台角度
    
    // 计算偏航角 (yaw) - 水平方向
    yaw_ = atan2(target_world_x_, target_world_z_) * (180.0 / M_PI) + 90;
    
    // 计算俯仰角 (pitch) - 垂直方向  
    double horizontal_distance = sqrt(target_world_x_ * target_world_x_ + target_world_z_ * target_world_z_);
    double pitch_no_gravity = atan2(target_world_y_ + camera_height_, horizontal_distance) * (180.0 / M_PI);
    
    // 计算重力补偿
    double gravity_compensation = calculate_gravity_compensation(horizontal_distance, target_world_y_ + camera_height_);
    
    // 应用重力补偿
    pitch_ = pitch_no_gravity + gravity_compensation;
    
    return true;
}

double Shoot::calculate_gravity_compensation(double horizontal_distance, double target_height) {
    // 计算重力引起的弹道下坠补偿角度
    
    if (bullet_speed_ <= 0 || horizontal_distance <= 0) {
        return 0.0;
    }
    
    // 计算子弹飞行时间
    double time_to_target = horizontal_distance / bullet_speed_;
    
    // 计算重力引起的下坠距离
    double drop_distance = 0.5 * gravity_ * time_to_target * time_to_target;
    
    // 计算需要补偿的角度（弧度）
    double compensation_rad = atan2(drop_distance, horizontal_distance);
    
    // 转换为度
    return compensation_rad * (180.0 / M_PI);
}
#endif

#ifdef USE_PIXEL_CENTER_AIM
bool Shoot::calculate_target_angles_from_pixel() {
    // 计算图像中心
    double center_x = image_width_ / 2.0;
    double center_y = image_height_ / 2.0 - 15;
    
    // 计算当前误差
    double error_x = target_pixel_x_ - center_x;
    double error_y = target_pixel_y_ - center_y;
    
    // 计算微分项
    double derivative_x = error_x - prev_error_x_;
    double derivative_y = error_y - prev_error_y_;
    
    // 计算积分项
    integral_x_ += error_x;
    integral_y_ += error_y;
    
    // 积分限幅
    integral_x_ = clamp(integral_x_, -max_i_term_, max_i_term_);
    integral_y_ = clamp(integral_y_, -max_i_term_, max_i_term_);
    
    // PID计算
    double angle_adjust_x = pixel_kp_ * error_x + pixel_kd_ * derivative_x + pixel_ki_ * integral_x_;
    double angle_adjust_y = pixel_kp_ * error_y + pixel_kd_ * derivative_y + pixel_ki_ * integral_y_;
    
    // 归一化并转换为角度
    angle_adjust_x = (angle_adjust_x / image_width_) * fov_x_;
    angle_adjust_y = (angle_adjust_y / image_height_) * fov_y_;
    
    // 限制单步角度变化
    angle_adjust_x = clamp(angle_adjust_x, -max_angle_step_, max_angle_step_);
    angle_adjust_y = clamp(angle_adjust_y, -max_angle_step_, max_angle_step_);
    
    // 重要修改：直接更新yaw_和pitch_，而不是使用偏移量
    yaw_ += angle_adjust_x;
    pitch_ -= angle_adjust_y;
    
    // 保存当前误差
    prev_error_x_ = error_x;
    prev_error_y_ = error_y;
    
    // // 角度限幅
    // yaw_ = clamp(yaw_, 0.0, 180.0);
    // pitch_ = clamp(pitch_, -17.0, 60.0);
    
    // RCLCPP_INFO(this->get_logger(),
    //                      "像素瞄准: 误差(%.1f,%.1f), 调整(%.3f,%.3f), 角度(%.2f,%.2f)",
    //                      error_x, error_y, angle_adjust_x, angle_adjust_y, yaw_, pitch_);
    
    return true;
}
#endif
bool Shoot::is_aimed_at_center() {
    double center_x = image_width_ / 2.0;
    double center_y = image_height_ / 2.0;
    
    double error_x = abs(target_pixel_x_ - center_x);
    double error_y = abs(target_pixel_y_ - center_y);
    
    bool aimed = (error_x <= aim_tolerance_ && error_y <= aim_tolerance_);
    
    return aimed;
}

double Shoot::clamp(double value, double min_val, double max_val) {
    return max(min_val, min(value, max_val));
}


int main(int argc, char * argv[]) {
    setenv("ROS_DOMAIN_ID", "0", 1);
    rclcpp::init(argc, argv);
    int id=1;
    if(argc>1){
        try{
            id=stoi(argv[1]);
            if (id!=1&&id!=2){
                cerr << "玩家ID必须是1或2" << endl;
                return 1;
            }
        }catch(const exception& e){
            cerr << "玩家ID必须是数字" << endl;
            return 1;
        }
    }
    auto node = make_shared<Shoot>(id);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}