#!/bin/bash
echo “start...”
PLAYER_OPTION="all" 
if [ $# -gt 0 ]; then  # 如果用户输入了参数（$#是参数个数）
    case $1 in  # 根据第一个参数（$1）判断选项
        1)
            PLAYER_OPTION="player1"  # 输入1→启动玩家1节点
            ;;
        2)
            PLAYER_OPTION="player2"  # 输入2→启动玩家2节点
            ;;
        all|"")
            PLAYER_OPTION="all"  # 输入all或空→启动所有节点
            ;;
        *)  # 其他无效参数
            echo "❌ 无效参数: $1"
            exit 1 
            ;;
    esac
fi
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

case $PLAYER_OPTION in
    "all")  # 启动所有节点
        echo "🎮 启动所有节点..."
        
        # 启动Unity相机捕获节点（游戏画面采集）
        start_node "Unity图像捕获节点" "image" "image_node"
        # 启动玩家1控制节点
        start_node "玩家1移动控制" "move" "move_node" "1"  # 传递玩家ID=1
        start_node "玩家1射击控制" "shoot" "shoot_node" "1"
        
        # 启动玩家2控制节点
        start_node "玩家2移动控制" "move" "move_node" "2"  # 传递玩家ID=2
        start_node "玩家2射击控制" "shoot" "shoot_node" "2"
        ;;
        
    "player1")  # 只启动玩家1节点
        echo "🎮 只启动玩家1相关节点..."
        
        # 启动Unity相机捕获节点（注释表示可选，按需开启）
        # start_node "Unity图像捕获节点" "image" "image_node"
        
        # 启动玩家1控制节点
        start_node "玩家1移动控制" "move" "move_node" "1"
        start_node "玩家1射击控制" "shoot" "shoot_node" "1"
        ;;
        
    "player2")  # 只启动玩家2节点
        echo "🎮 只启动玩家2相关节点..."
        
        # 启动Unity相机捕获节点
        start_node "Unity图像捕获节点" "mage" "image_node"
        
        # 启动玩家2控制节点
        start_node "玩家2移动控制" "move" "move_node" "2"
        start_node "玩家2射击控制" "shoot" "shoot_node" "2"
        ;;
esac

ros2 topic pub --once /game_start_player_1 std_msgs/Bool "data: true" &
ros2 topic pub --once /game_start_player_2 std_msgs/Bool "data: true" &      

echo "按 Ctrl+C 停止所有节点"

wait