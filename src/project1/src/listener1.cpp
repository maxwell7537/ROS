#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "my_msgs/msg/msg1.hpp"  
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include "sensor_msgs/image_encodings.hpp"

using std::placeholders::_1;

class listener : public rclcpp::Node{
public:
    listener(): Node("listener"), window_created_(false) {
        subscription_ = this->create_subscription<my_msgs::msg::Msg1>(
            "topic", 10, std::bind(&listener::topic_callback, this, _1));
        RCLCPP_INFO(this->get_logger(), "订阅者节点已启动");
    }
    ~listener() {
        if (window_created_) {
            cv::destroyWindow("Received Image");
        }
    }

private:
    void topic_callback(const my_msgs::msg::Msg1 & msg) {
        RCLCPP_INFO(this->get_logger(), "Linked OpenCV version: %s", CV_VERSION);
        RCLCPP_INFO(this->get_logger(), "收到消息: '%s', 序号: %d", 
                   msg.s.c_str(), msg.num);
        RCLCPP_INFO(this->get_logger(), "图像信息 - 宽度: %d, 高度: %d, 步长: %d, 编码: %s", 
                   msg.image.width, msg.image.height, msg.image.step, msg.image.encoding.c_str());
        
        if(msg.image.data.empty()){
            RCLCPP_ERROR(this->get_logger(), "图像数据为空");return;}
        if(msg.image.width<=0||msg.image.height<=0){
            RCLCPP_ERROR(this->get_logger(), "图像尺寸无效: %dx%d", msg.image.width, msg.image.height);return;}
        if(msg.image.step<=0){
            RCLCPP_ERROR(this->get_logger(), "图像步长无效: %d", msg.image.step);return;}
        try {
            cv_bridge::CvImagePtr cv_ptr;
            if(msg.image.encoding.empty()||msg.image.encoding == "bgr8"){
                cv_ptr = cv_bridge::toCvCopy(msg.image, sensor_msgs::image_encodings::BGR8);
            }else{
                cv_ptr = cv_bridge::toCvCopy(msg.image);
            }
            // 检查转换后的图像
            if (cv_ptr->image.empty()){
                RCLCPP_ERROR(this->get_logger(), "转换后的图像为空");return;}
            if (cv_ptr->image.cols<= 0||cv_ptr->image.rows <= 0) {
                RCLCPP_ERROR(this->get_logger(), "转换后的图像尺寸无效: %dx%d", 
                           cv_ptr->image.cols, cv_ptr->image.rows);
                return;
            }
            RCLCPP_INFO(this->get_logger(), "转换成功，图像尺寸: %dx%d, 通道: %d", 
                       cv_ptr->image.cols, cv_ptr->image.rows, cv_ptr->image.channels());
            
            // 延迟创建窗口到第一次收到有效图像时
            if (!window_created_) {
                cv::namedWindow("Received Image", cv::WINDOW_AUTOSIZE);
                window_created_ = true;
                RCLCPP_INFO(this->get_logger(), "创建图像显示窗口");
            }
            // cv::imwrite("debug_image.jpg", cv_ptr->image);
            // 显示图像
            cv::imshow("Received Image", cv_ptr->image);
            cv::waitKey(1);
            
        } catch (cv_bridge::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "cv_bridge 异常: %s", e.what());
        } catch (const cv::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "OpenCV 异常: %s", e.what());
        } catch (const std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "图像处理失败: %s", e.what());
        } catch (...) {
            RCLCPP_ERROR(this->get_logger(), "未知异常");
        }
    }
    
    rclcpp::Subscription<my_msgs::msg::Msg1>::SharedPtr subscription_;
    bool window_created_;
};

int main(int argc, char* argv[]){
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<listener>());
    rclcpp::shutdown();
    return 0;
}