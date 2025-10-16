#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <std_msgs/msg/float32_multi_array.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>

#define is_show 1

using namespace std;
using namespace rclcpp;
using namespace std::chrono_literals;

class core : public rclcpp::Node {
public:
    core() : Node("core_node") {
        // 创建订阅者
        detection_result_sub_ = this->create_subscription<std_msgs::msg::Float32MultiArray>(
            "detection_result", 10,
            std::bind(&core::detection_result_callback, this, std::placeholders::_1));
        
        // 订阅image
        // image_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
        //     "unity_camera_image", 10,
        //     std::bind(&core::image_callback, this, std::placeholders::_1));

        if(is_show){
            image_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/camera_image_player_1", 10,
            std::bind(&core::image_callback, this, std::placeholders::_1));
        }
            

        process_timer_ = this->create_wall_timer(
            100ms, std::bind(&core::process_data, this));
        last_detection_time_ = this->now();
        detection_count_ = 0;

        if(is_show)
            cv::namedWindow("Target Detection Display", cv::WINDOW_NORMAL);
        
        RCLCPP_INFO(this->get_logger(), "目标检测订阅节点已启动");
    }
    
    ~core() {
        if(is_show)
            cv::destroyWindow("Target Detection Display");
    }

private:
    
    void detection_result_callback(const std_msgs::msg::Float32MultiArray::SharedPtr msg) {
        if (msg->data.size() >= 11) {
            // 解析完整检测结果
            detection_result_.pixel_x = msg->data[0];
            detection_result_.pixel_y = msg->data[1];
            detection_result_.world_x = msg->data[2];
            detection_result_.world_y = msg->data[3];
            detection_result_.world_z = msg->data[4];
            detection_result_.distance = msg->data[5];
            detection_result_.confidence = msg->data[6];
            detection_result_.bbox_x = msg->data[7];
            detection_result_.bbox_y = msg->data[8];
            detection_result_.bbox_width = msg->data[9];
            detection_result_.bbox_height = msg->data[10];
            
            // 解析旋转向量（如果存在）
            if (msg->data.size() > 14) {
                detection_result_.rvec_x = msg->data[11];
                detection_result_.rvec_y = msg->data[12];
                detection_result_.rvec_z = msg->data[13];
                detection_result_.tvec_x = msg->data[14];
                detection_result_.tvec_y = msg->data[15];
                detection_result_.tvec_z = msg->data[16];
            }
            
            last_detection_time_ = this->now();
            detection_count_++;
            
            // 保留关键信息日志，但减少频率
            if (detection_count_ % 10 == 0) {
                RCLCPP_INFO(this->get_logger(), 
                    "检测到目标 #%d - 像素: (%.1f, %.1f), 世界: (%.2f, %.2f, %.2f), 距离: %.2f, 置信度: %.2f",
                    detection_count_, 
                    detection_result_.pixel_x, detection_result_.pixel_y,
                    detection_result_.world_x, detection_result_.world_y, detection_result_.world_z,
                    detection_result_.distance, detection_result_.confidence);
            }
        }
    }
    
    // 注释掉图像回调函数
    void image_callback(const sensor_msgs::msg::Image::SharedPtr msg) {
        try {
            cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, "bgr8");
            current_image_ = cv_ptr->image;
            has_image_ = true;
        } catch (const cv_bridge::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "cv_bridge异常: %s", e.what());
        }
    }
    
    void process_data() {
        // 检查检测是否超时
        auto current_time = this->now();
        auto time_since_last_detection = current_time - last_detection_time_;
        
        if (time_since_last_detection.seconds() > 1.0) {
            if (!detection_timeout_reported_) {
                RCLCPP_WARN(this->get_logger(), "检测数据超时，最后接收时间: %.1f秒前", 
                           time_since_last_detection.seconds());
                detection_timeout_reported_ = true;
            }
        } else {
            detection_timeout_reported_ = false;
            
            // 处理检测数据
            process_detection_data();
        }
        
        // 注释掉图像显示
        if(is_show)
            if (has_image_) {
                display_detection_result();
            }
    }
    
    void process_detection_data() {
        // 添加业务逻辑


        if (detection_result_.distance > 0) {
            if (detection_result_.distance < 200.0f) {
                // 目标很近，可能需要后退
                RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                                   "目标过近 (%.1f)，建议后退", detection_result_.distance);
            } else if (detection_result_.distance > 800.0f) {
                // 目标很远，可能需要前进
                RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                                   "目标过远 (%.1f)，建议前进", detection_result_.distance);
            }
        }
        
        // 示例：记录数据到文件
        if (detection_count_ % 50 == 0) {
            RCLCPP_DEBUG(this->get_logger(), 
                "目标统计 - 检测次数: %d", detection_count_);
        }
    }
    
    void display_detection_result() {
        cv::Mat display_image = current_image_.clone();
        
        //绘制边界框
        if (detection_result_.bbox_width > 0 && detection_result_.bbox_height > 0) {
            cv::Rect bbox(detection_result_.bbox_x, detection_result_.bbox_y,
                         detection_result_.bbox_width, detection_result_.bbox_height);
            cv::rectangle(display_image, bbox, cv::Scalar(0, 255, 0), 1);
            
            // 绘制标签
            std::string label = cv::format("Rob: %.2f", detection_result_.confidence);
            cv::putText(display_image, label, 
                       cv::Point(bbox.x, bbox.y - 10), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
        }
        
        // 绘制中心点
        if (detection_result_.pixel_x > 0 && detection_result_.pixel_y > 0) {
            cv::Point center(detection_result_.pixel_x, detection_result_.pixel_y);
            cv::circle(display_image, center, 1, cv::Scalar(0, 0, 255), -1);
            cv::circle(display_image, center, 5, cv::Scalar(0, 0, 255), 2);
        }
        
        // 显示距离信息
        if (detection_result_.distance > 0) {
            std::string distance_info = cv::format("Distance: %.2f", detection_result_.distance);
            cv::putText(display_image, distance_info, 
                       cv::Point(10, display_image.rows - 20), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
            
            std::string world_info = cv::format("World: (%.1f, %.1f, %.1f)", 
                                              detection_result_.world_x,
                                              detection_result_.world_y,
                                              detection_result_.world_z);
            cv::putText(display_image, world_info, 
                       cv::Point(10, display_image.rows - 50), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 0), 2);
        }
        
        // 显示检测计数
        std::string count_info = cv::format("Detections: %d", detection_count_);
        cv::putText(display_image, count_info, 
                   cv::Point(10, 30), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);
        
        // 显示状态
        auto current_time = this->now();
        auto time_since_last = current_time - last_detection_time_;
        std::string status = (time_since_last.seconds() < 0.5) ? "ACTIVE" : "INACTIVE";
        cv::Scalar status_color = (time_since_last.seconds() < 0.5) ? 
                                 cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
        
        cv::putText(display_image, "Status: " + status, 
                   cv::Point(10, 60), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, status_color, 2);
        
        cv::imshow("Target Detection Display", display_image);
        cv::waitKey(1);
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
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;
    
    rclcpp::TimerBase::SharedPtr process_timer_;
    
    DetectionResult detection_result_;
    cv::Mat current_image_;
    
    // 状态变量
    rclcpp::Time last_detection_time_;
    int detection_count_;
    bool has_image_ = false;
    bool detection_timeout_reported_ = false;
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    
    auto node = std::make_shared<core>();
    rclcpp::spin(node);
    if(is_show)
        cv::destroyAllWindows();
    rclcpp::shutdown();
    return 0;
}