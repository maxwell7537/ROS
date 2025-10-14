#include <string>
#include <algorithm>
#include "rclcpp/rclcpp.hpp"
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/image_encodings.hpp"
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <memory>
#include <thread>
#include <chrono>
#include <iostream>
#include <atomic>

class move:public rclcpp::Node{
public:
    move(int id=1):Node("move_node"){
        player_id_ = id;
        
        // 写死的参数
        max_speed_ = 3.0;
        rotation_speed_ = 1.0;
        
        // 创建发布者
        std::string topic_name = "target_speed_player_" + std::to_string(player_id_);
        speed_pub_ = this->create_publisher<geometry_msgs::msg::TwistStamped>(topic_name, 10);
        
        // 初始化X11显示连接
        display_ = XOpenDisplay(nullptr);
        if (!display_) {
            RCLCPP_ERROR(this->get_logger(), "无法打开X11显示连接");
            return;
        }
        
        // 启动键盘监听线程
        keyboard_thread_ = std::thread(&move::keyboardLoop, this);
        
        // 创建定时器以定期发布控制消息
        control_timer_ = this->create_wall_timer(
            std::chrono::milliseconds(50),  // 20Hz
            std::bind(&move::publishControl, this));
        
        RCLCPP_INFO(this->get_logger(), "Player control node initialized for player %d", player_id_);
        RCLCPP_INFO(this->get_logger(), "Controls: WASD for movement, Q/E for rotation");
        RCLCPP_INFO(this->get_logger(), "Global keyboard hook active - no window focus needed");
    }
    ~move()
    {
        running_ = false;
        if (keyboard_thread_.joinable()) {
            keyboard_thread_.join();
        }
        
        if (display_) {
            XCloseDisplay(display_);
        }
    }

private:
   void keyboardLoop()
    {
        // 创建一个不可见的窗口用于接收键盘事件
        Window root_window = DefaultRootWindow(display_);
        Window event_window = XCreateSimpleWindow(display_, root_window, 0, 0, 1, 1, 0, 0, 0);
        
        // 选择键盘事件
        XSelectInput(display_, event_window, KeyPressMask | KeyReleaseMask);
        
        // 将窗口映射到屏幕上
        XMapWindow(display_, event_window);
        
        // 同步并刷新
        XSync(display_, False);
        
        // 键位状态数组
        bool key_states[256];
        std::memset(key_states, 0, sizeof(key_states));
        
        XEvent event;
        
        while (running_) {
            // 检查是否有待处理的事件
            while (XPending(display_) > 0) {
                XNextEvent(display_, &event);
                
                if (event.type == KeyPress || event.type == KeyRelease) {
                    bool is_pressed = (event.type == KeyPress);
                    KeySym key_sym = XLookupKeysym(&event.xkey, 0);
                    
                    // 更新键位状态
                    if (event.xkey.keycode < 256) {
                        key_states[event.xkey.keycode] = is_pressed;
                    }
                    
                    // 显示按键信息
                    char* key_name = XKeysymToString(key_sym);
                    RCLCPP_INFO(this->get_logger(), "Key %s: %s (keycode: %d)", 
                                is_pressed ? "pressed" : "released", 
                                key_name ? key_name : "unknown", 
                                event.xkey.keycode);
                }
            }
            
            // 处理移动输入 (WASD)
            float move_x = 0.0f;  // 前后移动
            float move_z = 0.0f;  // 左右移动
            
            // 检查WASD键状态
            // W键
            if (key_states[XKeysymToKeycode(display_, XK_w)]) move_x += 1.0f;
            // S键
            if (key_states[XKeysymToKeycode(display_, XK_s)]) move_x -= 1.0f;
            // A键
            if (key_states[XKeysymToKeycode(display_, XK_a)]) move_z += 1.0f;
            // D键
            if (key_states[XKeysymToKeycode(display_, XK_d)]) move_z -= 1.0f;
            
            // 处理旋转输入 (Q/E)
            float rotation = 0.0f;
            // Q键
            if (key_states[XKeysymToKeycode(display_, XK_q)]) rotation -= 1.0f;
            // E键
            if (key_states[XKeysymToKeycode(display_, XK_e)]) rotation += 1.0f;
            
            // 更新控制值
            {
                std::lock_guard<std::mutex> lock(control_mutex_);
                move_x_ = move_x;
                move_z_ = move_z;
                rotation_ = rotation;
            }
            
            // 如果有按键输入，显示当前控制状态
            if (move_x != 0.0f || move_z != 0.0f || rotation != 0.0f) {
                RCLCPP_DEBUG(this->get_logger(), "Control input - Move X: %.1f, Move Z: %.1f, Rotation: %.1f", 
                             move_x, move_z, rotation);
            }
            
            // 控制循环频率
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        // 清理窗口
        XDestroyWindow(display_, event_window);
    }
    
    void publishControl()
    {
        auto message = geometry_msgs::msg::TwistStamped();
        
        // 设置时间戳
        message.header.stamp = this->now();
        message.header.frame_id = "base_link";
        
        // 获取当前控制值
        float move_x, move_z, rotation;
        {
            std::lock_guard<std::mutex> lock(control_mutex_);
            move_x = move_x_;
            move_z = move_z_;
            rotation = rotation_;
        }
        
        // 设置线性速度
        message.twist.linear.x = move_x * max_speed_;
        message.twist.linear.y = move_z * max_speed_;
        message.twist.linear.z = 0.0;
        
        // 设置角速度（绕Z轴旋转）
        message.twist.angular.x = 0.0;
        message.twist.angular.y = 0.0;
        message.twist.angular.z = rotation * rotation_speed_;
        
        // 发布消息
        speed_pub_->publish(message);
        
        // 显示发布的控制信息（仅在有控制输入时）
        if (move_x != 0.0f || move_z != 0.0f || rotation != 0.0f) {
            RCLCPP_DEBUG(this->get_logger(), "Publishing control - Linear: [%.2f, %.2f, %.2f], Angular: [%.2f, %.2f, %.2f]",
                         message.twist.linear.x, message.twist.linear.y, message.twist.linear.z,
                         message.twist.angular.x, message.twist.angular.y, message.twist.angular.z);
        }
    }
    
    // 节点参数
    int player_id_ = 1;
    double max_speed_ = 3.0;
    double rotation_speed_ = 1.0;
    
    // 控制值
    float move_x_ = 0.0f;
    float move_z_ = 0.0f;
    float rotation_ = 0.0f;
    std::mutex control_mutex_;
    
    // ROS2组件
    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr speed_pub_;
    rclcpp::TimerBase::SharedPtr control_timer_;
    
    // X11组件
    Display* display_ = nullptr;
    
    // 线程控制
    std::thread keyboard_thread_;
    std::atomic<bool> running_{true};
};

int main(int argc, char * argv[])
{
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
    
    std::cout << "启动玩家" << player_id << "控制节点..." << std::endl;
    std::cout << "全局键盘钩子已激活 - 无需窗口焦点" << std::endl;
    std::cout << "按Ctrl+C退出" << std::endl;
    
    // 初始化ROS2
    rclcpp::init(argc, argv);
    auto node = std::make_shared<move>(player_id);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}



