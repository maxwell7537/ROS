#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <std_msgs/msg/float32_multi_array.hpp>
#include <std_msgs/msg/int32_multi_array.hpp>
#include <tdt_interface/msg/receive_data.hpp>
#include <tdt_interface/msg/send_data.hpp>
#include <std_msgs/msg/int32.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <queue>
#include <vector>
#include <limits>
#include <cstring>
#include <map>

using namespace std;
using namespace rclcpp;
using namespace std::chrono_literals;

class core : public rclcpp::Node {
public:
    core() : Node("core_node") {
        player_id_=1;
        linear_velocity_ = 100.0; // 线速度
        
        // 地图节点
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
        point_set[12]=cv::Point2f(-20.46,-20.86);//12 3号敌人
        point_set[13]=cv::Point2f(-25.00,-37.50);//13 5号敌人
        point_set[14]=cv::Point2f(-27.23,4.10);//14 4号敌人
        // map adjacency 
        map_[0].push_back(1);  map_[1].push_back(0);
        map_[1].push_back(2);  map_[2].push_back(1);
        map_[2].push_back(13);  map_[13].push_back(2);
        map_[2].push_back(5);  map_[5].push_back(2);
        map_[2].push_back(6);  map_[6].push_back(2);
        map_[2].push_back(8);  //单向
        map_[3].push_back(4);  map_[4].push_back(3);
        map_[3].push_back(13);  map_[13].push_back(3);
        map_[3].push_back(11);  map_[11].push_back(3);
        map_[4].push_back(5);  map_[5].push_back(4);
        map_[4].push_back(6);  map_[6].push_back(4);
        map_[4].push_back(11);  map_[11].push_back(4);
        map_[4].push_back(14);  map_[14].push_back(4);
        map_[5].push_back(6);  map_[6].push_back(5);
        map_[6].push_back(7);  map_[7].push_back(6);
        map_[8].push_back(9);  //单向
        map_[9].push_back(10); //单向
        map_[10].push_back(6);  map_[6].push_back(10); 
        map_[11].push_back(12);  map_[12].push_back(11);
        
        is_shooting_ = false;
        last_gimbal_control_time_ = this->now();
        turn_completed_ = false;
        shooting_authority_transferred_ = false;
        eps=0.50;
        std::memset(blue_healths_, 0, sizeof(blue_healths_));
        std::memset(red_healths_, 0, sizeof(red_healths_));
        
        // 初始化敌人配置
        initializeEnemyConfig();
        
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
            
        string turn_topic_name = "target_angles_player_" + std::to_string(player_id_);
        turn_publisher_ = this->create_publisher<tdt_interface::msg::SendData>(turn_topic_name, 10);
        
        healths_sub_ = this->create_subscription<std_msgs::msg::Int32MultiArray>(
                "game_healths", 10,
                std::bind(&core::healths_callback, this, std::placeholders::_1));

        time_sub_ = this->create_subscription<std_msgs::msg::Int32>(
                "game_time", 10,
                std::bind(&core::time_callback, this, std::placeholders::_1));

        // 新增：订阅射击状态
        
        shoot_state_pub_ = this->create_publisher<std_msgs::msg::Int32>("shoot_state", 10);
        timer_ = this->create_wall_timer(
            100ms, std::bind(&core::publishCommand, this));
        last_detection_time_ = this->now();
        last_display_time_ = this->now();
        detection_count_ = 0;

        // FSM 初始化
        current_state_ = State::INIT;
        last_state_change_time_ = this->now();
        revive_start_time_ = this->now();
        invincible_end_time_ = this->now();
        current_target_node_ = -1;
        game_started_ = false;
        first_death_ignored_ = false;
        
        // 新增：云台控制状态
        is_shooting_ = false;
        last_gimbal_control_time_ = this->now();
        
        // 新增：云台控制标志
        initial_orientation_set_ = false;
        current_enemy_orientation_set_ = false;
        
        // 当前攻击的敌人ID
        current_enemy_id_ = -1;
        
        RCLCPP_INFO(this->get_logger(), "目标检测订阅节点已启动");
    }
    

private:
    // 初始化敌人配置
    void initializeEnemyConfig() {
        // 敌人ID到节点和角度的映射
        // 格式: {敌人ID, {节点ID, 目标yaw角度, 是否已设置朝向}}
        enemy_config_[2] = {7, 90.0f, false};   // 2号敌人（敌方基地）: 节点7, yaw=90度
        enemy_config_[3] = {12, 180.0f, false};   // 3号敌人: 节点12, yaw=180度
        enemy_config_[4] = {14, 90.0f, false}; // 4号敌人: 节点14, yaw=90度  
        enemy_config_[5] = {13, -90.0f, false}; // 5号敌人: 节点13, yaw=-90度
        
        // 初始化敌人状态
        for (auto& enemy : enemy_config_) {
            int enemy_id = enemy.first;
            enemy_states_[enemy_id] = {true, 0}; // {是否存活, 上一次血量}
        }
    }

    // 根据当前状态和目标节点设置云台角度
    void setGimbalAnglesBasedOnState() {
        // 如果处于射击状态，完全由shoot模块控制云台
        if (is_shooting_) {
            RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                                "射击状态中，shoot模块控制云台");
            return;
        }

        auto msg = tdt_interface::msg::SendData();
        auto shoot_msg = std_msgs::msg::Int32();
        msg.if_shoot = false;

        // 检查是否到达当前攻击敌人的节点
        int current_node = nearest_node_index(current_x_, current_y_);
        
        // 设置初始朝向为90度（只在开始时设置一次）
        if (!initial_orientation_set_) {
            msg.yaw = 90.0f;
            msg.pitch = 0.0f;
            initial_orientation_set_ = true;
            RCLCPP_INFO(this->get_logger(), "设置初始朝向: yaw=90.0");
        }
        // 到达敌人节点且未设置过朝向时，设置为对应角度
        else if (current_enemy_id_ != -1 && current_node == enemy_config_[current_enemy_id_].node_id && 
                 !enemy_config_[current_enemy_id_].orientation_set&&enemy_states_[current_enemy_id_].alive) {
            if(current_enemy_id_==2&&current_y_<=33){
                cout<<"id 匹配"<<endl;
                msg.yaw = yaw;
                msg.pitch = pitch;
            }else{
                float target_yaw = enemy_config_[current_enemy_id_].target_yaw;
                msg.yaw = target_yaw;
                msg.pitch = 0.0f;
                enemy_config_[current_enemy_id_].orientation_set = true;
                current_enemy_orientation_set_ = true;
                RCLCPP_INFO(this->get_logger(), "到达敌人%d，设置朝向: yaw=%.1f", current_enemy_id_, target_yaw);
            }
        }
            
        else if (current_enemy_id_ != -1 && current_node == enemy_config_[current_enemy_id_].node_id && 
                 current_enemy_orientation_set_&&enemy_states_[current_enemy_id_].alive) {
                //  RCLCPP_INFO(this->get_logger(), "debug：current_enemy_id_=%d != -1 && current_node=%d == enemy_config_[current_enemy_id_].node_id=%d && current_enemy_orientation_set_=%d && !turn_completed_=%d", current_enemy_id_, current_node,enemy_config_[current_enemy_id_].node_id,current_enemy_orientation_set_,turn_completed_);
            // 检查当前云台角度是否接近目标角度
            float target_yaw = enemy_config_[current_enemy_id_].target_yaw;
            if (fabs(yaw - target_yaw) < 5.0f) { // 容忍度5度
                turn_completed_ = true;
                RCLCPP_INFO(this->get_logger(), "敌人%d转向完成，当前yaw=%.2f", current_enemy_id_, yaw);
                
                // 发布射击状态，将射击权交给shoot模块
                shoot_msg.data = 1;
                shoot_state_pub_->publish(shoot_msg);
                is_shooting_ = true;
                shooting_authority_transferred_ = true;
                RCLCPP_INFO(this->get_logger(), "射击权已交给shoot模块");
            } else {
                RCLCPP_DEBUG(this->get_logger(), "等待转向完成，当前yaw=%.2f，目标yaw=%.1f", yaw, target_yaw);
            }
            return;
        }
        // 如果当前敌人被打死，将云台转回90度
        else if (current_enemy_id_ != -1  && 
                 current_enemy_orientation_set_ && !enemy_states_[current_enemy_id_].alive) {
            msg.yaw = 90.0f;
            msg.pitch = 0.0f;
            RCLCPP_INFO(this->get_logger(), "敌人%d已被击败，回调云台转向: yaw=90.0", current_enemy_id_);
            // 检查转向是否完成
            if (fabs(yaw - 90.0f) < 5.0f) {
                RCLCPP_INFO(this->get_logger(), "云台回调转向完成");
                // 重置相关标志，准备下一个目标
                enemy_config_[current_enemy_id_].orientation_set = false;
                current_enemy_orientation_set_ = false;
                turn_completed_ = false;
                current_enemy_id_ = -1;
            }
        }
        else {
            msg.yaw = yaw;
            msg.pitch = pitch;
            return; 
        }
        
        shoot_msg.data=0;
        auto now = this->now();
        if ((now - last_gimbal_control_time_).seconds() > 0.5) { 
            shoot_state_pub_->publish(shoot_msg);
            turn_publisher_->publish(msg);
            last_gimbal_control_time_ = now;
        }
    }

    // 检查敌人状态并处理射击权回收
    void checkEnemyStatus() {
        // 检查所有增益敌人血量变化
        for (auto& enemy_state : enemy_states_) {
            int enemy_id = enemy_state.first;
            int current_health = red_healths_[enemy_id - 2]; // 敌人ID到血量数组索引的映射
            if(enemy_id==2){
                current_health = red_healths_[112];
            }
            // 如果之前有血量但现在为0，说明敌人被打死了
            if (enemy_state.second.previous_health > 0 && current_health <= 0) {
                enemy_state.second.alive = false;
                RCLCPP_INFO(this->get_logger(), "敌人%d已被击败！", enemy_id);
                
                // 如果这个敌人是当前正在攻击的敌人，收回射击权
                if (current_enemy_id_ == enemy_id && is_shooting_) {
                    auto shoot_msg = std_msgs::msg::Int32();
                    shoot_msg.data = 0;
                    shoot_state_pub_->publish(shoot_msg);
                    is_shooting_ = false;
                    shooting_authority_transferred_ = false;
                    RCLCPP_INFO(this->get_logger(), "已收回射击权（敌人%d死亡）", enemy_id);
                }
            }
            
            // 更新上一次的血量记录
            enemy_state.second.previous_health = current_health;
            
            // 如果敌人重新出现（复活等情况）
            if (current_health > 0 && !enemy_state.second.alive) {
                enemy_state.second.alive = true;
                RCLCPP_INFO(this->get_logger(), "敌人%d重新出现", enemy_id);
            }
        }
    }

    // 获取敌人对应的血量数组索引
    int getEnemyHealthIndex(int enemy_id) {
        // 敌人ID: 3,4,5 对应血量数组索引: 1,2,3
        return enemy_id - 2;
    }

    bool bfs_find_node_path(int start_idx, int goal_idx, vector<int>& out_path) {
        if (start_idx < 0 || goal_idx < 0 || start_idx == goal_idx) {
            return false;
        }
        
        int N = 20;
        vector<int> parent(N, -1);
        vector<bool> visited(N, false);
        vector<int> distance(N, INT_MAX); // 记录到每个节点的距离
        queue<int> q;
        
        q.push(start_idx);
        visited[start_idx] = true;
        distance[start_idx] = 0;
        bool found = false;
        
        while(!q.empty() && !found) {
            int u = q.front(); 
            q.pop();
            
            for (int v : map_[u]) {
                if (!visited[v]) {
                    visited[v] = true;
                    parent[v] = u;
                    distance[v] = distance[u] + 1;
                    q.push(v);
                    
                    if (v == goal_idx) {
                        found = true;
                        break;
                    }
                }
            }
        }
        
        if (!found) {
            return false;
        }
        
        // 回溯路径
        vector<int> reverse_path;
        int current = goal_idx;
        while (current != -1) {
            reverse_path.push_back(current);
            current = parent[current];
        }
        
        out_path.assign(reverse_path.rbegin(), reverse_path.rend());
        
        // 记录路径信息用于调试
        // RCLCPP_INFO(this->get_logger(), "从节点%d到节点%d的路径: ", start_idx, goal_idx);
        // for (int node : out_path) {
        //     RCLCPP_INFO(this->get_logger(), "-> %d", node);
        // }
        // RCLCPP_INFO(this->get_logger(), "路径长度: %zu", out_path.size());
        
        // return true;
    }

    int nearest_node_index(double x, double y) {
        int best = -1;
        double best_distance = numeric_limits<double>::max();
        
        for (int i = 0; i < 20; i++) {
            double px = point_set[i].x;
            double py = point_set[i].y;
            
            // 跳过未初始化的点
            if (i > 15 && px == 0.0f && py == 0.0f) continue;
            
            double distance = sqrt(pow(px - x, 2) + pow(py - y, 2));
            if (distance < best_distance) {
                best = i;
                best_distance = distance;
            }
        }
        return best;
    }

    // 将节点序列转换为 move_list（点序列）
    void set_move_list_from_nodes(const vector<int>& nodes) {
        // 清空当前路径
        while(!move_list.empty()) {
            move_list.pop();
        }
        
        // 添加新路径点
        for (int idx : nodes) {
            move_list.push(point_set[idx]);
        }
        
        if (!move_list.empty()) {
            RCLCPP_INFO(this->get_logger(), "设置新路径，共 %zu 个路径点", nodes.size());
        }
    }

    // 检查增益敌人是否存在
    bool hasAliveBuffEnemies() {
        // 3号敌人(横向平移靶), 4号敌人(前后平移靶), 5号敌人(陀螺靶)
        return (red_healths_[2] > 0) || (red_healths_[3] > 0);
    }

    // 选择要攻击的增益敌人目标点
    int selectBuffEnemyTarget() {
        static int last_target_enemy = 5; // 默认目标敌人ID
        static rclcpp::Time last_change_time = this->now();
        
        auto now = this->now();
        auto change_duration = now - last_change_time;
        
        // 只有当目标改变持续时间超过3秒才允许切换目标，避免频繁切换
        if (change_duration.seconds() < 3.0) {
            return last_target_enemy;
        }
        
        int new_target_enemy = last_target_enemy;
        
        // 优先攻击陀螺靶（5号敌人）- 积分最高
        if (red_healths_[3] > 0 && enemy_states_[5].alive) {
            new_target_enemy = 5; // 5号敌人
        } else if (red_healths_[2] > 0 && enemy_states_[4].alive) {
            new_target_enemy = 4; // 4号敌人
        }
        //  else if (red_healths_[1] > 0 && enemy_states_[3].alive) {
        //     new_target_enemy = 3; // 3号敌人
        // }
        
        if (new_target_enemy != last_target_enemy) {
            last_target_enemy = new_target_enemy;
            last_change_time = now;
            RCLCPP_INFO(this->get_logger(), "切换攻击目标到敌人%d", new_target_enemy);
        }
        
        return last_target_enemy;
    }

    // 根据敌人ID获取对应的目标节点
    int getTargetNodeByEnemyId(int enemy_id) {
        if (enemy_config_.find(enemy_id) != enemy_config_.end()) {
            return enemy_config_[enemy_id].node_id;
        }
        return -1;
    }

    bool isDead() {
        return blue_healths_[0] <= 0;
    }
    bool isBaseInvincible() {
        return time_ >= 13 * 60; // 剩余时间大于13分钟时基地无敌
    }
    // 检查是否已经到达目标节点
    bool isAtTargetNode(int target_node) {
        if (target_node == -1) return false;
        
        int current_node = nearest_node_index(current_x_, current_y_);
        return current_node == target_node;
    }
    bool isGameStarted() {
        return time_ < 900;
    }

    // 简化的FSM状态转移逻辑
    void calculation_path() {
        if (!game_started_) {
            game_started_ = isGameStarted();
            if (!game_started_) {
                current_state_ = State::INIT;
                return;
            } else {
                RCLCPP_INFO(this->get_logger(), "游戏开始！");
            }
        }
        checkEnemyStatus();

        // 检查复活机制 - 忽略开局第一次死亡判定
        if (isDead()) {
            if (!first_death_ignored_) {
                first_death_ignored_ = true;
                current_state_ = State::ATTACK_BUFF_ENEMY;
                last_state_change_time_ = this->now();
                current_target_node_ = -1;
                RCLCPP_INFO(this->get_logger(), "忽略开局死亡判定，直接开始攻击");
                return;
            }
            //
            return;
        }
        auto now = this->now();
        auto state_duration = now - last_state_change_time_;
        
        // 只有当状态持续时间超过2秒才允许状态转换，避免频繁切换
        if (state_duration.seconds() < 2.0) {
            return;
        }

        // 状态转换逻辑
         switch (current_state_) {
            case State::INIT:
                current_state_ = State::ATTACK_BUFF_ENEMY;
                last_state_change_time_ = now;
                current_target_node_ = -1;
                RCLCPP_INFO(this->get_logger(), "STATE -> ATTACK_BUFF_ENEMY");
                break;
                
            case State::ATTACK_BUFF_ENEMY:
                if (!hasAliveBuffEnemies()){
                    current_state_ = State::ATTACK_BASE;
                    last_state_change_time_ = now;
                    current_target_node_ = -1;
                    RCLCPP_INFO(this->get_logger(), "STATE -> ATTACK_BASE (增益敌人全部消灭且基地无敌时间结束)");
                }
                break;
                
            case State::ATTACK_OUTPOST:
                // 如果前哨站被摧毁或基地无敌时间结束，直接攻击基地
                if (red_healths_[4] <= 0 || !isBaseInvincible()) {
                    current_state_ = State::ATTACK_BASE;
                    last_state_change_time_ = now;
                    current_target_node_ = -1;
                    RCLCPP_INFO(this->get_logger(), "STATE -> ATTACK_BASE (前哨站已摧毁或基地无敌时间结束)");
                }
                break;
                
            case State::ATTACK_BASE:
                // 基地被摧毁后保持在该状态
                break;
        }

        // 设置云台角度
        setGimbalAnglesBasedOnState();
        // 根据当前状态设置目标路径
        setTargetBasedOnState();
        
    }

    void setTargetBasedOnState() {
        // 如果当前有路径且未完成，不重新规划
        if (!move_list.empty()) {
            return;
        }
        
        vector<int> path_nodes;
        int start_node = nearest_node_index(current_x_, current_y_);
        int target_node = -1;
        
        switch (current_state_) {
            case State::ATTACK_BUFF_ENEMY:
                current_enemy_id_ = selectBuffEnemyTarget();
                if (current_enemy_id_ != -1) {
                    target_node = getTargetNodeByEnemyId(current_enemy_id_);
                    // RCLCPP_INFO(this->get_logger(), "攻击增益敌人: 敌人%d, 目标节点%d", current_enemy_id_, target_node);
                } else {
                    // 如果没有存活的增益敌人，检查是否需要转换状态
                    RCLCPP_INFO(this->get_logger(), "没有存活的增益敌人，等待状态转换");
                    return;
                }
                break;
                
            case State::ATTACK_OUTPOST:
                target_node = 5; // 前往前哨站
                current_enemy_id_ = -1; // 重置当前敌人ID
                // RCLCPP_INFO(this->get_logger(), "攻击前哨站，目标节点5");
                break;
                
            case State::ATTACK_BASE:
                target_node = 7; // 前往敌方基地
                current_enemy_id_ = 2; // 重置当前敌人ID
                // RCLCPP_INFO(this->get_logger(), "攻击基地，目标节点7");
                break;
                
            case State::DEAD:
            case State::REVIVING:
                // 死亡和复活期间不移动
                return;
                
            default:
                // 默认情况下攻击5号敌人
                current_enemy_id_ = 5;
                target_node = getTargetNodeByEnemyId(current_enemy_id_);
                break;
        }
        
        // 检查是否需要重新规划路径
        if (target_node != current_target_node_ || isAtTargetNode(target_node)) {
            current_target_node_ = target_node;
            
            if (target_node != -1 && start_node != -1 && start_node != target_node) {
                RCLCPP_INFO(this->get_logger(), "规划路径到节点%d (敌人%d)", target_node, current_enemy_id_);
                if (bfs_find_node_path(start_node, target_node, path_nodes)) {
                    set_move_list_from_nodes(path_nodes);
                } else {
                    RCLCPP_ERROR(this->get_logger(), "无法找到从节点%d到节点%d的路径!", start_node, target_node);
                }
            } else if (start_node == target_node) {
                // RCLCPP_INFO(this->get_logger(), "已经在目标节点%d，无需移动", target_node);
            }
        }
    }

    void detection_result_callback(const std_msgs::msg::Float32MultiArray::SharedPtr msg) {
        if (msg->data.size() >= 12) {
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
    void publishCommand() {
        auto msg = geometry_msgs::msg::Twist();
        calculation_path();
        
        // 死亡和复活期间不移动
        if (current_state_ == State::DEAD || current_state_ == State::REVIVING) {
            RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 2000, 
                                "死亡/复活状态，停止移动");
            cmd_pub_->publish(msg);
            return;
        }
        
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
        
        double direction_x = dx / distance;
        double direction_y = dy / distance;
        
        msg.linear.x = direction_x * linear_velocity_;
        msg.linear.y = direction_y * linear_velocity_;
        
        cmd_pub_->publish(msg);
        
        static int count=0;
        if (count++%10 == 0) {
            std::string enemy_status = "无";
            if (current_enemy_id_ != -1) {
                enemy_status = std::to_string(current_enemy_id_) + "号(" + 
                              (enemy_states_[current_enemy_id_].alive ? "存活" : "死亡") + ")";
            }
            
            RCLCPP_INFO(this->get_logger(), "位置: (%.2lf, %.2lf), 状态: %d, 血量: %d, 目标节点: %d, 当前敌人: %s, 射击状态: %s", 
                       current_x_, current_y_, static_cast<int>(current_state_), blue_healths_[0], current_target_node_,
                       enemy_status.c_str(), is_shooting_ ? "是" : "否");
        }
    }

    // 敌人配置结构
    struct EnemyConfig {
        int node_id;           // 对应的节点ID
        float target_yaw;      // 目标yaw角度
        bool orientation_set;  // 是否已设置朝向
    };

    // 敌人状态结构
    struct EnemyState {
        bool alive;            // 是否存活
        int previous_health;   // 上一次记录的血量
    };

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
    rclcpp::Publisher<tdt_interface::msg::SendData>::SharedPtr turn_publisher_;
    rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr detection_result_sub_;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr position_sub_;
    rclcpp::Subscription<tdt_interface::msg::ReceiveData>::SharedPtr angles_sub_;
    rclcpp::Subscription<std_msgs::msg::Int32MultiArray>::SharedPtr healths_sub_;
    rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr time_sub_;
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr shoot_state_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    
    DetectionResult detection_result_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
    

    enum class State {
        INIT,
        ATTACK_BUFF_ENEMY,
        ATTACK_OUTPOST,
        ATTACK_BASE,
        DEAD,
        REVIVING,
        INVINCIBLE
    };
    State current_state_;
    rclcpp::Time last_state_change_time_;
    rclcpp::Time revive_start_time_;  // 复活开始时间
    rclcpp::Time invincible_end_time_; // 无敌结束时间
    rclcpp::Time last_gimbal_control_time_; // 上次云台控制时间
    int current_target_node_; // 当前目标节点
    bool game_started_;       // 游戏是否开始
    bool first_death_ignored_; // 是否已忽略第一次死亡判定
    rclcpp::Time last_detection_time_;
    int detection_count_;
    rclcpp::Time last_display_time_;
    int player_id_;
    double linear_velocity_;
    double current_x_ = 0.0, current_y_ = 0.0;
    queue<cv::Point2f> move_list;
    double eps;
    int point_count = 0;
    cv::Point2f point_set[20];
    vector<int> map_[20];
    int time_ = 0;
    int blue_healths_[6];  // 0:玩家, 1:3号, 2:4号, 3:5号, 4:前哨站, 5:基地
    int red_healths_[6];   // 同上
    float yaw = 0.0f;
    float pitch = 0.0f;
    int32_t sec;
    uint32_t nanosec;
    bool is_shooting_;
    bool turn_completed_;           // 转向是否完成
    bool shooting_authority_transferred_;  // 射击权是否已转移
    
    // 新增：云台控制标志
    bool initial_orientation_set_;   // 初始朝向是否已设置
    bool current_enemy_orientation_set_; // 当前敌人朝向是否已设置
    
    // 新增：敌人配置和状态管理
    std::map<int, EnemyConfig> enemy_config_;  // 敌人ID到配置的映射
    std::map<int, EnemyState> enemy_states_;   // 敌人ID到状态的映射
    int current_enemy_id_;                     // 当前攻击的敌人ID
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<core>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}