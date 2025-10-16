#!/bin/bash
echo “start...”

source /opt/ros/humble/setup.bash
if [ -f "install/setup.bash" ]; then  # 如果本地有编译好的包（install目录）
    source install/setup.bash  # 加载本地包的环境变量（优先使用本地编译的节点）
fi

export ROS_DOMAIN_ID=0  # 配置ROS 2的域ID，用于隔离不同网络中的ROS节点（避免跨域干扰）

start_node() {
    local name=$1  # 节点名称（仅用于日志显示）
    local package=$2  # 节点所在的ROS包名
    local executable=$3  # 节点的可执行文件名
    local args=$4  # 传递给节点的参数（可选）
    
    echo "启动 $name..."  # 打印启动提示
    if [ -z "$args" ]; then  # 如果没有参数（-z判断字符串为空）
        ros2 run $package $executable &  # 无参数启动节点，&表示后台运行
    else
        ros2 run $package $executable $args &  # 带参数启动节点，后台运行
    fi
    sleep 1  # 等待1秒，避免节点启动过于密集导致的资源竞争或通信延迟
}

        
# 启动Unity相机捕获节点（游戏画面采集）
#start_node "Unity图像捕获节点" "image" "image_node"
start_node "移动控制" "move" "move_node"  
start_node "射击控制" "shoot" "shoot_node"
start_node "模型节点" "model" "model_node"
start_node "核心节点" "core" "core_node"

ros2 topic pub --once /game_start_player_1 std_msgs/Bool "data: true" &
ros2 topic pub --once /game_start_player_2 std_msgs/Bool "data: true" &      

echo "按 Ctrl+C 停止所有节点"

wait