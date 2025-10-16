#include "yolo.hpp"
#include <iostream>

using namespace cv;
using namespace std;
using namespace dnn;

RobDetector::RobDetector(float confThreshold) {
    this->confThreshold = confThreshold;
    this->input_size = Size(640, 640);
}

bool RobDetector::loadModel(const string& modelPath) {
    net = readNetFromONNX(modelPath);
    if (net.empty()) {
        cerr << "无法加载模型: " << modelPath << endl;
        return false;
    }
    net.setPreferableBackend(DNN_BACKEND_OPENCV);
    net.setPreferableTarget(DNN_TARGET_CPU);
    return true;
}

vector<Detection> RobDetector::detect(Mat& frame) {
    vector<Detection> detections;
    if (net.empty()) return detections;
    
    Size original_size = frame.size();
    Mat blob;
    Mat resized;
    resize(frame, resized, input_size);
    blobFromImage(resized, blob, 1.0/255.0, Size(), Scalar(0, 0, 0), true, false);
    net.setInput(blob);
    
    vector<Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());
    
    if (!outputs.empty()) {
        postprocessYOLOv8(outputs[0], original_size, detections);
    }
    
    return detections;
}

void RobDetector::postprocessYOLOv8(Mat& output, Size original_size, vector<Detection>& detections) {
    int rows = output.size[2];
    int dimensions = output.size[1];
    
    Mat output_transposed;
    transpose(output.reshape(1, dimensions), output_transposed);
    
    vector<float> confidences;
    vector<Rect> boxes;
    
    for (int i = 0; i < rows; i++) {
        Mat row = output_transposed.row(i);
        float* data = row.ptr<float>();
        float cx = data[0], cy = data[1], w = data[2], h = data[3], confidence;
        
        if (dimensions == 5) {
            confidence = data[4];
        } else {
            float max_score = 0;
            for (int j = 4; j < dimensions; j++) {
                if (data[j] > max_score) {
                    max_score = data[j];
                }
            }
            confidence = max_score;
        }
        
        if (confidence > confThreshold) {
            confidences.push_back(confidence);
            float left = (cx - w/2) / 640 * original_size.width;
            float top = (cy - h/2) / 640 * original_size.height;
            float width = w * original_size.width / 640;
            float height = h * original_size.height / 640;
            
            boxes.push_back(Rect(left, top, width, height));
        }
    }
    
    vector<int> indices;
    NMSBoxes(boxes, confidences, confThreshold, 0.45, indices);
    
    for (int idx : indices) {
        Detection det;
        det.confidence = confidences[idx];
        det.box = boxes[idx];
        detections.push_back(det);
    }
}