# ROS2 学习任务一：Hello ROS2 !
----
### 文件结构： 
```
ROS
 ├── build
 ├── install
 ├── log
 └── src
      │──my_msgs
      │     ├──includee
      │     ├──msg/Msg1.msg
      │     ├──src
      │     ├──CMakeLists.txt
      │     └──package.xml
      │
      └──project1
            ├──build
            ├──include
            ├──install
            ├──log
            ├──msg/Msg1.msg
            ├──src
            │   ├──listener1.cpp
            │   ├──talker1.cpp
            │   └──talker1.md
            ├──CMakeLists.txt
            ├──package.xml
            └──REAEME.md
```
- #### 功能包  
    my_msgs——存放自定义消息
- #### 主要项目
    project1——实现`talker`节点与`listen`节点通讯
----
- ### 运行说明：
在ROS目录下运行：
```
    colcon build --packages-select my_msgs project1
    source install/setup.bash
    ros2 run project1 talker1_node
```
另一个终端运行：
```
    colcon build --packages-select my_msgs project1
    source install/setup.bash
    ros2 run project1 listener1_node
```
实现了运行中改变`talker`节点的消息发送频率：
```
    ros2 node list
    ros2 param set /talker publish_frequency 值
```