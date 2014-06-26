/*
 * update_kinematics.hpp
 *
 *  Created on: Jun 16, 2014
 *      Author: cady
 */

#ifndef UPDATE_KINEMATICS_HPP_
#define UPDATE_KINEMATICS_HPP_

#include "Point.hpp"
#include "RotationMatrix.hpp"
#include "StateMacros.hpp"
#include "Transform.hpp"

class Body;
class YamlBody;
class YamlRotation;

#include "tr1_macros.hpp"
#include TR1INC(memory)

class Kinematics;
typedef TR1(shared_ptr)<Kinematics> KinematicsPtr;

Point get_origin(const StateType& x, const size_t i);
RotationMatrix get_rot_from_ned_to(const StateType& x, const size_t body);
Point get_position_of_body_relative_to_mesh(const Body& body);
kinematics::Transform get_transform_from_mesh_to(const Body& body);
kinematics::Transform get_transform_from_ned_to(const StateType& x, const Body& body, const size_t idx);
void update_kinematics(const StateType& x, const Body& body, const size_t i, const KinematicsPtr& k);
StateType get_initial_states(const YamlRotation& convention, const std::vector<YamlBody>& bodies);

#endif /* UPDATE_KINEMATICS_HPP_ */
