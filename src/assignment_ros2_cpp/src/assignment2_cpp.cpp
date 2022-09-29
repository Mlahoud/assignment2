// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <cinttypes>
#include <memory>
#include "turtlesim/msg/pose.hpp"
#include "turtlesim/srv/kill.hpp"
#include "turtlesim/srv/spawn.hpp"
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

using namespace std::chrono_literals;
using Kill = turtlesim::srv::Kill;
using Spawn = turtlesim::srv::Spawn;

class MyController : public rclcpp::Node
{
public:
  
  MyController()
  : Node("my_controller")
  {
    // init the publisher
    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/turtle2/cmd_vel", 1);
    
    // init the subscriber    
    subscription_ = this->create_subscription<turtlesim::msg::Pose>("/turtle2/pose", 10, std::bind(&MyController::pose_callback, this, std::placeholders::_1));
    
    // init the kill client
    client1_ = this->create_client<Kill>("kill");
    while (!client1_->wait_for_service(std::chrono::seconds(1))){
     if (!rclcpp::ok()) {
      RCLCPP_ERROR(this->get_logger(), "client interrupted while waiting for service to appear.");
      return;
    }
    RCLCPP_INFO(this->get_logger(), "waiting for service to appear...");
    }

    // init the spawn client
    client2_ = this->create_client<Spawn>("spawn");
    while (!client2_->wait_for_service(std::chrono::seconds(1))){
     if (!rclcpp::ok()) {
      RCLCPP_ERROR(this->get_logger(), "client interrupted while waiting for service to appear.");
      return;
    }
    RCLCPP_INFO(this->get_logger(), "waiting for service to appear...");
  }
  this->request1_ = std::make_shared<Kill::Request>();
  this->response1_ = std::make_shared<Kill::Response>();
  this->request2_ = std::make_shared<Spawn::Request>();
  this->response2_ = std::make_shared<Spawn::Response>();
  }
  
  void call_server1()
  {
      auto result_future = client1_->async_send_request(request1_);
      if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future) != rclcpp::FutureReturnCode::SUCCESS)
      {
        RCLCPP_ERROR(this->get_logger(), "service call failed :(");
      }
      this->response1_=result_future.get();
  }
  
  void call_server2()
  {
      auto result_future = client2_->async_send_request(request2_);
      if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future) != rclcpp::FutureReturnCode::SUCCESS)
      {
        RCLCPP_ERROR(this->get_logger(), "service call failed :(");
      }
      this->response2_=result_future.get();
  }
  std::shared_ptr<Kill::Request> request1_;
  std::shared_ptr<Kill::Response> response1_;
  std::shared_ptr<Spawn::Request> request2_;
  std::shared_ptr<Spawn::Response> response2_;
  
private:
  void pose_callback(const turtlesim::msg::Pose::SharedPtr msg) const
  {
    geometry_msgs::msg::Twist my_twist; 

    RCLCPP_INFO(this->get_logger(), "Turtlebot pose is: x: "+ std::to_string(msg->x) + " y: " + std::to_string(msg->y) + " theta: " + std::to_string(msg->theta));
    if (msg->x > 9.0){
            my_twist.angular.z = 4.0;
        } else if (msg->x < 2.0){
            my_twist.angular.z = -4.0;
        } else {
            my_twist.angular.z = 0.0;
        }
	    my_twist.linear.x = 1.0;
	    my_twist.linear.y = 0.0;
	    publisher_->publish(my_twist);

  }
  rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr subscription_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::Client<Kill>::SharedPtr client1_; 
  rclcpp::Client<Spawn>::SharedPtr client2_; 
    
};

int main(int argc, char * argv[])
{

  rclcpp::init(argc, argv);
  auto node = std::make_shared<MyController>();
  node->request1_->name = "turtle1";

  node->call_server1();
  
  node->request2_->name = "turtle2";
  node->request2_->x = 2.0;
  node->request2_->y = 1.0;
  node->request2_->theta = 0.0;
  node->call_server2();
  

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}

