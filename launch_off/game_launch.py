from launch.launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess, DeclareLaunchArgument
from launch.substitutions import EnvironmentVariable
from launch.conditions import IfCondition

def generate_launch_description():
    # 声明ROS域ID（与脚本中export ROS_DOMAIN_ID=0对应）
    ros_domain_id = EnvironmentVariable('ROS_DOMAIN_ID', default_value='0')
    
    # 1. 启动Unity图像捕获节点
    image_node = Node(
        package='image',          # 对应脚本中的package="image"
        executable='image_node',  # 对应脚本中的executable="image_node"
        name='unity_image_capture',
        output='screen',
        environment=[('ROS_DOMAIN_ID', ros_domain_id)]  # 传递域ID环境变量
    )
    
    # 2. 启动移动控制节点
    move_node = Node(
        package='move',
        executable='move_node',
        name='move_controller',
        output='screen',
        environment=[('ROS_DOMAIN_ID', ros_domain_id)]
    )
    
    # 3. 启动射击控制节点
    shoot_node = Node(
        package='shoot',
        executable='shoot_node',
        name='shoot_controller',
        output='screen',
        environment=[('ROS_DOMAIN_ID', ros_domain_id)]
    )
    
    # 4. 启动模型节点（修正原脚本中的引号错误）
    model_node = Node(
        package='model',          # 原脚本中"模型节点“ ”model"存在引号错误，修正为正确包名
        executable='model_node',
        name='detection_model',
        output='screen',
        environment=[('ROS_DOMAIN_ID', ros_domain_id)]
    )
    
    # 5. 启动核心节点（修正原脚本中的引号错误）
    core_node = Node(
        package='core',           # 原脚本中"核心节点“ ”core"存在引号错误，修正为正确包名
        executable='core_node',
        name='game_core',
        output='screen',
        environment=[('ROS_DOMAIN_ID', ros_domain_id)]
    )
    
    # 6. 发送游戏开始信号（对应脚本中的ros2 topic pub命令）
    start_signal_1 = ExecuteProcess(
        cmd=['ros2', 'topic', 'pub', '--once', '/game_start_player_1', 'std_msgs/Bool', 'data: true'],
        output='screen'
    )
    
    start_signal_2 = ExecuteProcess(
        cmd=['ros2', 'topic', 'pub', '--once', '/game_start_player_2', 'std_msgs/Bool', 'data: true'],
        output='screen'
    )
    
    # 组装所有启动动作
    return LaunchDescription([
        # 先启动所有节点
        image_node,
        move_node,
        shoot_node,
        model_node,
        core_node,
        # 节点启动后发送开始信号
        start_signal_1,
        start_signal_2
    ])