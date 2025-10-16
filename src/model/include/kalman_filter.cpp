#include "kalman_filter.hpp"
#include <iostream>
using namespace std;
using namespace cv;
namespace kf{
    kalmanfilter::kalmanfilter():stateSize(0),measSize(0),controlSize(0){}
void kalmanfilter::init(int _stateSize,int _measSize,int _controlSize){
    stateSize=_stateSize;
    measSize=_measSize;
    controlSize=_controlSize;
    state=Mat::zeros(stateSize,1,CV_32F);
    measurement=Mat::zeros(measSize,1,CV_32F);
    transitionMatrix=Mat::eye(stateSize,stateSize,CV_32F);
    measurementMatrix=Mat::zeros(measSize,stateSize,CV_32F);
    if(controlSize>0)
        controlMatrix=Mat::zeros(stateSize,controlSize,CV_32F);
    processNoiseCov=Mat::eye(stateSize,stateSize,CV_32F)*1e-2;
    measurementNoiseCov=Mat::eye(measSize,measSize,CV_32F)*1e-1;
    errorCov=Mat::eye(stateSize,stateSize,CV_32F);
    gain=Mat::zeros(stateSize,measSize,CV_32F);
}
const Mat& kalmanfilter::predict(const Mat& control) {
    // 状态预测: x'=F*x+B*u
    state=transitionMatrix*state;
    if(controlSize>0&&!control.empty())
        state+=controlMatrix*control;
    // 误差协方差预测: P'=F*P*F^T+Q
    errorCov=transitionMatrix*errorCov*transitionMatrix.t()+processNoiseCov;
    return state;
}
const Mat& kalmanfilter::correct(const Mat& _measurement) {
    measurement=_measurement;
    // 计算卡尔曼增益: K=P*H^T*(H*P*H^T+R)^-1
    Mat temp=measurementMatrix*errorCov*measurementMatrix.t()+measurementNoiseCov;
    gain=errorCov*measurementMatrix.t()*temp.inv();
    // 状态更新: x=x'+K*(z-H*x')
    Mat innovation=measurement-measurementMatrix*state;
    state=state+gain*innovation;
    // 误差协方差更新:P=(I-K*H)*P'
    Mat I=Mat::eye(stateSize,stateSize,CV_32F);
    errorCov=(I-gain*measurementMatrix)*errorCov;
    return state;
}
void kalmanfilter::setState(const Mat& _state){
    if(_state.rows==stateSize&&_state.cols==1)
        _state.copyTo(state);
    else
        cerr<<"State size Error"<<endl;
}
void kalmanfilter::setTransitionMatrix(const Mat& _transitionMatrix) {
    if (_transitionMatrix.rows == stateSize && _transitionMatrix.cols == stateSize) {
        _transitionMatrix.copyTo(transitionMatrix);
    } else {
        cerr << "Error: Transition matrix size mismatch!" << endl;
    }
}
void kalmanfilter::setMeasurementMatrix(const Mat& _measurementMatrix) {
    if (_measurementMatrix.rows == measSize && _measurementMatrix.cols == stateSize) {
        _measurementMatrix.copyTo(measurementMatrix);
    } else {
        cerr << "Error: Measurement matrix size mismatch!" << endl;
    }
}
void kalmanfilter::setProcessNoiseCov(const Mat& _processNoiseCov) {
    if (_processNoiseCov.rows == stateSize && _processNoiseCov.cols == stateSize) {
        _processNoiseCov.copyTo(processNoiseCov);
    } else {
        cerr << "Error: Process noise covariance size mismatch!" << endl;
    }
}
void kalmanfilter::setMeasurementNoiseCov(const Mat& _measurementNoiseCov) {
    if (_measurementNoiseCov.rows == measSize && _measurementNoiseCov.cols == measSize) {
        _measurementNoiseCov.copyTo(measurementNoiseCov);
    } else {
        cerr << "Error: Measurement noise covariance size mismatch!" << endl;
    }
}
void kalmanfilter::setErrorCov(const Mat& _errorCov) {
    if (_errorCov.rows == stateSize && _errorCov.cols == stateSize) {
        _errorCov.copyTo(errorCov);
    } else {
        cerr << "Error: Error covariance size mismatch!" << endl;
    }
}
void kalmanfilter::setControlMatrix(const Mat& _controlMatrix) {
    if (_controlMatrix.rows == stateSize && _controlMatrix.cols == controlSize) {
        _controlMatrix.copyTo(controlMatrix);
    } else {
        cerr << "Error: Control matrix size mismatch!" << endl;
    }
}
const Mat& kalmanfilter::getState() const {
    return state;
}
const Mat& kalmanfilter::getErrorCov() const {
    return errorCov;
}
}