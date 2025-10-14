# ROS2 
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
    colcon build --packages-select core shoot move image
    source install/setup.bash
    ros2 run core core_node
    ros2 run shoot shoot_node
    ros2 run move move_node
    ros2 run image image_node
```
运行中改变节点的消息发送频率：
```
    ros2 node list
    ros2 param set /talker publish_frequency 值
```

------
- 2025.10.11
加入了额外的cv_bridge子模块  
在其他地方克隆包含子模块的仓库时，需要运行  
`git submodule update --init --recursive`  
来获取并检出子模块的文件。  
增加了talker和listener发送和接收图片的功能  
修改了ros2节点连接的opencv版本--改为自定义的4.7.0版本而不是系统的4.5版本  
解决了highgui：：imshow版本冲突的问题  

------
- 2025.10.12
加入了core与image模块  
image调用x11实现从屏幕截取图像并转换为opencv的格式发布  
core实现简单的打印收到的图像功能  
## 思路1：x
想让image节点直接对截取的图像进行处理直接传出识别的类别及甲板数据  
    core负责调用move和shoot进行决策功能
    move负责将二维坐标的移动实现
    shoot负责将对二维图像的点进行三维解算并实现射击

## 思路2：v  
image直接传出图片数据，在让各个节点各自识别所需的数据

选型推荐：优先选思路 2  
Robomaster 模拟比赛中，“快速迭代调试”（比如临时修改射击阈值、调整路径识别规则）是核心需求，且现代 CPU（如你的 Legion Y9000P）处理轻量级图像识别（如 OpenCV 模板匹配、YOLOv5s）的重复计算压力不大，带宽也能承载 640×480 图像的传输（约 900KB / 帧 ×30FPS=27MB/s，远低于 ROS2 的通信上限）。  
思路 2 的模块化优势能显著降低比赛中的调试成本，且后续新增功能（如加入 “避障识别”）只需新增一个节点，无需改动现有逻辑，灵活性更高。

方案分为感知层（Image 节点）、决策层（Core 节点）、执行层（Move/Shoot 节点） 三层，配合 Unity 模拟器实现闭环控制。

core 用 有限状态机（FSM）决定下一步动作（进攻 / 防守 / 占领甲板 / 避障）

闭环控制：

    Move 和 Shoot 用 ROS2 的Action通信（而非Topic），Core 可获取 “执行进度”（如 Move 是否在途中、Shoot 是否正在瞄准），避免指令冲突

参数化配置：

    将识别阈值（如 YOLO 置信度）、PID 参数、目标坐标等设为 ROS2 参数（declare_parameter），比赛中可通过ros2 param set实时调整，无需重新编译