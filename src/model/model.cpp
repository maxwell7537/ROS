#include "./include/yolo.hpp"
#include "./include/light.hpp"
#include "./include/kalman_filter.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "geometry_msgs/msg/point.hpp"
#include "std_msgs/msg/float32_multi_array.hpp"
#include "cv_bridge/cv_bridge.h"

using namespace cv;
using namespace std;
using namespace std::chrono_literals;

vector<Point3f> objectPoints = {
    Point3f(67.5, 62.5, 0),   
    Point3f(67.5, 0, 0),      
    Point3f(67.5, -62.5, 0),  
    Point3f(-67.5, 62.5, 0),  
    Point3f(-67.5, 0, 0),     
    Point3f(-67.5, -62.5, 0)  
};
vector<Point2f> imagePoints;

// 内参矩阵
Mat cameraMatrix = (Mat_<double>(3,3) << 
    1.7774091341308808e+03, 0, 7.1075979428865026e+02,
    0, 1.7754170626354828e+03, 5.3472407285624729e+02,
    0, 0, 1
);
// 畸变系数
Mat distCoeffs = (Mat_<double>(5,1) << 
    0, 0,
    0, 0,
    0
);
Mat rvec;
Mat tvec;

struct node{
    Point2f p;
    int id;
    double dis;
}lights_a[100];

bool cmp(node a,node b){
    return a.dis<b.dis;
}

struct DetectionResult {
    Point2f pixel_position;
    Point3f world_position;
    float distance;
    vector<float> rvec;
    vector<float> tvec;
    float confidence;
    Rect bounding_box;
};

class ModelNode : public rclcpp::Node {
public:
    ModelNode() : Node("model_node") {
        // 初始化YOLO检测器
        string model_path = "/home/tdt/tool/ros/src/model/best.onnx";
        if(!detector_rob_.loadModel(model_path)){
            RCLCPP_ERROR(this->get_logger(), "无法加载模型: %s", model_path.c_str());
            throw std::runtime_error("无法加载YOLO模型");
        }
        
        // 创建订阅者
        image_subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
            "camera_image_player_1", 10,
            std::bind(&ModelNode::image_callback, this, std::placeholders::_1));
        
        // 创建发布者
        detection_result_publisher_ = this->create_publisher<std_msgs::msg::Float32MultiArray>(
            "detection_result", 10);
            
        // 轨迹存储
        max_trajectory_length_ = 50;
        
        RCLCPP_INFO(this->get_logger(), "模型节点已启动，等待图像数据...");
    }

private:
    void image_callback(const sensor_msgs::msg::Image::SharedPtr msg) {
        try {
            // 转换ROS图像消息为OpenCV格式
            cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, "bgr8");
            Mat frame = cv_ptr->image;
            
            // 执行检测
            auto detection_result = process_frame(frame);
            
            // 发布检测结果
            publish_detection_result(detection_result);
            
            // 显示图像
            // display_debug_image(frame, detection_result);
            
        } catch (const cv_bridge::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "cv_bridge异常: %s", e.what());
        } catch (const std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "处理图像时发生异常: %s", e.what());
        }
    }
    
    DetectionResult process_frame(Mat& frame) {
        DetectionResult result;
        
        auto detections = detector_rob_.detect(frame);
        
        if(!detections.empty()) {
            // 使用检测到的第一个机器人的中心
            Rect rob_box = detections[0].box;
            Point2f current_center = Point2f(rob_box.x + rob_box.width/2, 
                                           rob_box.y + rob_box.height/2);
            
            result.pixel_position = current_center;
            result.confidence = detections[0].confidence;
            result.bounding_box = rob_box;
            
            // 灯条检测和姿态估计
            Rect roi(rob_box.x-100, rob_box.y-100, 
                    rob_box.width+200, rob_box.height+200);
            
            if(roi.x >= 0 && roi.y >= 0 && 
               roi.x + roi.width <= frame.cols && 
               roi.y + roi.height <= frame.rows) {
                
                Mat small_mat = frame(roi);
                vector<LightBar> lightBars = detector_light_.detect(small_mat);
                
                if(!lightBars.empty()) {
                    int lightCount = min((int)lightBars.size(), 100);
                    
                    // 计算机器人中心点
                    Point2f robCenter(rob_box.x + rob_box.width/2, 
                                     rob_box.y + rob_box.height/2);
                    
                    for (int i = 0; i < lightCount; i++) {
                        const auto& lb = lightBars[i];
                        lights_a[i].p.x = lb.center.x + roi.x;
                        lights_a[i].p.y = lb.center.y + roi.y;
                        lights_a[i].id = i;
                        lights_a[i].dis = sqrt((lights_a[i].p.x - robCenter.x) * (lights_a[i].p.x - robCenter.x) + 
                                              (lights_a[i].p.y - robCenter.y) * (lights_a[i].p.y - robCenter.y));    
                    }
                    
                    sort(lights_a, lights_a + lightCount, cmp);
                    
                    if(lightCount >= 2) {
                        int point_1 = lights_a[0].id;
                        int point_2 = lights_a[1].id;
                        
                        if(lightBars[point_1].center.y > lightBars[point_2].center.y) {
                            swap(point_1, point_2);
                        }
                        
                        Point2f center[7];
                        center[1] = lightBars[point_1].getTopEndpoint() + Point2f(roi.x, roi.y);
                        center[2] = lightBars[point_1].center + Point2f(roi.x, roi.y);
                        center[3] = lightBars[point_1].getBottomEndpoint() + Point2f(roi.x, roi.y);
                        center[4] = lightBars[point_2].getTopEndpoint() + Point2f(roi.x, roi.y);
                        center[5] = lightBars[point_2].center + Point2f(roi.x, roi.y);
                        center[6] = lightBars[point_2].getBottomEndpoint() + Point2f(roi.x, roi.y);
                        
                        if(center[1].y > center[3].y) swap(center[1], center[3]);
                        if(center[4].y > center[6].y) swap(center[4], center[6]);
                        
                        imagePoints.clear();
                        for(int i = 1; i <= 6; i++) {
                            imagePoints.push_back(center[i]);
                        }
                        
                        if(imagePoints.size() == objectPoints.size()) {
                            bool success = solvePnP(objectPoints, imagePoints, cameraMatrix, distCoeffs, rvec, tvec, false, SOLVEPNP_IPPE);
                            if(success){
                                // 计算距离
                                double distance = norm(tvec);
                                result.distance = distance;
                                result.world_position = Point3f(tvec.at<double>(0), 
                                                              tvec.at<double>(1), 
                                                              tvec.at<double>(2));
                                
                                // 存储旋转和平移向量
                                result.rvec = vector<float>{(float)rvec.at<double>(0), 
                                                          (float)rvec.at<double>(1), 
                                                          (float)rvec.at<double>(2)};
                                result.tvec = vector<float>{(float)tvec.at<double>(0), 
                                                          (float)tvec.at<double>(1), 
                                                          (float)tvec.at<double>(2)};
                                
                                RCLCPP_DEBUG(this->get_logger(), 
                                    "检测到目标 - 像素位置: (%.1f, %.1f), 世界位置: (%.2f, %.2f, %.2f), 距离: %.2f", 
                                    result.pixel_position.x, result.pixel_position.y,
                                    result.world_position.x, result.world_position.y, result.world_position.z,
                                    result.distance);
                            }
                        }
                    }
                }
            }
            
            // 更新轨迹
            update_trajectory(current_center);
        }
        
        return result;
    }
    
    void publish_detection_result(const DetectionResult& result) {
        // 发布完整检测结果
        auto result_msg = std_msgs::msg::Float32MultiArray();
        result_msg.data = {
            result.pixel_position.x,      // 0: 像素x
            result.pixel_position.y,      // 1: 像素y
            result.world_position.x,      // 2: 世界x
            result.world_position.y,      // 3: 世界y
            result.world_position.z,      // 4: 世界z
            result.distance,              // 5: 距离
            result.confidence,            // 6: 置信度
            (float)result.bounding_box.x, // 7: 边界框x
            (float)result.bounding_box.y, // 8: 边界框y
            (float)result.bounding_box.width,  // 9: 边界框宽度
            (float)result.bounding_box.height  // 10: 边界框高度
        };
        // 添加旋转和平移向量
        for(auto val : result.rvec) {
            result_msg.data.push_back(val);
        }
        for(auto val : result.tvec) {
            result_msg.data.push_back(val);
        }
        
        detection_result_publisher_->publish(result_msg);
    }
    
    void update_trajectory(const Point2f& point) {
        trajectory_.push_back(point);
        if(trajectory_.size() > max_trajectory_length_) {
            trajectory_.erase(trajectory_.begin());
        }
    }
    
    // void display_debug_image(Mat& frame, const DetectionResult& result) {
    //     // 绘制检测框
    //     if(!result.bounding_box.empty()) {
    //         rectangle(frame, result.bounding_box, Scalar(0, 255, 0), 2);
    //         string label = format("Rob: %.2f", result.confidence);
    //         putText(frame, label, Point(result.bounding_box.x, result.bounding_box.y - 10), 
    //                FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
    //     }
        
    //     // 绘制中心点
    //     circle(frame, result.pixel_position, 5, Scalar(0, 0, 255), -1);
        
    //     // 绘制轨迹
    //     for(size_t i = 1; i < trajectory_.size(); i++) {
    //         line(frame, trajectory_[i-1], trajectory_[i], Scalar(255, 0, 0), 2);
    //     }
        
    //     // 显示距离信息
    //     if(result.distance > 0) {
    //         string info = format("Distance: %.2f", result.distance);
    //         putText(frame, info, Point(10, frame.rows - 20), 
    //                 FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,255,255), 2);
    //     }
        
    //     imshow("Rob Detection", frame);
    //     waitKey(1);
    // }

private:
    // 检测器
    RobDetector detector_rob_{0.3};
    LightBarDetector detector_light_;
    
    // ROS2相关
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_subscription_;
    rclcpp::Publisher<geometry_msgs::msg::Point>::SharedPtr target_position_publisher_;
    rclcpp::Publisher<geometry_msgs::msg::Point>::SharedPtr target_world_position_publisher_;
    rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr detection_result_publisher_;
    
    // 轨迹跟踪
    vector<Point2f> trajectory_;
    size_t max_trajectory_length_;
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    
    try {
        auto node = std::make_shared<ModelNode>();
        rclcpp::spin(node);
    } catch (const std::exception& e) {
        std::cerr << "节点运行异常: " << e.what() << std::endl;
        return -1;
    }
    
    rclcpp::shutdown();
    return 0;
}