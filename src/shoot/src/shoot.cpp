#include <string>
#include <algorithm>
#include "rclcpp/rclcpp.hpp"
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/image_encodings.hpp"
#include <SDL2/SDL.h>
#include "my_msgs/msg/send_data.hpp"

class shoot:public rclcpp::Node{
public:
    shoot(int id=1):Node("shoot_node"){
        this->player_id = id;
        // 初始化云台角度
        pitch_ = 0.0;
        yaw_ = 0.0;
        
        // 初始化鼠标状态
        mouse_x_ = 0;
        mouse_y_ = 0;
        is_mouse_captured_ = false;
        
        // 创建发布者
        std::string topic_name = "target_angles_player_" + std::to_string(player_id);
        publisher_ = this->create_publisher<my_msgs::msg::SendData>(topic_name, 10);
        
        // 初始化SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            RCLCPP_ERROR(this->get_logger(), "无法初始化SDL: %s", SDL_GetError());
            return;
        }
        
        // 创建一个不可见窗口以捕获鼠标输入
        window_ = SDL_CreateWindow("Gimbal Controller",
                                  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  100, 100,
                                  SDL_WINDOW_HIDDEN);
        
        if (!window_) {
            RCLCPP_ERROR(this->get_logger(), "无法创建SDL窗口: %s", SDL_GetError());
            SDL_Quit();
            return;
        }
        
        // 捕获鼠标
        SDL_SetRelativeMouseMode(SDL_TRUE);
        is_mouse_captured_ = true;
        
        // 创建定时器
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(10),  // 20Hz
            std::bind(&shoot::timer_callback, this));
            
        RCLCPP_INFO(this->get_logger(), "云台控制节点已启动，发布到话题: %s", topic_name.c_str());
        RCLCPP_INFO(this->get_logger(), "使用鼠标控制云台，按ESC退出");
    }
    ~shoot()
    {
        if (window_) {
            SDL_DestroyWindow(window_);
        }
        SDL_Quit();
    }

private:
   void timer_callback()
    {
        // 处理鼠标事件
        handle_mouse_events();
        
        auto msg = my_msgs::msg::SendData();
        
        // 设置云台角度
        msg.pitch = pitch_;
        msg.yaw = yaw_;
        msg.if_shoot = is_shooting_; // 默认不射击
        
        // 发布消息
        publisher_->publish(msg);
        
        RCLCPP_DEBUG(this->get_logger(), "发布角度: pitch=%.2f, yaw=%.2f", 
                    msg.pitch, msg.yaw);
    }
    
    void handle_mouse_events()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_MOUSEMOTION:
                    // 根据鼠标移动调整云台角度
                    // X轴移动影响偏航角(Yaw)
                    yaw_ += event.motion.xrel * 0.2; // 灵敏度系数可调
                    
                    // Y轴移动影响俯仰角(Pitch)
                    pitch_ -= event.motion.yrel * 0.2; // 灵敏度系数可调
                    
                    // 限制俯仰角范围在-60到60度之间
                    if (pitch_ > 60.0) pitch_ = 60.0;
                    if (pitch_ < -17.0) pitch_ = -17.0;
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    // 鼠标左键按下开始射击
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        is_shooting_ = true;
                        RCLCPP_INFO(this->get_logger(), "开始射击");
                    }
                    break;
                    
                // case SDL_MOUSEBUTTONUP:
                //     // 鼠标左键松开停止射击
                //     if (event.button.button == SDL_BUTTON_LEFT) {
                //         is_shooting_ = false;
                //         RCLCPP_INFO(this->get_logger(), "停止射击");
                //     }
                //     break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        RCLCPP_INFO(this->get_logger(), "ESC键按下，退出程序");
                        rclcpp::shutdown();
                    }
                    break;
                    
                case SDL_QUIT:
                    rclcpp::shutdown();
                    break;
            }
        }
    }
    
    rclcpp::Publisher<my_msgs::msg::SendData>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    int player_id=0;
    
    // 云台角度
    double pitch_;  // 俯仰角 (-60.0 到 60.0)
    double yaw_;    // 偏航角 (无限制)
    bool is_shooting_=true;
    
    // SDL相关
    SDL_Window* window_;
    int mouse_x_, mouse_y_;
    bool is_mouse_captured_;
};

int main(int argc, char * argv[]){
    setenv("ROS_DOMAIN_ID", "0", 1);
    rclcpp::init(argc, argv);
    int player_id = 1;
    if (argc > 1) {
        try {
            player_id = std::stoi(argv[1]);
            if (player_id != 1 && player_id != 2) {
                std::cerr << "玩家ID必须是1或2" << std::endl;
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "玩家ID必须是数字" << std::endl;
            return 1;
        }
    }
    auto node = std::make_shared<shoot>(player_id);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}



