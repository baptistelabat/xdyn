/*
 * Sim.cpp
 *
 *  Created on: Jun 16, 2014
 *      Author: cady
 */

#include "coriolis_and_centripetal.hpp"
#include "DataSource.hpp"
#include "hydrostatic.hpp"
#include "Kinematics.hpp"
#include "Mesh.hpp"
#include "RotationMatrix.hpp"
#include "Sim.hpp"
#include "Transform.hpp"
#include "update_kinematics.hpp"
#include "WaveModelInterface.hpp"

Sim::Sim(const std::vector<Body>& bodies_,
         const std::vector<ListOfForces>& forces_,
         const KinematicsPtr& k_,
         const StateType& x) :
         state(x), bodies(bodies_), forces(forces_), k(k_),
         _dx_dt(StateType(x.size(),0))
{
}

void Sim::operator()(const StateType& x, StateType& dx_dt, double )
{
    for (size_t i = 0 ; i < bodies.size() ; ++i)
    {
        update_kinematics(x, bodies[i], i, k);
        calculate_state_derivatives(sum_of_forces(x, i), bodies[i].inverse_of_the_total_inertia, x, dx_dt, i);
    }
    state = x;
    _dx_dt = dx_dt;
}

void Sim::update_discrete_states()
{
}

void Sim::update_continuous_states()
{
}

StateType Sim::get_state_derivatives() const
{
    return _dx_dt;
}

std::vector<std::string> Sim::get_names_of_bodies() const
{
    std::vector<std::string> ret;
    for (const auto body:bodies) ret.push_back(body.name);
    return ret;
}

UnsafeWrench Sim::sum_of_forces(const StateType& x, const size_t body) const
{
    const Eigen::Vector3d uvw_in_body_frame = Eigen::Vector3d::Map(_U(x,body));
    const Eigen::Vector3d pqr = Eigen::Vector3d::Map(_P(x,body));
    UnsafeWrench S(coriolis_and_centripetal(bodies[body].G,bodies[body].solid_body_inertia.get(),uvw_in_body_frame, pqr));
    for (auto F:forces[body]){
        S += (*F)(bodies[body]);
    }
    return S;
}

void Sim::calculate_state_derivatives(const Wrench& sum_of_forces,
                                      const MatrixPtr& inverse_of_the_total_inertia,
                                      const StateType& x,
                                      StateType& dx_dt,
                                      const size_t i) const
{
    // du/dt, dv/dt, dw/dt, dp/dt, dq/dt, dr/dt
    Eigen::Matrix<double,6,1> dXdt = Eigen::Matrix<double,6,1>::Map(_U(dx_dt,i));
    dXdt = inverse_of_the_total_inertia->operator*(sum_of_forces.to_vector());

    *_U(dx_dt,i) = dXdt(0);
    *_V(dx_dt,i) = dXdt(1);
    *_W(dx_dt,i) = dXdt(2);
    *_P(dx_dt,i) = dXdt(3);
    *_Q(dx_dt,i) = dXdt(4);
    *_R(dx_dt,i) = dXdt(5);

    // dx/dt, dy/dt, dz/dt
    const RotationMatrix R = k->get(bodies[i].name, "NED").get_rot();
    const Eigen::Vector3d uvw_in_body_frame = Eigen::Vector3d::Map(_U(x,i));
    const Eigen::Vector3d uvw_in_ned_frame(R*uvw_in_body_frame);
    *_X(dx_dt,i) = uvw_in_ned_frame(0);
    *_Y(dx_dt,i) = uvw_in_ned_frame(1);
    *_Z(dx_dt,i) = uvw_in_ned_frame(2);

    // dqr/dt, dqi/dt, dqj/dt, dqk/dt
    const Eigen::Quaternion<double> q1(*_QR(x,i),
                                       *_QI(x,i),
                                       *_QJ(x,i),
                                       *_QK(x,i));
    const Eigen::Quaternion<double> q2(0,*_P(x,i),*_Q(x,i),*_R(x,i));
    const Eigen::Quaternion<double> dq_dt(Eigen::Quaternion<double>(q1*q2));
    *_QR(dx_dt,i) = 0.5*(double)dq_dt.w();
    *_QI(dx_dt,i) = 0.5*(double)dq_dt.x();
    *_QJ(dx_dt,i) = 0.5*(double)dq_dt.y();
    *_QK(dx_dt,i) = 0.5*(double)dq_dt.z();
}
