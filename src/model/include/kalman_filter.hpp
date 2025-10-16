#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#include <opencv2/opencv.hpp>

namespace kf {
    class kalmanfilter {
    private:
        int stateSize;              // 状态维度
        int measSize;               // 测量维度
        int controlSize;            // 控制维度
        cv::Mat state;              // 状态向量
        cv::Mat measurement;        // 测量向量
        cv::Mat transitionMatrix;   // 转移矩阵(stateSize x stateSize)
        cv::Mat controlMatrix;      // 控制矩阵(stateSize x controlSize)
        cv::Mat measurementMatrix;  // 测量矩阵(measSize x stateSize)
        cv::Mat processNoiseCov;    // 过程噪声协方差(stateSize x stateSize)
        cv::Mat measurementNoiseCov;// 测量噪声协方差(measSize x measSize)
        cv::Mat errorCov;           // 误差协方差(stateSize x stateSize)
        cv::Mat gain;               // 卡尔曼增益(stateSize x measSize)

    public:
        // 构造函数
        kalmanfilter();
        
        // 初始化滤波器
        void init(int stateSize, int measSize, int controlSize = 0);
        
        // 预测步骤
        const cv::Mat& predict(const cv::Mat& control = cv::Mat());
        
        // 更新步骤
        const cv::Mat& correct(const cv::Mat& measurement);
        
        // 设置初始状态
        void setState(const cv::Mat& state);
        
        // 设置转移矩阵
        void setTransitionMatrix(const cv::Mat& transitionMatrix);
        
        // 设置测量矩阵
        void setMeasurementMatrix(const cv::Mat& measurementMatrix);
        
        // 设置过程噪声协方差
        void setProcessNoiseCov(const cv::Mat& processNoiseCov);
        
        // 设置测量噪声协方差
        void setMeasurementNoiseCov(const cv::Mat& measurementNoiseCov);
        
        // 设置误差协方差
        void setErrorCov(const cv::Mat& errorCov);
        
        // 设置控制矩阵
        void setControlMatrix(const cv::Mat& controlMatrix);
        
        // 获取当前状态
        const cv::Mat& getState() const;
        
        // 获取误差协方差
        const cv::Mat& getErrorCov() const;
    };
 }
#endif