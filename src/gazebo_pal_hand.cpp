/*
 *  gazebo_pal_hand.cpp
 *  Copyright (c) 2014 PAL Robotics sl. All Rights Reserved
 *  Created on: 7 Jan 2014
 *      Author: luca
 *
 * \brief A plugin for gazebo for controlling the pal underactuated hand in simulation
 * \author  Luca Marchionni (luca.marchionni@pal-robotics.com)
 */

#include <algorithm>
#include <assert.h>

#include <pal_gazebo_plugins/gazebo_pal_hand.h>

#include <gazebo/math/gzmath.hh>
#include <sdf/sdf.hh>

#include <ros/ros.h>

namespace gazebo {

  GazeboPalHand::GazeboPalHand() {}

  // Destructor
  GazeboPalHand::~GazeboPalHand() {
  }

  // Load the controller
  void GazeboPalHand::Load(physics::ModelPtr _parent, sdf::ElementPtr _sdf) {

    this->parent = _parent;
    this->world = _parent->GetWorld();

    this->robot_namespace_ = "";
    if (!_sdf->HasElement("robotNamespace")) {
      ROS_INFO("GazeboPalHand Plugin missing <robotNamespace>, defaults to \"%s\"",
          this->robot_namespace_.c_str());
    } else {
      this->robot_namespace_ =
        _sdf->GetElement("robotNamespace")->Get<std::string>() + "/";
    }

    this->finger_joint_name_ = "actuated_finger_joint";
    if (!_sdf->HasElement("actuatedJoint")) {
      ROS_WARN("GazeboPalHand Plugin (ns = %s) missing <actuatedJoint>, defaults to \"%s\"",
          this->robot_namespace_.c_str(), this->finger_joint_name_.c_str());
    } else {
      this->finger_joint_name_ = _sdf->GetElement("actuatedJoint")->Get<std::string>();
    }

    this->finger_1_joint_name_ = "finger_joint_1";
    if (!_sdf->HasElement("fingerJoint1")) {
      ROS_WARN("GazeboPalHand Plugin (ns = %s) missing <fingerJoint1>, defaults to \"%s\"",
          this->robot_namespace_.c_str(), this->finger_1_joint_name_.c_str());
    } else {
      this->finger_1_joint_name_ = _sdf->GetElement("fingerJoint1")->Get<std::string>();
    }

    this->finger_2_joint_name_ = "finger_joint_2";
    if (!_sdf->HasElement("fingerJoint1")) {
      ROS_WARN("GazeboPalHand Plugin (ns = %s) missing <fingerJoint2>, defaults to \"%s\"",
          this->robot_namespace_.c_str(), this->finger_2_joint_name_.c_str());
    } else {
      this->finger_2_joint_name_ = _sdf->GetElement("fingerJoint2")->Get<std::string>();
    }

    this->finger_3_joint_name_ = "finger_joint_3";
    if (!_sdf->HasElement("fingerJoint3")) {
      ROS_WARN("GazeboPalHand Plugin (ns = %s) missing <fingerJoint3>, defaults to \"%s\"",
          this->robot_namespace_.c_str(), this->finger_3_joint_name_.c_str());
    } else {
      this->finger_3_joint_name_ = _sdf->GetElement("fingerJoint3")->Get<std::string>();
    }

    physics::Joint_V all_joints = parent->GetJoints();
    for(unsigned int i=0; i < all_joints.size(); ++i)
    {
      ROS_INFO("Joint name %s", all_joints[i]->GetName().c_str());
    }
    joints[0] = this->parent->GetJoint(finger_joint_name_);
    joints[1] = this->parent->GetJoint(finger_1_joint_name_);
    joints[2] = this->parent->GetJoint(finger_2_joint_name_);
    joints[3] = this->parent->GetJoint(finger_3_joint_name_);

    if (!joints[0]) {
      char error[200];
      snprintf(error, 200,
          "GazeboPalHand Plugin (ns = %s) couldn't get actuated finger hinge joint named \"%s\"",
          this->robot_namespace_.c_str(), this->finger_joint_name_.c_str());
      gzthrow(error);
    }
    if (!joints[1]) {
      char error[200];
      snprintf(error, 200,
          "GazeboPalHand Plugin (ns = %s) couldn't get  finger 1 hinge joint named \"%s\"",
          this->robot_namespace_.c_str(), this->finger_1_joint_name_.c_str());
      gzthrow(error);
    }

    if (!joints[2]) {
      char error[200];
      snprintf(error, 200,
          "GazeboPalHand Plugin (ns = %s) couldn't get  finger 2 hinge joint named \"%s\"",
          this->robot_namespace_.c_str(), this->finger_2_joint_name_.c_str());
      gzthrow(error);
    }

    if (!joints[3]) {
      char error[200];
      snprintf(error, 200,
          "GazeboPalHand Plugin (ns = %s) couldn't get  finger 3 hinge joint named \"%s\"",
          this->robot_namespace_.c_str(), this->finger_3_joint_name_.c_str());
      gzthrow(error);
    }

    // listen to the update event (broadcast every simulation iteration)
    this->update_connection_ =
      event::Events::ConnectWorldUpdateBegin(
          boost::bind(&GazeboPalHand::UpdateChild, this));

  }

  bool forceGreaterThan(physics::JointWrench const &wrench, double threshold)
  {
    if(fabs(wrench.body1Force.x) > threshold || fabs(wrench.body1Force.y) > threshold ||  fabs(wrench.body1Force.z) > threshold ||
    fabs(wrench.body2Force.x) > threshold || fabs(wrench.body2Force.y) > threshold ||  fabs(wrench.body2Force.z) > threshold )
      return true;

    return false;
  }

  // Update the controller
  void GazeboPalHand::UpdateChild() {

      math::Angle actuator_angle = joints[0]->GetAngle(0u);
      math::Angle lower_limit    = math::Angle(0.02);
      if( actuator_angle > lower_limit)
      {

        math::Angle index_1_angle = ( actuator_angle/2.5 > joints[1]->GetUpperLimit(0u) ) ? joints[1]->GetUpperLimit(0u) : actuator_angle/2.5;
        joints[1]->SetAngle(0u, index_1_angle);

        math::Angle index_2_angle = ( actuator_angle/3.2 > joints[2]->GetUpperLimit(0u) ) ? joints[2]->GetUpperLimit(0u) : actuator_angle/3.2;
        joints[2]->SetAngle(0u, index_2_angle);

        math::Angle index_3_angle = ( actuator_angle/3.2 > joints[3]->GetUpperLimit(0u) ) ? joints[3]->GetUpperLimit(0u) : actuator_angle/3.2;
        joints[3]->SetAngle(0u, index_3_angle);

        }

      else
      {
        joints[1]->SetAngle(0u, lower_limit);
        joints[2]->SetAngle(0u, lower_limit);
        joints[3]->SetAngle(0u, lower_limit);
      }

  }

  GZ_REGISTER_MODEL_PLUGIN(GazeboPalHand)
}


