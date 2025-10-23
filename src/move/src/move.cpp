#include <string>
#include "rclcpp/rclcpp.hpp"
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/twist.hpp> 
#include <memory>

class AutoMove : public rclcpp::Node {
public:
    AutoMove(int id = 1) : Node("auto_move_node") {
        player_id_ = id;
        
        // 可配置的参数
        control_rate_ = 20.0;  // 控制频率 Hz
        
        // 创建发布者 - 发布速度命令
        std::string topic_name = "target_speed_player_" + std::to_string(player_id_);
        speed_pub_ = this->create_publisher<geometry_msgs::msg::TwistStamped>(topic_name, 10);
        
        // 创建订阅者 - 从core_node接收控制命令
        std::string control_topic = "control_cmd_player_" + std::to_string(player_id_);
        control_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            control_topic, 10,
            std::bind(&AutoMove::controlCallback, this, std::placeholders::_1));
        
        // 创建定时器以定期发布控制消息
        int timer_ms = static_cast<int>(1000.0 / control_rate_);
        control_timer_ = this->create_wall_timer(
            std::chrono::milliseconds(timer_ms),
            std::bind(&AutoMove::publishControl, this));
        
        RCLCPP_INFO(this->get_logger(), "Auto move node initialized for player %d", player_id_);
        RCLCPP_INFO(this->get_logger(), "Subscribing to: %s", control_topic.c_str());
        RCLCPP_INFO(this->get_logger(), "Publishing to: %s", topic_name.c_str());
    }

private:
    void controlCallback(const geometry_msgs::msg::Twist::SharedPtr msg) {
        // 更新当前控制命令
        std::lock_guard<std::mutex> lock(control_mutex_);
        current_control_ = *msg;
        
        RCLCPP_DEBUG(this->get_logger(), 
                    "Received control - Linear: [%.2f, %.2f, %.2f], Angular: [%.2f, %.2f, %.2f]",
                    msg->linear.x, msg->linear.y, msg->linear.z,
                    msg->angular.x, msg->angular.y, msg->angular.z);
    }
    
    void publishControl() {
        auto message = geometry_msgs::msg::TwistStamped();
        
        // 设置时间戳
        message.header.stamp = this->now();
        message.header.frame_id = "base_link";
        
        // 获取当前控制命令
        geometry_msgs::msg::Twist control_cmd;
        {
            std::lock_guard<std::mutex> lock(control_mutex_);
            control_cmd = current_control_;
        }
        
        // 设置速度命令
        message.twist.linear.x = control_cmd.linear.x;
        message.twist.linear.y = control_cmd.linear.y;
        message.twist.linear.z = control_cmd.linear.z;
        message.twist.angular.x = control_cmd.angular.x;
        message.twist.angular.y = control_cmd.angular.y;
        message.twist.angular.z = control_cmd.angular.z;
        
        // 发布消息
        speed_pub_->publish(message);
        
        // 偶尔输出日志，避免过于频繁
        static int count = 0;
        if (count++ % 100 == 0) {  // 每100次发布输出一次
            RCLCPP_DEBUG(this->get_logger(), 
                        "Publishing control - Linear: [%.2f, %.2f, %.2f], Angular: [%.2f, %.2f, %.2f]",
                        message.twist.linear.x, message.twist.linear.y, message.twist.linear.z,
                        message.twist.angular.x, message.twist.angular.y, message.twist.angular.z);
        }
    }
    
    // 节点参数
    int player_id_ = 1;
    double control_rate_ = 20.0;
    
    // 控制命令
    geometry_msgs::msg::Twist current_control_;
    std::mutex control_mutex_;
    
    // ROS2组件
    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr speed_pub_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr control_sub_;
    rclcpp::TimerBase::SharedPtr control_timer_;
};

int main(int argc, char * argv[]) {
    // 检查参数
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " <player_id>" << std::endl;
        std::cerr << "例如: " << argv[0] << " 1" << std::endl;
        std::cerr << "     " << argv[0] << " 2" << std::endl;
        return 1;
    }
    
    // 解析玩家ID
    int player_id = atoi(argv[1]);
    if (player_id != 1 && player_id != 2) {
        std::cerr << "错误: player_id必须是1或2" << std::endl;
        return 1;
    }
    
    std::cout << "启动玩家" << player_id << "自动移动节点..." << std::endl;
    std::cout << "节点将根据core_node的控制命令移动" << std::endl;
    std::cout << "按Ctrl+C退出" << std::endl;
    
    // 初始化ROS2
    rclcpp::init(argc, argv);
    auto node = std::make_shared<AutoMove>(player_id);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}