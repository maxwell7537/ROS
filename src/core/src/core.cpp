#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <std_msgs/msg/float32_multi_array.hpp>
#include <std_msgs/msg/int32_multi_array.hpp>
#include <tdt_interface/msg/receive_data.hpp>
#include <std_msgs/msg/int32.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <queue>


using namespace std;
using namespace rclcpp;
using namespace std::chrono_literals;

class core : public rclcpp::Node {
public:
    core() : Node("core_node") {
        player_id_=1;
        linear_velocity_ = 100.0; // 线速度 (m/s)
        point_set[0]=cv::Point2f(-16.00,-45.00);//0 补给区(起点)
        point_set[1]=cv::Point2f(7.00,-45.00);//1 补给区前方
        point_set[2]=cv::Point2f(5.50,-37.50);//2 基地右前方
        point_set[3]=cv::Point2f(-26.50,-37.50);//3 5号敌人前方
        point_set[4]=cv::Point2f(-26.50,-6.00);//4 4号敌人前方
        point_set[5]=cv::Point2f(-5.50,-3.00);//5 敌方前哨战右前方
        point_set[6]=cv::Point2f(0.00,0.00);//6 地图中心
        point_set[7]=cv::Point2f(0.00,35.00);//7 地方基地前方
        point_set[8]=cv::Point2f(7.12,-32.84);//8 隧道入口
        point_set[9]=cv::Point2f(22.30,-16.27);//9 隧道中点
        point_set[10]=cv::Point2f(22.55,6.00);//10 隧道终点
        point_set[11]=cv::Point2f(-27.60,-20.67);//11 通往3号敌人的拐点
        point_set[12]=cv::Point2f(-20.46,-20.86);//11 3号敌人
        map_[0].push_back(1);  map_[1].push_back(0);
        map_[1].push_back(2);  map_[2].push_back(1);
        map_[2].push_back(3);  map_[3].push_back(2);
        map_[2].push_back(5);  map_[5].push_back(2);
        map_[2].push_back(6);  map_[6].push_back(2);
        map_[2].push_back(8);  //单向
        map_[3].push_back(4);  map_[4].push_back(3);
        map_[3].push_back(11);  map_[11].push_back(3);
        map_[4].push_back(5);  map_[5].push_back(4);
        map_[4].push_back(6);  map_[6].push_back(4);
        map_[4].push_back(11);  map_[11].push_back(4);
        map_[5].push_back(6);  map_[6].push_back(5);
        map_[6].push_back(7);  map_[7].push_back(6);
        map_[8].push_back(9);  //单向
        map_[9].push_back(10); //单向
        map_[10].push_back(6);  map_[6].push_back(10); 
        map_[11].push_back(12);  map_[12].push_back(11);
        
        
        eps=0.50;
        std::memset(blue_healths_, 0, sizeof(blue_healths_));
        std::memset(red_healths_, 0, sizeof(red_healths_));

        detection_result_sub_ = this->create_subscription<std_msgs::msg::Float32MultiArray>(
            "detection_result", 10,
            std::bind(&core::detection_result_callback, this, std::placeholders::_1));
        

        std::string topic_name = "control_cmd_player_" + std::to_string(player_id_);
        cmd_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(topic_name, 10);

        string position_topic = "position_player_" + to_string(player_id_);
        position_sub_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
            position_topic, 10,
            std::bind(&core::position_callback, this, std::placeholders::_1));
            
        string angles_topic = "real_angles_player_" + to_string(player_id_);
        angles_sub_ = this->create_subscription<tdt_interface::msg::ReceiveData>(
            angles_topic, 10,
            std::bind(&core::angles_callback, this, std::placeholders::_1));
        healths_sub_ = this->create_subscription<std_msgs::msg::Int32MultiArray>(
                "game_healths", 10,
                std::bind(&core::healths_callback, this, std::placeholders::_1));

        time_sub_ = this->create_subscription<std_msgs::msg::Int32>(
                "game_time", 10,
                std::bind(&core::time_callback, this, std::placeholders::_1));

        timer_ = this->create_wall_timer(
            100ms, std::bind(&core::publishCommand, this));
        last_detection_time_ = this->now();
        last_display_time_ = this->now();
        detection_count_ = 0;
        
        RCLCPP_INFO(this->get_logger(), "目标检测订阅节点已启动");
    }
    

private:

    void detection_result_callback(const std_msgs::msg::Float32MultiArray::SharedPtr msg) {
        if (msg->data.size() >= 12) {
            // 解析完整检测结果
            detection_result_.pixel_x = msg->data[0];
            detection_result_.pixel_y = msg->data[1];
            detection_result_.world_x = msg->data[2];
            detection_result_.world_y = msg->data[3];
            detection_result_.world_z = msg->data[4];
            detection_result_.distance = msg->data[5];
            detection_result_.confidence = msg->data[6];
            detection_result_.class_id = (int)msg->data[7];
            detection_result_.bbox_x = msg->data[8];
            detection_result_.bbox_y = msg->data[9];
            detection_result_.bbox_width = msg->data[10];
            detection_result_.bbox_height = msg->data[11];
            
            // 解析旋转向量
            if (msg->data.size() > 15) {
                detection_result_.rvec_x = msg->data[12];
                detection_result_.rvec_y = msg->data[13];
                detection_result_.rvec_z = msg->data[14];
                detection_result_.tvec_x = msg->data[15];
                detection_result_.tvec_y = msg->data[16];
                detection_result_.tvec_z = msg->data[17];
            }
            
            last_detection_time_ = this->now();
            detection_count_++;
            if (detection_count_ % 60 == 0) {
                RCLCPP_INFO(this->get_logger(), 
                    "检测到目标 #%d - 像素: (%.1f, %.1f), 世界: (%.2f, %.2f, %.2f), 距离: %.2f, 置信度: %.2f",
                    detection_count_, 
                    detection_result_.pixel_x, detection_result_.pixel_y,
                    detection_result_.world_x, detection_result_.world_y, detection_result_.world_z,
                    detection_result_.distance, detection_result_.confidence);
            }
        }
    }
 
    void position_callback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
        current_x_ = msg->pose.position.x;
        current_y_ = msg->pose.position.y;
    } 
    void time_callback(const std_msgs::msg::Int32::SharedPtr msg) {
        time_ = msg->data;
        
    } 
    void healths_callback(const std_msgs::msg::Int32MultiArray::SharedPtr msg) {
    for(int i=0;i<6;i++){
            blue_healths_[i]=msg->data[i];
        }
    for(int i=0;i<6;i++){
            red_healths_[i]=msg->data[i+6];
        }
    } 
    void angles_callback(const tdt_interface::msg::ReceiveData::SharedPtr msg) {
        yaw=msg->yaw;
        pitch=msg->pitch;
        sec=msg->timestamp.sec;
        nanosec=msg->timestamp.nanosec;
    }
    void calculation_path(){
        //帮我完善决策代码框架
        return;
    }
    void publishCommand() {
        auto msg = geometry_msgs::msg::Twist();
        calculation_path();
        // 检查是否还有路径点
        if (move_list.empty()) {
            RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                                "所有路径点已完成，停止移动");
            cmd_pub_->publish(msg);
            return;
        }

        cv::Point2f target_point = move_list.front();
        double dx = target_point.x - current_x_;
        double dy = target_point.y - current_y_;
        double distance = sqrt(dx*dx + dy*dy);
        if (distance < eps) {
            move_list.pop();
            point_count--;
            
            if (!move_list.empty()) {
                cv::Point2f next_point = move_list.front();
                RCLCPP_INFO(this->get_logger(), "到达路径点，剩余 %zu 个点，下一个目标: (%.2f, %.2f)", 
                        move_list.size(), next_point.x, next_point.y);
            } else {
                RCLCPP_INFO(this->get_logger(), "所有路径点已完成!");
            }
            cmd_pub_->publish(msg);
            return;
        }
        // 归一化方向向量
        double direction_x = dx / distance;
        double direction_y = dy / distance;
        
        msg.linear.x = direction_x *linear_velocity_;  // X方向速度
        msg.linear.y = direction_y *linear_velocity_;  // Y方向速度
        msg.linear.z = 0.0;
        msg.angular.x = 0.0;
        msg.angular.y = 0.0;
        msg.angular.z = 0.0;
        cmd_pub_->publish(msg);
        
        static int count=0;
        if (count++%10 == 0) {
            RCLCPP_INFO(this->get_logger(), "x:%.2lf,y:%.2lf",current_x_,current_y_);
        }
    }

    

    
    // 检测结果结构体
    struct DetectionResult {
        float pixel_x = 0.0f;
        float pixel_y = 0.0f;
        float world_x = 0.0f;
        float world_y = 0.0f;
        float world_z = 0.0f;
        float distance = 0.0f;
        float confidence = 0.0f;
        int class_id = 0;
        float bbox_x = 0.0f;
        float bbox_y = 0.0f;
        float bbox_width = 0.0f;
        float bbox_height = 0.0f;
        float rvec_x = 0.0f;
        float rvec_y = 0.0f;
        float rvec_z = 0.0f;
        float tvec_x = 0.0f;
        float tvec_y = 0.0f;
        float tvec_z = 0.0f;
    };

private:
    rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr detection_result_sub_;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr position_sub_;
    rclcpp::Subscription<tdt_interface::msg::ReceiveData>::SharedPtr angles_sub_;
    rclcpp::Subscription<std_msgs::msg::Int32MultiArray>::SharedPtr healths_sub_;
    rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr time_sub_;
    rclcpp::TimerBase::SharedPtr timer_;
    
    DetectionResult detection_result_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
    // 状态变量
    rclcpp::Time last_detection_time_;
    int detection_count_;
    bool detection_timeout_reported_ = false;
    rclcpp::Time last_display_time_;
    int player_id_;
    double linear_velocity_;
    double angular_velocity_;
    double current_x_, current_y_;
    queue<cv::Point2f> move_list;
    double eps;
    int point_count;
    cv::Point2f point_set[20];
    vector<int> map_[20];
    int time_;
    int blue_healths_[6];  // 0:玩家, 1:3号, 2:4号, 3:5号, 4:前哨站, 5:基地
    int red_healths_[6];   // 同上
    float yaw;
    float pitch;
    int32_t sec;
    uint32_t nanosec;
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<core>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}