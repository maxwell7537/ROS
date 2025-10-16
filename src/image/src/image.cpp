#include <string>
#include <algorithm>
#include "rclcpp/rclcpp.hpp"
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include "sensor_msgs/image_encodings.hpp"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
using namespace std;

class image:public rclcpp::Node{
    public:
        image():Node("image_node"){
        window_name_ = "T-DT_CampusGame_2025";
        capture_rate_ = 80; 
        // this->declare_parameter<double>("publish_frequency", 2.0);
        publisher_ = this->create_publisher<sensor_msgs::msg::Image>("unity_camera_image", 10);
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(1000 / capture_rate_),
            std::bind(&image::captureAndDisplay, this));
        RCLCPP_INFO(this->get_logger(), "image_node is readly!");
        RCLCPP_INFO(this->get_logger(), "查找窗口名称包含 '%s' 的窗口", window_name_.c_str());
        RCLCPP_INFO(this->get_logger(), "显示频率: %d FPS", capture_rate_);
        RCLCPP_INFO(this->get_logger(), "发布话题: unity_camera_image");
        //cv::namedWindow("Unity Game Viewer", cv::WINDOW_AUTOSIZE);
    }
    private:
        void captureAndDisplay(){  // 定时器回调函数：每12.5ms执行一次
            cv::Mat screenshot = captureUnityWindow();//捕获Unity窗口
            if (!screenshot.empty()){
                //cv::imshow("Unity Game Viewer", screenshot);// 显示图像
                //cv::waitKey(1);  // 必须调用waitKey来刷新显示

                // 发布图像消息
                sensor_msgs::msg::Image::SharedPtr msg = 
                    cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", screenshot).toImageMsg();
                msg->header.stamp = this->now();// 设置时间戳
                msg->header.frame_id = "unity_camera_frame";

                publisher_->publish(*msg);
                RCLCPP_DEBUG(this->get_logger(), "发布图像消息: %dx%d",msg->width, msg->height);
            }
        }
        cv::Mat captureUnityWindow(){// 捕获 Unity 窗口，返回 OpenCV 图像（BGR 格式）

            //连接 X11 服务器（Linux 下的窗口系统），nullptr 表示使用默认 DISPLAY 环境变量（当前显示器）
            Display* display = XOpenDisplay(nullptr);
            if (!display) {
                RCLCPP_ERROR(this->get_logger(), "无法连接到X服务器");
                return cv::Mat();
            }

            // 获取 X11 根窗口（整个显示器的根节点，所有窗口都是它的子窗口）
            Window root = DefaultRootWindow(display);
            // 调用 findWindowByTitle() 递归查找含目标名称的窗口（返回窗口句柄）
            Window unity_window = findWindowByTitle(display, root, window_name_);
            if(unity_window == 0){
                RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                                    "未找到包含 '%s' 的窗口", window_name_.c_str());
                XCloseDisplay(display);
                return cv::Mat();
            }
            // 获取窗口属性

            XWindowAttributes attrs;// 存储窗口属性的结构体
            XGetWindowAttributes(display, unity_window, &attrs);// 读取属性到 attrs

            // 捕获窗口图像：XGetImage() 是 X11 捕获窗口的核心函数
            // 参数：display（X11 连接）、unity_window（目标窗口句柄）、x/y（捕获起始坐标，0,0 表示窗口左上角）
            // width/height（捕获宽高，用窗口实际宽高）、AllPlanes（捕获所有颜色平面）、ZPixmap（图像格式，适合转换为 OpenCV）
            XImage* ximage = XGetImage(display, unity_window, 0, 0, 
                                    attrs.width, attrs.height, AllPlanes, ZPixmap);

            if(!ximage){
                RCLCPP_ERROR(this->get_logger(), "无法捕获窗口图像");
                XCloseDisplay(display);
                return cv::Mat();
            }
            // 将 X11 图像（XImage）转换为 OpenCV 图像（cv::Mat）
            // cv::Mat 构造参数：高（attrs.height）、宽（attrs.width）、类型（CV_8UC4：8位无符号，4通道，对应 X11 的 BGRA 格式）
            // 数据指针（ximage->data：XImage 的像素数据地址，直接复用内存，避免拷贝）
            cv::Mat image(attrs.height, attrs.width, CV_8UC4, ximage->data);

            if(attrs.width == 0 || attrs.height == 0) {
                RCLCPP_WARN(this->get_logger(), "窗口大小为 0,跳过捕获");
                XDestroyImage(ximage);
                XCloseDisplay(display);
                return cv::Mat();
            }
            cv::Mat bgr_image;// 存储转换后的 BGR 图像
            cv::cvtColor(image, bgr_image, cv::COLOR_BGRA2BGR);// COLOR_BGRA2BGR：BGRA → BGR
            XDestroyImage(ximage);// 清理资源
            XCloseDisplay(display);
            return bgr_image;
        }
        Window findWindowByTitle(Display* display, Window window, const std::string& title){
            Window root, parent;
            Window* children;
            unsigned int num_children;

            char* window_name = nullptr;
            XFetchName(display, window, &window_name);// 读取窗口标题到 window_name

            if (window_name) {
                std::string name(window_name);
                XFree(window_name);// 释放 X11 分配的内存
                if (name.find(title) != std::string::npos) {
                    return window;// 找到目标窗口，返回窗口句柄
                }
            }

            // 若标题未找到，检查窗口类名（部分窗口标题可能隐藏，类名更可靠）

            // 分配窗口类提示结构体（存储类名）
            XClassHint* class_hint = XAllocClassHint();
            if(!class_hint)return 0;

            // 读取窗口的类名（res_name：应用名，res_class：窗口类名）
            if (XGetClassHint(display, window, class_hint)){
                // 检查 res_name 是否包含目标标题
                if (class_hint->res_name && std::string(class_hint->res_name).find(title) != std::string::npos) {
                    XFree(class_hint->res_name);// 释放 res_name 内存
                    if (class_hint->res_class) XFree(class_hint->res_class);
                    XFree(class_hint);// 释放结构体
                    return window;
                }
                // 检查 res_class 是否包含目标标题
                if (class_hint->res_class && std::string(class_hint->res_class).find(title) != std::string::npos) {
                    XFree(class_hint->res_name);
                    if (class_hint->res_class) XFree(class_hint->res_class);
                    XFree(class_hint);
                    return window;
                }
                // 未找到，释放类名内存
                XFree(class_hint->res_name);
                if (class_hint->res_class) XFree(class_hint->res_class);
            }
            XFree(class_hint);// 释放结构体（无论是否找到，都要释放）

            // 递归搜索子窗口
            if(XQueryTree(display, window, &root, &parent, &children, &num_children)) {
                for(unsigned int i=0;i<num_children;i++){// 遍历每个子窗口
                    Window result=findWindowByTitle(display,children[i],title);
                    if(result!=0){// 找到目标窗口
                        XFree(children);
                        return result;
                    }
                }
                XFree(children);
            }
            return 0;
        }
        rclcpp::TimerBase::SharedPtr timer_;
        rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;
        std::string window_name_;  // 窗口名称，写死在代码里
        int capture_rate_;
};

int main(int argc,char*argv[]){
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<image>());
    //cv::destroyAllWindows();
    rclcpp::shutdown();
}
