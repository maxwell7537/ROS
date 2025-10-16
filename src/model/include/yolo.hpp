#ifndef YOLO_HPP
#define YOLO_HPP

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <string>

struct Detection {
    cv::Rect box;
    float confidence;
};

class RobDetector {
private:
    cv::dnn::Net net;
    float confThreshold;
    cv::Size input_size;
    
    void postprocessYOLOv8(cv::Mat& output, cv::Size original_size, std::vector<Detection>& detections);
    
public:
    RobDetector(float confThreshold = 0.25);
    bool loadModel(const std::string& modelPath);
    std::vector<Detection> detect(cv::Mat& frame);
};

#endif