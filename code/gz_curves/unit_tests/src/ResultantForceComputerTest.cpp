/*
 * ResultantForceComputerTest.cpp
 *
 *  Created on: Feb 9, 2015
 *      Author: cady
 */

#include <set>

#include "make_sim_for_GZ.hpp"
#include "ResultantForceComputerTest.hpp"
#include "ResultantForceComputer.hpp"
#include "SurfaceElevationInterface.hpp"
#include "GZTypes.hpp"
#include "STL_data.hpp"
#include "yaml_data.hpp"

#define EPS (1E-10)

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI

Sim ResultantForceComputerTest::sim = GZ::make_sim(test_data::oscillating_cube_example(), test_data::cube());

ResultantForceComputerTest::ResultantForceComputerTest() : a(ssc::random_data_generator::DataGenerator(87878899))
{
}

ResultantForceComputerTest::~ResultantForceComputerTest()
{
}

void ResultantForceComputerTest::SetUp()
{
}

void ResultantForceComputerTest::TearDown()
{
}

TEST_F(ResultantForceComputerTest, sim_only_contains_gravity_and_hydrostatic_forces)
{
    const auto s = GZ::make_sim(test_data::stable_cube_example(), test_data::cube());

    std::map<std::string,std::vector<ForcePtr> > forces = s.get_forces();
    std::set<std::string> force_models;
    for (auto force:forces["cube"])
    {
        force_models.insert(force->get_name());
    }
    ASSERT_EQ(2, force_models.size());
    const bool has_gravity = force_models.find("gravity") != force_models.end();
    const bool has_hydrostatic = force_models.find("hydrostatic") != force_models.end();
    ASSERT_TRUE(has_gravity);
    ASSERT_TRUE(has_hydrostatic);
}

TEST_F(ResultantForceComputerTest, sim_only_contains_default_surface_elevation)
{
    const auto s = GZ::make_sim(test_data::cube_in_waves(), test_data::cube());
    EnvironmentAndFrames env = s.get_env();
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        const double x = a.random<double>().between(-1000,1000);
        const double y = a.random<double>().between(-1000,1000);
        const double t = a.random<double>().between(0,1000);
        ASSERT_DOUBLE_EQ(0, env.w->wave_height(x,y,t));
    }
}

TEST_F(ResultantForceComputerTest, can_compute_resultant_force_for_a_cube)
{
    GZ::ResultantForceComputer sum_of_forces(sim);
    const auto F0 = sum_of_forces(GZ::State(0,0,0));
    ASSERT_NEAR(1000*9.81-1026*0.5*9.81, F0.z,EPS);
    ASSERT_NEAR(0, F0.phi,EPS);
    ASSERT_NEAR(0, F0.theta,EPS);
    const auto F1 = sum_of_forces(GZ::State(2,0,0));
    ASSERT_NEAR(1000*9.81-1026*1.*9.81, F1.z,EPS);
    ASSERT_NEAR(0, F1.phi,EPS);
    ASSERT_NEAR(0, F1.theta,EPS);
}

TEST_F(ResultantForceComputerTest, can_compute_torque_for_a_cube_rotated_on_phi)
{
    GZ::ResultantForceComputer sum_of_forces(sim);
    const auto F = sum_of_forces(GZ::State(0,PI/3,0));
    ASSERT_NEAR((1000*9.81-1026*0.5*9.81)*cos(PI/3), F.z,EPS);
    EXPECT_NEAR(-1026*9.81*0.5*(2./9.*cos(PI/3.)-0.096225044864937658851*sin(PI/3.)), F.phi,EPS);
    EXPECT_NEAR(0, F.theta,EPS);
}

TEST_F(ResultantForceComputerTest, can_compute_torque_for_a_cube_rotated_on_theta)
{
    GZ::ResultantForceComputer sum_of_forces(sim);
    const auto F = sum_of_forces(GZ::State(0,0,PI/3));
    ASSERT_NEAR((1000*9.81-1026*0.5*9.81)*cos(PI/3), F.z,EPS);
    EXPECT_NEAR(0, F.phi,EPS);
    EXPECT_NEAR(-1026*9.81*0.5*(2./9.*cos(PI/3.)-0.096225044864937658851*sin(PI/3.)), F.theta,EPS);
}

TEST_F(ResultantForceComputerTest, can_compute_torque_for_a_cube_rotated_by_a_quarter)
{
    GZ::ResultantForceComputer sum_of_forces(sim);
    const auto F = sum_of_forces(GZ::State(0,PI/4,0));
    ASSERT_NEAR((1000*9.81-1026*0.5*9.81)*cos(PI/4), F.z,EPS);
    ASSERT_NEAR(0, F.phi,EPS);
    ASSERT_NEAR(0, F.theta,EPS);
}

TEST_F(ResultantForceComputerTest, can_compute_torque_for_a_cube_at_the_surface)
{
    GZ::ResultantForceComputer sum_of_forces(sim);
    const auto F = sum_of_forces(GZ::State(-0.5,0,0));
    ASSERT_NEAR(1000*9.81, F.z,EPS);
    ASSERT_NEAR(0, F.phi,EPS);
    ASSERT_NEAR(0, F.theta,EPS);
}

TEST_F(ResultantForceComputerTest, can_compute_resultant_for_any_angle)
{
    GZ::ResultantForceComputer sum_of_forces(sim);
    const size_t n = 1;//100;
    for (size_t i = 0 ; i < n ; ++i)
    {
        const double angle = -PI + (2*PI)*(double)(i)/(std::max(1.,(double)n - 1));
        const auto F1 = sum_of_forces(GZ::State(0,angle,0));
        ASSERT_NEAR((1000*9.81-1026*0.5*9.81)*cos(angle), F1.z,EPS);
        const auto F2 = sum_of_forces(GZ::State(0,0,angle));
        ASSERT_NEAR((1000*9.81-1026*0.5*9.81)*cos(angle), F2.z,EPS);
    }

}