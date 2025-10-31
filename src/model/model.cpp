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

double armor_width = 135.0/200;  // 装甲板宽度
double armor_height = 125.0/200; // 装甲板高度

vector<Point3f> objectPoints = {
    Point3f(armor_width/2, armor_height/2, 0),   
    Point3f(armor_width/2, 0, 0),      
    Point3f(armor_width/2, -armor_height/2, 0),  
    Point3f(-armor_width/2, armor_height/2, 0),  
    Point3f(-armor_width/2, 0, 0),     
    Point3f(-armor_width/2, -armor_height/2, 0)  
};
vector<Point2f> imagePoints;

// 内参矩阵
Mat cameraMatrix = (Mat_<double>(3,3) << 
    554.26, 0, 320,
    0, 554.26, 240,
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
    Point2f pixel_position{0, 0};
    Point3f world_position{0, 0, 0};
    float distance = -1.0f;
    vector<float> rvec = {0.0f, 0.0f, 0.0f};
    vector<float> tvec = {0.0f, 0.0f, 0.0f};
    float confidence = 0.0f;
    Rect bounding_box{0, 0, 0, 0};
    int class_id = 0;  //0 表示未检测到

    DetectionResult() = default;
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

        init_kalman_filter();
        // 创建订阅者
        image_subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
            "camera_image_player_1", 10,
            std::bind(&ModelNode::image_callback, this, std::placeholders::_1));
        
        // 创建发布者
        detection_result_publisher_ = this->create_publisher<std_msgs::msg::Float32MultiArray>(
            "detection_result", 10);
            
        // 轨迹存储
        max_trajectory_length_ = 50;

         // 预测参数
        prediction_time_ = 0.01;  // 预测100ms后的位置
        kf_initialized_ = false;
        consecutive_misses_ = 0;
        max_consecutive_misses_ = 5;
        target_priority_ = 1;  // 优先检测1号目标
        
        RCLCPP_INFO(this->get_logger(), "模型节点已启动，卡尔曼滤波已初始化，等待图像数据...");
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
            
            
            //灯条二值化和原图
            //detector_light_.debugShowBinary(frame);

            // 显示图像
            // display_debug_image(frame, detection_result);
            
        } catch (const cv_bridge::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "cv_bridge异常: %s", e.what());
        } catch (const std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "处理图像时发生异常: %s", e.what());
        }
    }
    
    void init_kalman_filter() {
        // 状态向量: [x, y, z, vx, vy, vz] 6维
        // 观测向量: [x, y, z] 3维
        int stateSize = 6;  // 状态维度 (位置 + 速度)
        int measSize = 3;   // 观测维度 (位置)
        int controlSize = 0; // 控制维度
        
        kf_.init(stateSize, measSize, controlSize);
        
        // 设置状态转移矩阵 (匀速模型)
        kf_.setTransitionMatrix((Mat_<float>(6, 6) << 
            1, 0, 0, 1, 0, 0,
            0, 1, 0, 0, 1, 0,
            0, 0, 1, 0, 0, 1,
            0, 0, 0, 1, 0, 0,
            0, 0, 0, 0, 1, 0,
            0, 0, 0, 0, 0, 1));
        
        // 设置测量矩阵
        kf_.setMeasurementMatrix((Mat_<float>(3, 6) << 
            1, 0, 0, 0, 0, 0,
            0, 1, 0, 0, 0, 0,
            0, 0, 1, 0, 0, 0));
        
        kf_.setProcessNoiseCov(Mat::eye(6, 6, CV_32F) * 1e-2);
        kf_.setMeasurementNoiseCov(Mat::eye(3, 3, CV_32F) * 1e-1);
        kf_.setErrorCov(Mat::eye(6, 6, CV_32F) * 1.0);
            
        RCLCPP_INFO(this->get_logger(), "卡尔曼滤波器初始化完成");
    }

    DetectionResult process_frame(Mat& frame) {
        DetectionResult result;
        
        auto detections = detector_rob_.detect(frame);
        
        if(!detections.empty()) {
            Detection selected_detection;
             bool found_priority_target = false;
             for (const auto& det : detections) {
                if (det.class_id == target_priority_) {
                    selected_detection = det;
                    found_priority_target = true;
                    break;
                }
            }
            
            // 如果没有找到优先级目标，选择置信度最高的目标
            if (!found_priority_target) {
                float max_confidence = 0.0f;
                for (const auto& det : detections) {
                    if (det.confidence > max_confidence) {
                        max_confidence = det.confidence;
                        selected_detection = det;
                    }
                }
            }
            // 使用检测到的第一个机器人的中心
            Rect rob_box = selected_detection.box;
            Point2f current_center = Point2f(rob_box.x + rob_box.width/2, 
                                           rob_box.y + rob_box.height/2);
            
            result.pixel_position = current_center;
            result.confidence = selected_detection.confidence;
            result.bounding_box = rob_box;
            result.class_id = selected_detection.class_id;
            
            // 灯条检测和姿态估计
            Rect roi(rob_box.x-100, rob_box.y-100, 
                    rob_box.width+200, rob_box.height+200);
            
            if(roi.x >= 0 && roi.y >= 0 && 
               roi.x + roi.width <= frame.cols && 
               roi.y + roi.height <= frame.rows) {
                
                Mat small_mat = frame(roi);
                vector<LightBar> lightBars = detector_light_.detect(small_mat);
                // detector_light_.drawResults(frame, lightBars);
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
                        //显示检测到的等条中心点
                        circle(frame, Point(lb.center.x + roi.x, lb.center.y + roi.y), 5, Scalar(0, 255, 0), -1);
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


                                update_kalman_filter(result.world_position);
                
                                Point3f predicted_position = predict_future_position(prediction_time_);
                                Point2f predicted_pixel = world_to_pixel(predicted_position, frame.cols, frame.rows);
                                
                                // 用预测结果替代检测结果
                                // result.world_position = predicted_position;
                                // result.pixel_position = predicted_pixel;
                                // result.distance = norm(Mat(predicted_position));

                                // RCLCPP_INFO(this->get_logger(), 
                                //     "检测到目标 - 像素位置: (%.1f, %.1f), 世界位置: (%.2f, %.2f, %.2f), 距离: %.2f", 
                                //     result.pixel_position.x, result.pixel_position.y,
                                //     result.world_position.x, result.world_position.y, result.world_position.z,
                                //     result.distance);
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
            (float)result.class_id,       // 7: 类别 
            (float)result.bounding_box.x, // 8: 边界框x
            (float)result.bounding_box.y, // 9: 边界框y
            (float)result.bounding_box.width,  // 10: 边界框宽度
            (float)result.bounding_box.height  // 11: 边界框高度
        };
        // 添加旋转和平移向量
        for(auto val : result.rvec) {
            result_msg.data.push_back(val);
        }
        for(auto val : result.tvec) {
            result_msg.data.push_back(val);
        }
        
        detection_result_publisher_->publish(result_msg);
        // RCLCPP_INFO(this->get_logger(),"model_id:%d",result.class_id);
    }
    
    void update_trajectory(const Point2f& point) {
        trajectory_.push_back(point);
        if(trajectory_.size() > max_trajectory_length_) {
            trajectory_.erase(trajectory_.begin());
        }
    }
    void update_kalman_filter(const Point3f& position) {
    if (!kf_initialized_) {
        // 第一次初始化状态
        Mat initial_state = (Mat_<float>(6, 1) << 
            position.x, position.y, position.z, 0, 0, 0);
        kf_.setState(initial_state);
        kf_initialized_ = true;
        consecutive_misses_ = 0;
    } else {
        // 更新测量值
        Mat measurement = (Mat_<float>(3, 1) << 
            position.x, position.y, position.z);
        kf_.correct(measurement);
        consecutive_misses_ = 0;
    }
}
    
    Point3f predict_future_position(float dt) {
        if (!kf_initialized_) {
            return Point3f(0, 0, 0);
        }
        
        // 创建预测用的状态转移矩阵
        Mat F = (Mat_<float>(6, 6) << 
            1, 0, 0, dt, 0, 0,
            0, 1, 0, 0, dt, 0,
            0, 0, 1, 0, 0, dt,
            0, 0, 0, 1, 0, 0,
            0, 0, 0, 0, 1, 0,
            0, 0, 0, 0, 0, 1);
        
        // 预测状态
        Mat predicted_state = F * kf_.getState();
        
        return Point3f(predicted_state.at<float>(0),
                      predicted_state.at<float>(1),
                      predicted_state.at<float>(2));
    }
    
    void predict_without_measurement() {
        if (kf_initialized_) {
            // 只有预测步骤，没有更新
            kf_.predict();
            consecutive_misses_++;
            
            if (consecutive_misses_ > max_consecutive_misses_) {
                kf_initialized_ = false;
            }
        }
    }
    
   Point2f world_to_pixel(const Point3f& world_pos, int img_width, int img_height) {
        vector<Point3f> world_points = {world_pos};
        vector<Point2f> pixel_points;
        
        projectPoints(world_points, rvec, tvec, cameraMatrix, distCoeffs, pixel_points);
        
        if (!pixel_points.empty()) {
            Point2f pixel = pixel_points[0];
            // 宽松的检查：允许像素坐标在图像边界外一定范围内
            if (pixel.x >= -1000 && pixel.x < img_width+1000 && pixel.y >= -1000 && pixel.y < img_height+1000) {
                return pixel;
            }
        }
        // 返回图像中心
        return Point2f(img_width/2, img_height/2);
    }
    void display_debug_image(Mat& frame, const DetectionResult& result) {
        //绘制检测框
        if(!result.bounding_box.empty()) {
            rectangle(frame, result.bounding_box, Scalar(0, 255, 0), 2);
            string label = format("Rob: %.2f", result.confidence);
            putText(frame, label, Point(result.bounding_box.x, result.bounding_box.y - 10), 
                   FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
        }
        
        // 绘制中心点
        circle(frame, result.pixel_position, 5, Scalar(0, 0, 255), -1);
        
        //绘制轨迹
        for(size_t i = 1; i < trajectory_.size(); i++) {
            line(frame, trajectory_[i-1], trajectory_[i], Scalar(255, 0, 0), 2);
        }
        
        //显示距离信息
        if(result.distance > 0) {
            string info = format("Distance: %.2f", result.distance);
            putText(frame, info, Point(10, frame.rows - 20), 
                    FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,255,255), 2);
        }
        
        imshow("Rob Detection", frame);
        waitKey(1);
    }

private:
    // 检测器
    RobDetector detector_rob_{0.3};
    LightBarDetector detector_light_;
    
     // 卡尔曼滤波器
    kf::kalmanfilter kf_;
    bool kf_initialized_;
    int consecutive_misses_;
    int max_consecutive_misses_;
    float prediction_time_;
    int target_priority_;  // 目标优先级

    // ROS2相关
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_subscription_;
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