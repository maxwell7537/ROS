``` talker() : Node("talker"), count_(0) ```
- 构造函数初始化列表：
- 调用父类 Node 的构造函数，将节点名称设为 "talker" ；
- 初始化成员变量 count_ 为 0 （用于消息计数）。
---
```this->declare_parameter<double>("publish_frequency", 10.0);```
- 在节点中声明一个双精度浮点型参数 publish_frequency ，默认值为 10.0 。
- ROS2的参数系统支持运行时修改参数，此操作让节点具备“可配置发布频率”的能力。
---
```
publisher_ = this->create_publisher<my_msgs::msg::Msg1>("topic", 10);
```
- 创建一个发布者：
- 发布的消息类型是自定义的 my_msgs::msg::Msg1 （需提前在 .msg 文件中定义，包含 num （数值）和 s （字符串）字段）；
- 话题名称为 "topic" ；
- 队列大小为 10 （当订阅者处理滞后时，最多缓存10条消息，超出则丢弃旧消息）。
---
```
timer_ = this->create_wall_timer(chrono::milliseconds(100), std::bind(&talker::timer_callback, this));
```
- 创建一个壁钟定时器（wall timer，基于系统实际时间触发）：
- 周期为 100ms （0.1秒）；
- 通过 std::bind 将回调函数绑定到 talker 类的 timer_callback 成员函数， this 指针确保回调能访问类的成员变量。
-----
```
param_callback_handle_ = this->add_on_set_parameters_callback(std::bind(&talker::parameters_callback, this, std::placeholders::_1));
  ```
- 添加参数变更回调函数：
- 当节点的参数（如 publish_frequency ）被修改时，触发 talker::parameters_callback 函数；
-  std::placeholders::_1 表示回调的参数由系统自动传入（包含参数变更的详细信息）。
---
`RCLCPP_INFO(this->get_logger(), "发布者节点已启动");`
- 使用ROS2的日志系统输出“信息级”日志；
-  this->get_logger() 获取节点的日志器，消息内容为“发布者节点已启动”，用于提示节点启动状态。
----
### parameters_callback  函数（参数变更回调）

```
rcl_interfaces::msg::SetParametersResult parameters_callback(const vector<rclcpp::Parameter> &parameters){  
  rcl_interfaces::msg::SetParametersResult result;    
  result.successful = true;    
  for (const auto &param : parameters){    
    double new_frequency = param.as_double();    
    update_timer_frequency();    
    RCLCPP_INFO(this->get_logger(), "发布频率已更新为: %.2f   Hz", new_frequency);  
  }  
  return result;  
}
```
 
- 返回类型： rcl_interfaces::msg::SetParametersResult ，ROS2用于反馈参数设置结果的消息类型。  
- 形参： const vector<rclcpp::Parameter> &parameters ，存储所有被修改的参数列表。  
-  rcl_interfaces::msg::SetParametersResult result; ：定义参数设置结果对象。  
-  result.successful = true; ：初始化结果为“设置成功”。  
-  for (const auto &param : parameters) ：遍历所有被修改的参数。  
-  double new_frequency = param.as_double(); ：将参数转换为双精度浮点型，获取新的发布频率。  
-  update_timer_frequency(); ：调用函数更新定时器的触发周期。  
-  RCLCPP_INFO(this->get_logger(), "发布频率已更新为: %.2f Hz", new_frequency); ：输出信息级日志，提示频率更新后的值。  
-  return result; ：返回参数设置结果。  
----
### update_timer_frequency  函数（更新定时器频率）
  
```
void update_timer_frequency(){
  double frequency = this->get_parameter("publish_frequency").as_double();
  timer_->cancel();// 取消现有定时器
  auto period = chrono::duration_cast<chrono::milliseconds>(1.0s / frequency);
  timer_ = this->create_wall_timer(period, bind(&talker::timer_callback, this));// 创建新的定时器
}
```

- 功能：根据新的 publish_frequency 参数，重新设置定时器周期。
-  double frequency = this->get_parameter("publish_frequency").as_double(); ：获取当前 publish_frequency 参数的数值。
-  timer_->cancel(); ：取消当前定时器，为创建新定时器做准备。
-  auto period = chrono::duration_cast<chrono::milliseconds>(1.0s / frequency); ：计算新周期（ 1.0s 除以频率得到周期秒数，再转换为毫秒）。这里依赖 using namespace std::chrono_literals; ，使 1.0s 可直接表示“1秒”。
-  timer_ = this->create_wall_timer(period, bind(&talker::timer_callback, this)); ：创建新的壁钟定时器（基于系统实际时间触发），周期为 period ，并将 timer_callback 函数绑定到当前 talker 类实例（ this ），确保回调能访问类成员。
 ----
### timer_callback  函数（定时器回调，周期性发布消息）

  
```
void timer_callback(){
  auto message = my_msgs::msg::Msg1();
  message.num = count_;
  message.s = "Hello, ROS2: " + to_string(count_++);  
  RCLCPP_INFO(this->get_logger(), "发布: '%s'", message.s.c_str());
  publisher_->publish(message); 
}
```

- 功能：每到定时器周期就发布一条自定义消息。
-  auto message = my_msgs::msg::Msg1(); ：创建自定义消息类型 Msg1 的实例 message （需提前在 .msg 文件中定义 Msg1 ，包含 num （数值）和 s （字符串）字段）。
-  message.num = count_; ：给消息的 num 字段赋值为当前 count_ 的值。
-  message.s = "Hello, ROS2: " + to_string(count_++); ：给消息的 s 字段赋值为字符串拼接结果，随后 count_ 自增1。
-  RCLCPP_INFO(this->get_logger(), "发布: '%s'", message.s.c_str()); ：输出信息级日志，显示发布的消息内容。
-  publisher_->publish(message); ：通过发布者 publisher_ 将消息发布到指定话题。
 ----
### 私有成员变量

```
rclcpp::TimerBase::SharedPtr timer_;
rclcpp::Publisher<my_msgs::msg::Msg1>::SharedPtr publisher_;
rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_callback_handle_;
```
-  rclcpp::TimerBase::SharedPtr timer_; ：定时器的智能指针，自动管理定时器的生命周期（避免内存泄漏），支持取消、重启等操作。
-  rclcpp::Publisher<my_msgs::msg::Msg1>::SharedPtr publisher_; ：消息发布者的智能指针，负责将 Msg1 类型的消息发布到话题。
-  rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_callback_handle_; ：参数回调句柄的智能指针，管理“参数变更回调”的生命周期。  
----
```
int main(int argc, char * argv[]){
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<talker>());
  rclcpp::shutdown();
  return 0;
}
```
- 程序入口函数：
-  rclcpp::init(argc, argv) ：初始化ROS2上下文，解析命令行参数等；
-  rclcpp::spin(std::make_shared<talker>()) ：创建 talker 类的共享指针实例，进入自旋循环（使节点保持运行，持续处理回调如定时器、参数变更等）；
-  rclcpp::shutdown() ：关闭ROS2上下文，释放资源；
