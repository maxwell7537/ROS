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
        linear_velocity_ = 40.0; // 线速度
        
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
        // point_set[13]=cv::Point2f(-25.00,-37.50);//13 5号敌人
        point_set[14]=cv::Point2f(-27.23,4.10);//14 4号敌人
        // map adjacency 
        map_[0].push_back(1);  map_[1].push_back(0);
        map_[1].push_back(2);  map_[2].push_back(1);
        map_[2].push_back(3);  map_[3].push_back(2);
        map_[2].push_back(5);  map_[5].push_back(2);
        map_[2].push_back(6);  map_[6].push_back(2);
        map_[2].push_back(8);  //单向
        map_[3].push_back(4);  map_[4].push_back(3);
        // map_[3].push_back(13);  map_[13].push_back(3);
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
        eps=0.30;

        last_valid_detection_time_ = this->now();
        no_target_reset_delay_ = 2.0;
        is_resetting_gimbal_ = false;
        reset_target_yaw_ = 90.0;

        is_tracking_enemy1_ = false;
        enemy1_last_detection_time_ = this->now();
        enemy1_lost_timeout_ = 3.0;
        original_target_enemy_id_ = -1;
        was_shooting_ = false;
        enemy1_tracking_start_time_ = this->now();
        max_enemy1_tracking_time_ = 5.0;

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
            10ms, std::bind(&core::publishCommand, this));
        last_detection_time_ = this->now();
        last_display_time_ = this->now();
        detection_count_ = 0;

        // FSM 初始化
        current_state_ = State::INIT;
        last_state_change_time_ = this->now();
        revive_start_time_ = this->now();
        invincible_end_time_ = this->now();
        current_target_node_ = 13;//特殊
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

        last_error_x_ = 0.0;
        last_error_y_ = 0.0;
        last_control_time_ = this->now();
        kp_ = 2.0;   // 可以根据实际效果调整
        kd_ = 0.5;   // 可以根据实际效果调整
        
        RCLCPP_INFO(this->get_logger(), "目标检测订阅节点已启动");
    }
    

private:
    // 初始化敌人配置
    void initializeEnemyConfig() {
        // 敌人ID到节点和角度的映射
        enemy_config_[2] = {7, 90.0f, false};   // 2号敌人（敌方基地）: 节点7, yaw=90度
        enemy_config_[3] = {12, 180.0f, false};   // 3号敌人: 节点12, yaw=180度
        enemy_config_[4] = {14, 90.0f, false}; // 4号敌人: 节点14, yaw=90度  
        enemy_config_[5] = {3, -90.0f, false}; // 5号敌人: 节点3, yaw=-90度
        enemy_config_[6] = {5, 86.0f, false}; // qs: 节点5, yaw=86度
        
        // 初始化敌人状态
        for (auto& enemy : enemy_config_) {
            int enemy_id = enemy.first;
            enemy_states_[enemy_id] = {true, 0}; // {是否存活, 上一次血量}
        }
    }

    // 根据当前状态和目标节点设置云台角度
    void setGimbalAnglesBasedOnState() {
        if(is_tracking_enemy1_) {
                return;
        }
        if(shouldResetGimbal()) {
            resetGimbalToInitial();
            return; // 重置过程中不执行其他云台控制
        }
        
        // 如果处于射击状态，完全由shoot模块控制云台
        if (is_shooting_) {
            RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                                "射击状态中，shoot模块控制云台");
            return;
        }
        auto msg = tdt_interface::msg::SendData();
        auto shoot_msg = std_msgs::msg::Int32();
        msg.if_shoot = false;

        
        // 设置初始朝向为90度（只在开始时设置一次）
        if (!initial_orientation_set_) {
            msg.yaw = 90.0f;
            msg.pitch = 0.0f;
            initial_orientation_set_ = true;
            RCLCPP_INFO(this->get_logger(), "设置初始朝向: yaw=90.0");
        } else if (current_enemy_id_ != -1 && !check_nearest_node(enemy_config_[current_enemy_id_].node_id)) {
                // 未到达敌人节点，进行小范围旋转搜索
                auto now = this->now();
                float time_offset = (now - last_gimbal_control_time_).seconds();
                // 以1秒为周期在90°±5°范围内摆动
                float angle_offset = 5.0f * sin(2 * M_PI * time_offset);
                msg.yaw = 90.0f + angle_offset;
                msg.pitch = 0.0f;
                RCLCPP_DEBUG(this->get_logger(), "云台搜索旋转: yaw=%.2f", msg.yaw);
                return; // 退出后续逻辑，直接发布搜索角度
            }
        // 到达敌人节点且未设置过朝向时，设置为对应角度
        else if (current_enemy_id_ != -1 && check_nearest_node(enemy_config_[current_enemy_id_].node_id) && 
                 !enemy_config_[current_enemy_id_].orientation_set&&enemy_states_[current_enemy_id_].alive) {
            if(current_enemy_id_==2&&current_y_<=34.8){
                // cout<<"id 匹配"<<endl;
                msg.yaw = yaw;
                msg.pitch = pitch;
            }else{
                float target_yaw = enemy_config_[current_enemy_id_].target_yaw;
                msg.yaw = target_yaw;
                msg.pitch = 0.0f;
                if(current_enemy_id_==6){
                    msg.pitch=20.0f;
                }
                enemy_config_[current_enemy_id_].orientation_set = true;
                current_enemy_orientation_set_ = true;
                RCLCPP_INFO(this->get_logger(), "到达敌人%d，设置朝向: yaw=%.1f", current_enemy_id_, target_yaw);
            }
        }
            
        else if (current_enemy_id_ != -1 && check_nearest_node(enemy_config_[current_enemy_id_].node_id) && 
                 current_enemy_orientation_set_&&enemy_states_[current_enemy_id_].alive) {
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
                        // RCLCPP_INFO(this->get_logger(), "debug： current_enemy_id_=%d != -1 && check_nearest_node(enemy_config_[current_enemy_id_].node_id)=%d && current_enemy_orientation_set_=%d&&enemy_states_[current_enemy_id_].alive=%d",current_enemy_id_ ,check_nearest_node(enemy_config_[current_enemy_id_].node_id),current_enemy_orientation_set_,enemy_states_[current_enemy_id_].alive);

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
        if (now.seconds()- last_gimbal_control_time_.seconds() > 0.5) { 
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
            int current_health = getEnemyHealthIndex(enemy_id);// 敌人ID到血量数组索引的映射
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
    int getEnemyHealthIndex(int id){
        if(id==2)return red_healths_[5];
        if(id==1)return red_healths_[0];
        return red_healths_[id-2];
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
        RCLCPP_INFO(this->get_logger(), "从节点%d到节点%d的路径: ", start_idx, goal_idx);
        for (int node : out_path) {
            RCLCPP_INFO(this->get_logger(), "-> %d", node);
        }
        RCLCPP_INFO(this->get_logger(), "路径长度: %zu", out_path.size());
        
        return true;
    }

    int check_nearest_node(int x) {
        double xx=point_set[x].x,  yy=point_set[x].y;
        return fabs(current_x_-xx)<3.0&&fabs(current_y_-yy)<3.0;
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
        return (red_healths_[2] > 0) || (red_healths_[3] > 0);//(red_healths_[1] > 0) ||
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
        return time_ <= 120; // 基地无敌
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
        if(is_tracking_enemy1_) {
            return;
        }
        checkEnemyStatus();
        injury();

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
                current_target_node_ = 3;
                RCLCPP_INFO(this->get_logger(), "STATE -> ATTACK_BUFF_ENEMY");
                break;
                
            case State::ATTACK_BUFF_ENEMY:
                 if (!hasAliveBuffEnemies()) {
                    if (isBaseInvincible()) {
                        // 基地无敌时攻击前哨站
                        current_state_ = State::ATTACK_OUTPOST;
                        last_state_change_time_ = now;
                        current_target_node_ = -1;
                        RCLCPP_INFO(this->get_logger(), "STATE -> ATTACK_OUTPOST (增益敌人全部消灭，基地无敌)");
                    } else {
                        // 基地无敌时间结束，直接攻击基地
                        current_state_ = State::ATTACK_BASE;
                        last_state_change_time_ = now;
                        current_target_node_ = -1;
                        RCLCPP_INFO(this->get_logger(), "STATE -> ATTACK_BASE (增益敌人全部消灭，基地无敌时间结束)");
                    }
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
            case State::INJURY:
                break;
        }

        // 设置云台角度
        setGimbalAnglesBasedOnState();
        // 根据当前状态设置目标路径
        setTargetBasedOnState();
        
    }
    
    void injury(){
        // 只在行进时判断受击
        if (!move_list.empty()) {
            checkHitStatus();
        }
        
        // 如果不在受击状态，直接返回
        if (!is_under_attack_) {
            return;
        }
        
        // 受击状态处理
        if (current_state_ != State::INJURY) {
            enterInjuryState();
        } else {
            handleInjuryState();
        }
    }

    // 修改checkHitStatus函数，修复判断逻辑
    void checkHitStatus() {
        int current_health = blue_healths_[0];
        
        // 检测血量减少（受击）
        if (current_health < last_health_) {
            int damage = last_health_ - current_health;
            
            // 血量减少就视为受击
            if (damage > 0) {
                is_under_attack_ = true;
                last_hit_time_ = this->now();
                RCLCPP_WARN(this->get_logger(), "受到攻击！伤害: %d, 当前血量: %d", damage, current_health);
            }
            
            last_health_ = current_health;
        }
        
        // 受击状态超时自动恢复
        if (is_under_attack_ && (this->now() - last_hit_time_).seconds() > 8.0) {
            RCLCPP_INFO(this->get_logger(), "受击状态超时结束");
            exitInjuryState();
        }
    }

    // 修改进入受击状态
    void enterInjuryState() {
        RCLCPP_WARN(this->get_logger(), "进入受击规避状态");
        current_state_ = State::INJURY;
        injury_start_time_ = this->now();
        is_evading_ = true;
        evasion_phase_ = 0;
        last_evasion_phase_time_ = this->now();
        
        // 保存原始目标点
        if (!move_list.empty()) {
            original_target_point_ = move_list.front();
            original_target_saved_ = true;
            RCLCPP_INFO(this->get_logger(), "保存原始目标点: (%.2f, %.2f)", 
                       original_target_point_.x, original_target_point_.y);
        } else {
            RCLCPP_WARN(this->get_logger(), "没有移动路径，无法保存原始目标点");
        }
    }

    // 修改受击状态处理
    void handleInjuryState() {
        auto now = this->now();
        auto state_duration = now - injury_start_time_;
        
        // 受击状态超时检查
        if (state_duration.seconds() > 15.0) {
            RCLCPP_INFO(this->get_logger(), "受击状态超时，恢复正常状态");
            exitInjuryState();
            return;
        }
        
        // 如果不再受击，退出受击状态
        if (!is_under_attack_ && state_duration.seconds() > 3.0) {
            RCLCPP_INFO(this->get_logger(), "威胁解除，退出受击状态");
            exitInjuryState();
            return;
        }
        
        // 执行折线规避
        executeZigzagEvasion();
    }

    // 修改折线规避，确保每次都会发布移动命令
    void executeZigzagEvasion() {
        if (!is_evading_ || !original_target_saved_) {
            RCLCPP_WARN(this->get_logger(), "规避条件不满足: is_evading=%d, original_target_saved=%d", 
                       is_evading_, original_target_saved_);
            return;
        }
        
        auto now = this->now();
        auto phase_duration = now - last_evasion_phase_time_;
        
        // 每1.5秒切换一次规避方向
        if (phase_duration.seconds() > 1.5) {
            evasion_phase_ = (evasion_phase_ + 1) % 2; // 只在左右两个方向切换
            last_evasion_phase_time_ = now;
            RCLCPP_DEBUG(this->get_logger(), "切换规避阶段: %d", evasion_phase_);
        }
        
        // 计算折线规避移动
        cv::Point2f target_point = original_target_point_;
        double dx = target_point.x - current_x_;
        double dy = target_point.y - current_y_;
        double distance = sqrt(dx*dx + dy*dy);
        
        if (distance > eps) {
            // 基础前进方向
            double base_direction_x = dx / distance;
            double base_direction_y = dy / distance;
            
            // 垂直于前进方向的偏移方向
            double perpendicular_x = -base_direction_y;
            double perpendicular_y = base_direction_x;
            
            // 根据规避阶段调整偏移方向（左或右）
            double phase_offset = (evasion_phase_ == 0) ? 1.0 : -1.0;
            double evasion_strength = 0.6; // 规避强度
            
            // 合成最终移动方向
            double final_direction_x = base_direction_x + perpendicular_x * phase_offset * evasion_strength;
            double final_direction_y = base_direction_y + perpendicular_y * phase_offset * evasion_strength;
            
            // 归一化
            double final_length = sqrt(final_direction_x*final_direction_x + final_direction_y*final_direction_y);
            if (final_length > 0) {
                final_direction_x /= final_length;
                final_direction_y /= final_length;
            }
            
            // 发布移动命令
            auto msg = geometry_msgs::msg::Twist();
            msg.linear.x = final_direction_x * linear_velocity_ * 0.8; // 受击时稍慢
            msg.linear.y = final_direction_y * linear_velocity_ * 0.8;
            cmd_pub_->publish(msg);
            
            RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 2000, 
                                "折线规避: 方向(%.2f, %.2f), 阶段%d", 
                                final_direction_x, final_direction_y, evasion_phase_);
        } else {
            RCLCPP_INFO(this->get_logger(), "已接近目标点，停止规避");
            exitInjuryState();
        }
    }

    // 修改退出受击状态
    void exitInjuryState() {
        RCLCPP_INFO(this->get_logger(), "退出受击状态");
        is_under_attack_ = false;
        is_evading_ = false;
        original_target_saved_ = false;
        
        // 恢复到攻击增益敌人状态
        current_state_ = State::ATTACK_BUFF_ENEMY;//last_state
        last_state_change_time_ = this->now();
        
        last_health_=blue_healths_[0];
        // 发布停止移动命令，确保状态切换
        auto msg = geometry_msgs::msg::Twist();
        cmd_pub_->publish(msg);
    }

    void setTargetBasedOnState() {
        // 如果当前有路径且未完成，不重新规划
        if (!move_list.empty()) {
            RCLCPP_DEBUG(this->get_logger(), "当前有未完成路径，跳过重新规划");
            return;
        }
        
        vector<int> path_nodes;
        int start_node = nearest_node_index(current_x_, current_y_);
        int target_node = -1;
        
        RCLCPP_INFO(this->get_logger(), "当前状态: %d, 当前敌人ID: %d, 起始节点: %d", 
                    static_cast<int>(current_state_), current_enemy_id_, start_node);
        
        switch (current_state_) {
            case State::ATTACK_BUFF_ENEMY:
                current_enemy_id_ = selectBuffEnemyTarget();
                if (current_enemy_id_ != -1) {
                    target_node = getTargetNodeByEnemyId(current_enemy_id_);
                    RCLCPP_INFO(this->get_logger(), "攻击增益敌人: 敌人%d, 目标节点%d", 
                            current_enemy_id_, target_node);
                } else {
                    RCLCPP_WARN(this->get_logger(), "没有存活的增益敌人，等待状态转换");
                    return;
                }
                break;
                
            case State::ATTACK_OUTPOST:
                target_node = 5; // 前往前哨站
                current_enemy_id_ = 6; // 设置当前敌人ID为6（前哨站）
                RCLCPP_INFO(this->get_logger(), "攻击前哨站，目标节点5，敌人ID6");
                break;
                
            case State::ATTACK_BASE:
                target_node = 7; // 前往敌方基地
                current_enemy_id_ = 2; // 设置当前敌人ID为2（基地）
                RCLCPP_INFO(this->get_logger(), "攻击基地，目标节点7，敌人ID2");
                break;
                
            case State::DEAD:
            case State::REVIVING:
                // 死亡和复活期间不移动
                RCLCPP_INFO(this->get_logger(), "死亡或复活状态，不移动");
                return;
                
            default:
                // 默认情况下攻击5号敌人
                current_enemy_id_ = 5;
                target_node = getTargetNodeByEnemyId(current_enemy_id_);
                RCLCPP_WARN(this->get_logger(), "默认状态，攻击敌人%d，目标节点%d", 
                        current_enemy_id_, target_node);
                break;
        }
        
        // 检查是否需要重新规划路径
        if (target_node != current_target_node_ || !check_nearest_node(target_node)) {
            current_target_node_ = target_node;
            RCLCPP_INFO(this->get_logger(), "重新规划路径到节点%d", current_target_node_);
            if (target_node != -1 && start_node != -1 && start_node != target_node) {
                RCLCPP_INFO(this->get_logger(), "规划路径从节点%d到节点%d (敌人%d)", 
                        start_node, target_node, current_enemy_id_);
                if (bfs_find_node_path(start_node, target_node, path_nodes)) {
                    set_move_list_from_nodes(path_nodes);
                    RCLCPP_INFO(this->get_logger(), "路径规划成功，共%ld个路径点", path_nodes.size());
                    
                    // 显示路径详情
                    RCLCPP_INFO(this->get_logger(), "路径详情:");
                    for (size_t i = 0; i < path_nodes.size(); ++i) {
                        cv::Point2f point = point_set[path_nodes[i]];
                        RCLCPP_INFO(this->get_logger(), "  节点%d: (%.2f, %.2f)", 
                                path_nodes[i], point.x, point.y);
                    }
                } else {
                    RCLCPP_ERROR(this->get_logger(), "无法找到从节点%d到节点%d的路径!", start_node, target_node);
                    
                    // 添加紧急处理：尝试直接移动到目标点
                    RCLCPP_WARN(this->get_logger(), "尝试直接移动到目标点");
                    cv::Point2f target_point = point_set[target_node];
                    move_list.push(target_point);
                }
            } else if (start_node == target_node&&check_nearest_node(target_node)) {
                RCLCPP_INFO(this->get_logger(), "已经在目标节点%d，无需移动", target_node);
            } else if(start_node == target_node&&(!check_nearest_node(target_node))){
                RCLCPP_WARN(this->get_logger(), "尝试直接移动到目标点");
                    cv::Point2f target_point = point_set[target_node];
                    move_list.push(target_point);
            }else{
                RCLCPP_ERROR(this->get_logger(), "无效的起始节点%d或目标节点%d", start_node, target_node);
            }
        } else {
            RCLCPP_DEBUG(this->get_logger(), "目标节点未变化且已在附近，无需重新规划");
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
            
            // 1号敌人检测逻辑
            if (detection_result_.confidence > 0.3 && detection_result_.class_id == 1) {
                if (!is_tracking_enemy1_) {
                    startTrackingEnemy1();
                }else {
                    // 如果已经在跟踪，只更新时间戳
                    enemy1_last_detection_time_ = this->now();
                }
                enemy1_last_detection_time_ = this->now();
            }
            
            if (detection_result_.confidence > 0.3) {
                last_valid_detection_time_ = this->now();
                is_resetting_gimbal_ = false;
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
        static rclcpp::Time last_movement_time = this->now();
        auto now = this->now();
        
        // 如果正在跟踪1号敌人，只执行跟踪逻辑
        if (is_tracking_enemy1_) {
            checkEnemy1Status(); // 检查1号敌人状态
            // 在跟踪期间不执行移动，完全由shoot模块控制
            return;
        }
        
        if (!move_list.empty()) {
            last_movement_time = now;
        } else if ((now - last_movement_time).seconds() > 10.0) {
            RCLCPP_WARN(this->get_logger(), "检测到卡死状态，强制重新规划路径");
            current_target_node_ = -1;
            setTargetBasedOnState();
            setGimbalAnglesBasedOnState();
            last_movement_time = now;
        }
        auto msg = geometry_msgs::msg::Twist();
        calculation_path();
        
        // 死亡和复活期间不移动
        if (current_state_ == State::DEAD || current_state_ == State::REVIVING) {
            cmd_pub_->publish(msg);
            return;
        }
        
        if (current_state_ == State::INJURY) {
            return;
        }

        if (move_list.empty()) {
            cmd_pub_->publish(msg);
            return;
        }

        cv::Point2f target_point = move_list.front();
        double dx = target_point.x - current_x_;
        double dy = target_point.y - current_y_;
        double distance = sqrt(dx*dx + dy*dy);
        
        // 到达判断
        if (distance < eps) {
            move_list.pop();
            // 重置PD控制器状态
            last_error_x_ = 0.0;
            last_error_y_ = 0.0;
            cmd_pub_->publish(msg);
            
            if (!move_list.empty()) {
                RCLCPP_INFO(this->get_logger(), "到达路径点，下一个目标: (%.2f, %.2f)", 
                        move_list.front().x, move_list.front().y);
            }
            return;
        }
        
        // 使用PD控制计算速度
        double vx, vy;
        calculatePDControl(dx, dy, vx, vy);
        
        // 根据距离进一步限制速度（安全减速）
        double target_speed = calculateOptimalSpeed(distance);
        double current_speed = sqrt(vx*vx + vy*vy);
        
        if (current_speed > target_speed) {
            double scale = target_speed / current_speed;
            vx *= scale;
            vy *= scale;
        }
        
        msg.linear.x = vx;
        msg.linear.y = vy;
        
        cmd_pub_->publish(msg);
        
        // 调试信息
        static int count = 0;
        if (count++ % 25 == 0) { // 约1.25秒输出一次
            RCLCPP_INFO(this->get_logger(), 
                    "位置: (%.2f, %.2f), 目标: (%.2f, %.2f), 距离: %.3f, 速度: (%.2f, %.2f)", 
                    current_x_, current_y_, target_point.x, target_point.y, 
                    distance, vx, vy);
        }
    }

    // 新增：计算最优速度的函数
    double calculateOptimalSpeed(double distance) {
        // 使用更平滑的速度曲线
        if (distance <= eps) {
            return 0.0;
        }
        else if (distance < 0.2) {
            return 1.0; // 极近距离，非常慢
        }
        else if (distance < 0.5) {
            return 3.0;
        }
        else if (distance < 1.0) {
            return 8.0;
        }
        else if (distance < 2.0) {
            return 15.0;
        }
        else if (distance < 4.0) {
            return 22.0;
        }
        else {
            return linear_velocity_; // 最大速度
        }
    }

    double last_error_x_ = 0.0;
    double last_error_y_ = 0.0;
    rclcpp::Time last_control_time_;
    double kp_ = 2.0;  // 比例系数
    double kd_ = 0.5;  // 微分系数

    // PD控制计算
    void calculatePDControl(double dx, double dy, double& out_vx, double& out_vy) {
        auto now = this->now();
        double dt = (now - last_control_time_).seconds();
        
        if (dt < 0.01) dt = 0.01; // 最小时间间隔
        if (dt > 0.2) dt = 0.2;   // 最大时间间隔
        
        // 比例项
        double p_x = kp_ * dx;
        double p_y = kp_ * dy;
        
        // 微分项
        double d_x = kd_ * (dx - last_error_x_) / dt;
        double d_y = kd_ * (dy - last_error_y_) / dt;
        
        out_vx = p_x + d_x;
        out_vy = p_y + d_y;
        
        // 限制最大速度
        double speed = sqrt(out_vx * out_vx + out_vy * out_vy);
        if (speed > linear_velocity_) {
            double scale = linear_velocity_ / speed;
            out_vx *= scale;
            out_vy *= scale;
        }
        
        last_error_x_ = dx;
        last_error_y_ = dy;
        last_control_time_ = now;
    }

    bool shouldResetGimbal() {
        if (!is_shooting_ || !shooting_authority_transferred_) {
            return false;
        }
        
        auto now = this->now();
        double time_since_last_detection = (now - last_valid_detection_time_).seconds();
        
        return (time_since_last_detection > no_target_reset_delay_ && !is_resetting_gimbal_);
    }
    void resetGimbalToInitial() {
        reset_target_yaw_=enemy_config_[current_enemy_id_].target_yaw;
        if (!is_resetting_gimbal_) {
            RCLCPP_INFO(this->get_logger(), "开始重置云台到初始位置: yaw=%.1f", reset_target_yaw_);
            is_resetting_gimbal_ = true;
        }

        auto msg = tdt_interface::msg::SendData();
        auto shoot_msg = std_msgs::msg::Int32();
        
        
        msg.yaw = reset_target_yaw_;
        msg.pitch = 0.0f;
        msg.if_shoot = false;
        
        shoot_msg.data = 0;
        is_resetting_gimbal_ = false;
        RCLCPP_INFO(this->get_logger(), "云台重置完成: yaw=%.1f", reset_target_yaw_);
        
        // 收回射击权，等待重新检测到目标
        is_shooting_ = false;
        shooting_authority_transferred_ = false;
        RCLCPP_INFO(this->get_logger(), "射击权已收回，等待重新检测目标");
        
        // 发布控制指令
        auto now = this->now();
        if ((now - last_gimbal_control_time_).seconds() > 0.1) { 
            shoot_state_pub_->publish(shoot_msg);
            turn_publisher_->publish(msg);
            last_gimbal_control_time_ = now;
        }
    }

    void startTrackingEnemy1() {
        auto now = this->now();
        
        // 检查是否已经在跟踪
        if (is_tracking_enemy1_) {
            // 如果已经在跟踪，只更新时间戳
            enemy1_last_detection_time_ = now;
            return;
        }

        RCLCPP_WARN(this->get_logger(), "检测到1号敌人，开始跟踪！");
        
        // 保存当前状态
        is_tracking_enemy1_ = true;
        original_target_enemy_id_ = current_enemy_id_;
        was_shooting_ = is_shooting_;
        enemy1_tracking_start_time_ = now;
        enemy1_last_detection_time_ = now;
        
        // 停止移动并清空路径
        while(!move_list.empty()) {
            move_list.pop();
        }
        
        // 发布停止移动命令
        auto stop_msg = geometry_msgs::msg::Twist();
        cmd_pub_->publish(stop_msg);
        
        // 设置云台到当前角度（保持稳定）
        // auto yuntai_msg = tdt_interface::msg::SendData();
        // yuntai_msg.yaw = yaw;
        // yuntai_msg.pitch = pitch;
        // yuntai_msg.if_shoot = false;
        // turn_publisher_->publish(yuntai_msg);

        // 发布射击授权（特殊消息，比如2表示只攻击1号敌人）
        auto shoot_msg = std_msgs::msg::Int32();
        shoot_msg.data = 2; // 2表示只攻击1号敌人
        shoot_state_pub_->publish(shoot_msg);
        
        // 设置射击状态
        is_shooting_ = true;
        shooting_authority_transferred_ = true;
        
        RCLCPP_INFO(this->get_logger(), "已授权射击1号敌人，停止移动，当前yaw=%.2f", yaw);
    }

    // 改进的1号敌人状态检查
        void checkEnemy1Status() {
        if (!is_tracking_enemy1_) {
            return;
        }
        
        auto now = this->now();
        double time_since_last_detection = now.seconds() - enemy1_last_detection_time_.seconds();
        double total_tracking_time = now.seconds() - enemy1_tracking_start_time_.seconds();
        
        // 检查总跟踪时间
        if (total_tracking_time > max_enemy1_tracking_time_) {
            RCLCPP_WARN(this->get_logger(), "1号敌人跟踪超时 (%.1f秒)，强制停止", total_tracking_time);
            stopTrackingEnemy1();
            return;
        }
        
        // 检查1号敌人是否死亡或目标丢失
        if (red_healths_[0] <= 0) {
            RCLCPP_INFO(this->get_logger(), "1号敌人已死亡，停止跟踪");
            stopTrackingEnemy1();
        } else if (time_since_last_detection > enemy1_lost_timeout_) {
            RCLCPP_WARN(this->get_logger(), "1号敌人目标丢失(%.1f秒)，停止跟踪", time_since_last_detection);
            stopTrackingEnemy1();
        }
    }

    // 改进的停止跟踪方法
    void stopTrackingEnemy1() {
        RCLCPP_INFO(this->get_logger(), "停止跟踪1号敌人");
        
        // 恢复原有状态
        is_tracking_enemy1_ = false;
        current_enemy_id_ = original_target_enemy_id_;
        
        // 收回射击权限
        auto shoot_msg = std_msgs::msg::Int32();
        shoot_msg.data = 0;
        shoot_state_pub_->publish(shoot_msg);
        
        // 恢复射击状态
        is_shooting_ = was_shooting_;
        shooting_authority_transferred_ = was_shooting_;
        
        // 重置云台控制标志，确保能重新触发云台控制逻辑
        initial_orientation_set_ = false;
        current_enemy_orientation_set_ = false;
        
        // 重要修复：重置当前敌人的朝向设置，确保能重新设置云台角度
        if (current_enemy_id_ != -1) {
            enemy_config_[current_enemy_id_].orientation_set = false;
        }
        turn_completed_ = false;
        
        // 重新规划路径
        current_target_node_ = -1; // 强制重新规划路径
        setTargetBasedOnState();
        setGimbalAnglesBasedOnState();
        
        RCLCPP_INFO(this->get_logger(), "已恢复原有路径和状态，当前敌人ID=%d", current_enemy_id_);
        
        // 重要修复：如果当前在基地节点，立即重新授权射击
        if (current_enemy_id_ == 2 && check_nearest_node(7)) {
            RCLCPP_INFO(this->get_logger(), "在基地节点，立即重新设置云台角度");
            // 强制重置云台角度设置，确保能重新进入射击授权流程
            enemy_config_[2].orientation_set = false;
            current_enemy_orientation_set_ = false;
            setGimbalAnglesBasedOnState();
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
        INJURY,
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

    rclcpp::Time last_hit_time_;           // 上次受击时间
    bool is_under_attack_ = false;         // 是否正在受击
    int last_health_ = 200;                // 上次记录的血量
    rclcpp::Time injury_start_time_;       // 受击状态开始时间
    bool is_evading_ = false;              // 是否正在规避
    cv::Point2f original_target_point_;    // 原始目标点
    bool original_target_saved_ = false;   // 是否已保存原始目标
    int evasion_phase_ = 0;                // 规避阶段
    rclcpp::Time last_evasion_phase_time_; // 上次规避阶段切换时间

    
    rclcpp::Time last_valid_detection_time_;
    double no_target_reset_delay_ ;
    bool is_resetting_gimbal_ ;
    double reset_target_yaw_ ;

    bool is_tracking_enemy1_;
    rclcpp::Time enemy1_last_detection_time_;
    double enemy1_lost_timeout_ ;
    int original_target_enemy_id_ ;
    bool was_shooting_ ;
    rclcpp::Time enemy1_tracking_start_time_;
    double max_enemy1_tracking_time_ ; 

};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<core>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}