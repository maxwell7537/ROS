#include "shoot.hpp"

using namespace std;

Shoot::Shoot(int id) : Node("shoot_node"), player_id_(id) {
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
    
    // 声明通用参数
    this->declare_parameter("pitch_offset", 0.0);
    this->declare_parameter("yaw_offset", 0.0);
    this->declare_parameter("auto_shoot", true);
    this->declare_parameter("confidence_threshold", 0.3);
    
    // 获取通用参数
    pitch_offset_ = this->get_parameter("pitch_offset").as_double();
    yaw_offset_ = this->get_parameter("yaw_offset").as_double();
    auto_shoot_ = this->get_parameter("auto_shoot").as_bool();
    confidence_threshold_ = this->get_parameter("confidence_threshold").as_double();
    
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
    RCLCPP_INFO(this->get_logger(), "启用PNP世界坐标瞄准模式");
    RCLCPP_INFO(this->get_logger(), "弹道参数: 初速度=%.1fm/s, 重力=%.1fm/s²", bullet_speed_, gravity_);
#endif

#ifdef USE_PIXEL_CENTER_AIM
    // 像素瞄准模式专用参数声明
    this->declare_parameter("pixel_kp", 0.1);  // 增大默认比例系数
    this->declare_parameter("image_width", 640.0);
    this->declare_parameter("image_height", 480.0);
    this->declare_parameter("fov_x", 60.0);
    this->declare_parameter("fov_y", 45.0);
    this->declare_parameter("aim_tolerance", 40.0); // 增大默认容忍度
    this->declare_parameter("max_angle_step", 10.0); // 最大单步角度变化
    
    // 获取像素瞄准模式参数
    pixel_kp_ = this->get_parameter("pixel_kp").as_double();
    image_width_ = this->get_parameter("image_width").as_double();
    image_height_ = this->get_parameter("image_height").as_double();
    fov_x_ = this->get_parameter("fov_x").as_double();
    fov_y_ = this->get_parameter("fov_y").as_double();
    aim_tolerance_ = this->get_parameter("aim_tolerance").as_double();
    max_angle_step_ = this->get_parameter("max_angle_step").as_double();
    
    control_mode_ = "PIXEL_CENTER";
    RCLCPP_INFO(this->get_logger(), "启用像素中心瞄准模式");
    RCLCPP_INFO(this->get_logger(), "控制参数: KP=%.3f, 容忍度=%.1f像素", pixel_kp_, aim_tolerance_);
    RCLCPP_INFO(this->get_logger(), "图像尺寸: %dx%d, 视野: %.1fx%.1f°", (int)image_width_, (int)image_height_, fov_x_, fov_y_);
#endif

    // 创建发布者
    string topic_name = "target_angles_player_" + to_string(player_id_);
    publisher_ = this->create_publisher<tdt_interface::msg::SendData>(topic_name, 10);

    // 创建目标位置订阅者
    detection_result_sub_ = this->create_subscription<std_msgs::msg::Float32MultiArray>(
        "detection_result", 10,
        std::bind(&Shoot::detection_result_callback, this, std::placeholders::_1));
        
    // 创建参数回调
    parameters_callback_handle_ = this->add_on_set_parameters_callback(
        std::bind(&Shoot::parameters_callback, this, std::placeholders::_1));

    // 创建定时器
    timer_ = this->create_wall_timer(
        chrono::milliseconds(20),  // 降低控制频率到50Hz，使控制更稳定
        bind(&Shoot::timer_callback, this));
        
    // 创建调试信息发布器
    debug_publisher_ = this->create_publisher<std_msgs::msg::Float32MultiArray>("shoot_debug", 10);
            
    RCLCPP_INFO(this->get_logger(), "自动瞄准云台控制节点已启动，发布到话题: %s", topic_name.c_str());
    RCLCPP_INFO(this->get_logger(), "等待目标检测数据...");
}

rcl_interfaces::msg::SetParametersResult Shoot::parameters_callback(const std::vector<rclcpp::Parameter> &parameters) {
    rcl_interfaces::msg::SetParametersResult result;
    result.successful = true;
    
    for (const auto &param : parameters) {
        // 通用参数
        if (param.get_name() == "pitch_offset") {
            pitch_offset_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新俯仰角偏移: %.2f°", pitch_offset_);
        } else if (param.get_name() == "yaw_offset") {
            yaw_offset_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新偏航角偏移: %.2f°", yaw_offset_);
        } else if (param.get_name() == "auto_shoot") {
            auto_shoot_ = param.as_bool();
            RCLCPP_INFO(this->get_logger(), "更新自动射击: %s", auto_shoot_ ? "开启" : "关闭");
        } else if (param.get_name() == "confidence_threshold") {
            confidence_threshold_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新置信度阈值: %.2f", confidence_threshold_);
        }
        
#ifdef USE_PNP_WORLD_COORDINATES
        // PNP模式专用参数
        else if (param.get_name() == "bullet_speed") {
            bullet_speed_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新子弹初速度: %.1f m/s", bullet_speed_);
        } else if (param.get_name() == "gravity") {
            gravity_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新重力加速度: %.1f m/s²", gravity_);
        } else if (param.get_name() == "camera_height") {
            camera_height_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新相机高度: %.2f m", camera_height_);
        }
#endif

#ifdef USE_PIXEL_CENTER_AIM
        // 像素瞄准模式专用参数
        else if (param.get_name() == "pixel_kp") {
            pixel_kp_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新像素控制比例系数: %.3f", pixel_kp_);
        } else if (param.get_name() == "image_width") {
            image_width_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新图像宽度: %.0f", image_width_);
        } else if (param.get_name() == "image_height") {
            image_height_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新图像高度: %.0f", image_height_);
        } else if (param.get_name() == "fov_x") {
            fov_x_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新水平视野角度: %.1f°", fov_x_);
        } else if (param.get_name() == "fov_y") {
            fov_y_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新垂直视野角度: %.1f°", fov_y_);
        } else if (param.get_name() == "aim_tolerance") {
            aim_tolerance_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新瞄准容忍度: %.1f像素", aim_tolerance_);
        } else if (param.get_name() == "max_angle_step") {
            max_angle_step_ = param.as_double();
            RCLCPP_INFO(this->get_logger(), "更新最大角度步长: %.1f°", max_angle_step_);
        }
#endif
    }
    
    return result;
}

void Shoot::timer_callback() {
    auto msg = tdt_interface::msg::SendData();
    
    if (has_target_ && target_confidence_ > confidence_threshold_) {
        bool valid_target = false;
        std::string used_mode = "NONE";
        
#ifdef USE_PNP_WORLD_COORDINATES
        // 优先使用PNP世界坐标模式
        if (is_world_coordinates_valid()) {
            valid_target = calculate_target_angles_from_world();
            used_mode = "PNP_WORLD";
        }
#endif

        // 如果PNP模式无效，回退到像素瞄准模式
#ifdef USE_PIXEL_CENTER_AIM
        if (!valid_target) {
            // 检查像素坐标是否有效
            if (target_pixel_x_ > 0 && target_pixel_y_ > 0 && 
                target_pixel_x_ < image_width_ && target_pixel_y_ < image_height_) {
                valid_target = calculate_target_angles_from_pixel();
                used_mode = "PIXEL_CENTER";
            }
        }
#endif

        if (valid_target) {
            // 应用角度偏移并限制范围
            double final_pitch = clamp(pitch_ + pitch_offset_, -17.0, 60.0);
            double final_yaw = clamp(yaw_ + yaw_offset_, 0.0, 180.0);
            
            msg.pitch = final_pitch;
            msg.yaw = final_yaw;
            
            // 检查是否瞄准中心
            bool is_aiming_center = is_aimed_at_center();
            
            // 只在瞄准中心时自动射击
            bool should_shoot = auto_shoot_ && is_aiming_center;
            
            msg.if_shoot = should_shoot;
            
            // 发布调试信息
            publish_debug_info(final_pitch, final_yaw);
            
            // 减少日志输出频率
            static int log_counter = 0;
            if (log_counter++ % 50 == 0) {
                RCLCPP_INFO(this->get_logger(), 
                    "自动瞄准[%s] - 角度: pitch=%.2f, yaw=%.2f, 瞄准: %s, 射击: %s", 
                    used_mode.c_str(), final_pitch, final_yaw,
                    is_aiming_center ? "是" : "否",
                    should_shoot ? "是" : "否");
            }
        } else {
            // 目标无效
            msg.pitch = pitch_;
            msg.yaw = yaw_;
            msg.if_shoot = false;
            
            static int invalid_counter = 0;
            if (invalid_counter++ % 100 == 0) {
                RCLCPP_WARN(this->get_logger(), "目标无效 - 像素: (%d,%d)", 
                           (int)target_pixel_x_, (int)target_pixel_y_);
            }
        }
    } else {
        // 没有目标时停止射击
        msg.pitch = pitch_;
        msg.yaw = yaw_;
        msg.if_shoot = false;
        
        static int no_target_counter = 0;
        if (no_target_counter++ % 100 == 0) {
            if (!has_target_) {
                RCLCPP_DEBUG(this->get_logger(), "等待目标...");
            } else {
                RCLCPP_DEBUG(this->get_logger(), "目标置信度 %.2f 低于阈值 %.2f", 
                           target_confidence_, confidence_threshold_);
            }
        }
    }
    
    // 发布消息
    publisher_->publish(msg);
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
        
        // 检查数据有效性
        bool data_valid = (target_pixel_x_ > 0 && target_pixel_y_ > 0 && 
                          target_pixel_x_ < 2000 && target_pixel_y_ < 2000 && // 合理的图像尺寸范围
                          target_confidence_ > 0.1);
        
        has_target_ = data_valid;
        
        if (data_valid) {
            last_target_time_ = this->now();
            
            // 减少日志输出频率
            static int target_log_counter = 0;
            if (target_log_counter++ % 50 == 0) {
                RCLCPP_DEBUG(this->get_logger(), 
                    "收到目标 - 像素: (%.1f, %.1f), 世界: (%.2f, %.2f, %.2f), 距离: %.2f, 置信度: %.2f",
                    target_pixel_x_, target_pixel_y_,
                    target_world_x_, target_world_y_, target_world_z_,
                    target_distance_, target_confidence_);
            }
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
    double center_y = image_height_ / 2.0;
    
    // 计算像素偏差
    double error_x = target_pixel_x_ - center_x;
    double error_y = target_pixel_y_ - center_y;
    
    // 计算角度调整量（关键修改：使用增量控制而不是绝对位置）
    // 将像素误差转换为角度调整量
    double angle_adjust_x = (error_x / image_width_) * fov_x_ * pixel_kp_;
    double angle_adjust_y = (error_y / image_height_) * fov_y_ * pixel_kp_;
    
    // 限制单步角度变化，避免过冲
    angle_adjust_x = clamp(angle_adjust_x, -max_angle_step_, max_angle_step_);
    angle_adjust_y = clamp(angle_adjust_y, -max_angle_step_, max_angle_step_);
    
    // 关键修改：基于当前角度进行增量调整
    // 目标在右侧时，需要向左转（减小yaw）
    yaw_ = yaw_ + angle_adjust_x;
    
    // 目标在下方时，需要向上抬（增加pitch）
    pitch_ = pitch_ - angle_adjust_y;
    
    // 限制角度范围
    yaw_ = clamp(yaw_, 0.0, 180.0);
    pitch_ = clamp(pitch_, -17.0, 60.0);
    
    // 详细的调试信息
    static int debug_counter = 0;
    if (debug_counter++ % 10 == 0) {
        RCLCPP_INFO(this->get_logger(), 
            "像素控制 - 目标: (%.1f, %.1f), 中心: (%.1f, %.1f), 误差: (%.1f, %.1f)", 
            target_pixel_x_, target_pixel_y_, center_x, center_y, error_x, error_y);
        RCLCPP_INFO(this->get_logger(), 
            "角度调整 - X: %.3f°, Y: %.3f°, 新角度: pitch=%.2f, yaw=%.2f", 
            angle_adjust_x, angle_adjust_y, pitch_, yaw_);
    }
    
    return true;
}

bool Shoot::is_aimed_at_center() {
    // 检查是否已经瞄准中心（在容忍度范围内）
    double center_x = image_width_ / 2.0;
    double center_y = image_height_ / 2.0;
    
    double error_x = abs(target_pixel_x_ - center_x);
    double error_y = abs(target_pixel_y_ - center_y);
    
    bool aimed = (error_x <= aim_tolerance_ && error_y <= aim_tolerance_);
    
    return aimed;
}
#endif

double Shoot::clamp(double value, double min_val, double max_val) {
    return std::max(min_val, std::min(value, max_val));
}

void Shoot::publish_debug_info(double final_pitch, double final_yaw) {
    auto debug_msg = std_msgs::msg::Float32MultiArray();
    
#ifdef USE_PIXEL_CENTER_AIM
    double center_x = image_width_ / 2.0;
    double center_y = image_height_ / 2.0;
    double error_x = target_pixel_x_ - center_x;
    double error_y = target_pixel_y_ - center_y;
    bool is_aimed = is_aimed_at_center();
    
    // 计算误差百分比
    double error_percent_x = error_x / (image_width_ / 2.0);
    double error_percent_y = error_y / (image_height_ / 2.0);
#else
    double error_x = 0;
    double error_y = 0;
    bool is_aimed = false;
    double error_percent_x = 0;
    double error_percent_y = 0;
#endif
    
    debug_msg.data = {
        static_cast<float>(final_pitch),                 // 0: 最终俯仰角
        static_cast<float>(final_yaw),                   // 1: 最终偏航角
        static_cast<float>(pitch_),                      // 2: 计算出的俯仰角
        static_cast<float>(yaw_),                        // 3: 计算出的偏航角
        static_cast<float>(target_distance_),            // 4: 目标距离
        static_cast<float>(target_confidence_),          // 5: 目标置信度
        static_cast<float>(target_pixel_x_),             // 6: 目标像素X
        static_cast<float>(target_pixel_y_),             // 7: 目标像素Y
        static_cast<float>(target_world_x_),             // 8: 目标世界X
        static_cast<float>(target_world_y_),             // 9: 目标世界Y
        static_cast<float>(target_world_z_),             // 10: 目标世界Z
        static_cast<float>(error_x),                     // 11: 像素X误差
        static_cast<float>(error_y),                     // 12: 像素Y误差
        static_cast<float>(is_aimed ? 1.0f : 0.0f),     // 13: 是否瞄准中心
        static_cast<float>(error_percent_x),             // 14: X误差百分比
        static_cast<float>(error_percent_y),             // 15: Y误差百分比
        static_cast<float>(pixel_kp_),                   // 16: 当前KP值
        static_cast<float>(aim_tolerance_)               // 17: 当前容忍度
    };
    
    debug_publisher_->publish(debug_msg);
}

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