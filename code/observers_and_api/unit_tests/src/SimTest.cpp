/*
 * SimTest.cpp
 *
 *  Created on: Jun 16, 2014
 *      Author: cady
 */
#include <math.h> // isnan

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI

#include <boost/algorithm/string.hpp> // replace in string

#include <ssc/data_source.hpp>

#include "SimObserver.hpp"
#include "Sim.hpp"
#include "SimTest.hpp"
#include "YamlSimulatorInput.hpp"
#include "yaml_data.hpp"
#include "SimException.hpp"
#include "SimulatorYamlParser.hpp"
#include "STL_data.hpp"
#include "simulator_api.hpp"
#include <ssc/solver.hpp>
#include "TriMeshTestData.hpp"
#include "generate_anthineas.hpp"

#define EPS (1E-10)
#define SQUARE(x) ((x)*(x))
#define DEG (atan(1.)/45.)

const VectorOfVectorOfPoints SimTest::anthineas_stl = anthineas();

SimTest::SimTest() : a(ssc::random_data_generator::DataGenerator(42222))
{
}

SimTest::~SimTest()
{
}

void SimTest::SetUp()
{
}

void SimTest::TearDown()
{
}

TEST_F(SimTest, can_simulate_falling_ball)
{
    const size_t N = 10;
    const auto res = simulate<ssc::solver::EulerStepper>(test_data::falling_ball_example(), 0, N, 1);
    ASSERT_EQ(N+1, res.size());
    const double g = 9.81;
    for (size_t i = 0 ; i < N+1 ; ++i)
    {
        const double t = (double) i;
        ASSERT_EQ(13, res.at(i).x.size())                             << "Time step: i=" << i;
        ASSERT_DOUBLE_EQ(t,          res.at(i).t)                << "Time step: i=" << i;
        ASSERT_NEAR(4+1.*t,          res.at(i).x[XIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(8,               res.at(i).x[YIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(12+g*t*(t-1)/2., res.at(i).x[ZIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(1,               res.at(i).x[UIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(0,               res.at(i).x[VIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(g*t,             res.at(i).x[WIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(0,               res.at(i).x[PIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(0,               res.at(i).x[QIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(0,               res.at(i).x[RIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(1,               res.at(i).x[QRIDX(0)], EPS) << "Time step: i=" << i;
        ASSERT_NEAR(0,               res.at(i).x[QIIDX(0)], EPS) << "Time step: i=" << i;
        ASSERT_NEAR(0,               res.at(i).x[QJIDX(0)], EPS) << "Time step: i=" << i;
        ASSERT_NEAR(0,               res.at(i).x[QKIDX(0)], EPS) << "Time step: i=" << i;
    }
}

TEST_F(SimTest, can_simulate_oscillating_cube)
{
    const double dt = 1E-1;
    const double tend = 10;
    const auto res = simulate<ssc::solver::RK4Stepper>(test_data::oscillating_cube_example(), test_data::cube(), 0, tend, dt);

    const double g = 9.81;
    const double rho = 1026;
    const double L = 1;
    const double m = 1e3;
    const double omega = L*sqrt(rho*g/m);
    const double A = m/(rho*L*L)*(1-rho*L*L*L/(2*m));
    const double z0 = L/2;
    const double eps = 1E-3;
    const size_t N = (size_t)(floor(tend/dt+0.5))+1;
    ASSERT_EQ(N, res.size());
    for (size_t i = 0 ; i < N ; ++i)
    {
        const double t = (double)i*dt;
        ASSERT_EQ(13,                          res.at(i).x.size())         << "Time step: i=" << i;
        ASSERT_DOUBLE_EQ(t,                    res.at(i).t)                << "Time step: i=" << i;
        ASSERT_NEAR(0,                         res.at(i).x[XIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(0,                         res.at(i).x[YIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR((z0-A)*cos(omega*t)+A,     res.at(i).x[ZIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(0,                         res.at(i).x[UIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(0,                         res.at(i).x[VIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(omega*(A-z0)*sin(omega*t), res.at(i).x[WIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(0,                         res.at(i).x[PIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(0,                         res.at(i).x[QIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(0,                         res.at(i).x[RIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(1,                         res.at(i).x[QRIDX(0)], EPS) << "Time step: i=" << i;
        ASSERT_NEAR(0,                         res.at(i).x[QIIDX(0)], EPS) << "Time step: i=" << i;
        ASSERT_NEAR(0,                         res.at(i).x[QJIDX(0)], EPS) << "Time step: i=" << i;
        ASSERT_NEAR(0,                         res.at(i).x[QKIDX(0)], EPS) << "Time step: i=" << i;
    }
}

TEST_F(SimTest, can_simulate_stable_cube)
{
    const double dt = 1E-1;
    const double tend = 10;
    const auto res = simulate<ssc::solver::RK4Stepper>(test_data::stable_cube_example(), test_data::cube(), 0, tend, dt);

    const size_t N = (size_t)(floor(tend/dt+0.5))+1;
    const double rho = 1026;
    const double L = 1;
    const double m = 1e3;
    const double z0 = m/rho-L/2;
    const double eps = 1E-3;
    ASSERT_EQ(N, res.size());
    for (size_t i = 0 ; i < N ; ++i)
    {
        const double t = (double)i*dt;
        ASSERT_EQ(13,       res.at(i).x.size())         << "Time step: i=" << i;
        ASSERT_DOUBLE_EQ(t, res.at(i).t)                << "Time step: i=" << i;
        ASSERT_NEAR(0,      res.at(i).x[XIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(0,      res.at(i).x[YIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(z0,     res.at(i).x[ZIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(0,      res.at(i).x[UIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(0,      res.at(i).x[VIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(0,      res.at(i).x[WIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(0,      res.at(i).x[PIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(0,      res.at(i).x[QIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(0,      res.at(i).x[RIDX(0)], eps)  << "Time step: i=" << i;
        ASSERT_NEAR(1,      res.at(i).x[QRIDX(0)], EPS) << "Time step: i=" << i;
        ASSERT_NEAR(0,      res.at(i).x[QIIDX(0)], EPS) << "Time step: i=" << i;
        ASSERT_NEAR(0,      res.at(i).x[QJIDX(0)], EPS) << "Time step: i=" << i;
        ASSERT_NEAR(0,      res.at(i).x[QKIDX(0)], EPS) << "Time step: i=" << i;
    }
}

TEST_F(SimTest, initial_angle_should_not_change_results_for_falling_ball)
{
    const size_t N = 3;
    auto yaml = SimulatorYamlParser(test_data::falling_ball_example()).parse();
    yaml.bodies.front().initial_position_of_body_frame_relative_to_NED_projected_in_NED.angle.theta = 45*DEG;
    yaml.bodies.front().initial_velocity_of_body_frame_relative_to_NED_projected_in_body.u = 0;
    const auto res = simulate<ssc::solver::EulerStepper>(yaml, 0, N, 1);
    ASSERT_EQ(N+1, res.size());
    const double g = 9.81;
    for (size_t i = 0 ; i < N+1 ; ++i)
    {
        const double t = (double) i;
        const auto quaternion_norm = SQUARE(res.at(i).x[QRIDX(0)])
                                   + SQUARE(res.at(i).x[QIIDX(0)])
                                   + SQUARE(res.at(i).x[QJIDX(0)])
                                   + SQUARE(res.at(i).x[QKIDX(0)]);
        ASSERT_EQ(13, res.at(i).x.size())                        << "Time step: i=" << i;
        ASSERT_DOUBLE_EQ(t,          res.at(i).t)                << "Time step: i=" << i;
        ASSERT_NEAR(4,               res.at(i).x[XIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(8,               res.at(i).x[YIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(12+g*t*(t-1)/2., res.at(i).x[ZIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(0,               res.at(i).x[PIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(0,               res.at(i).x[QIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(0,               res.at(i).x[RIDX(0)], EPS)  << "Time step: i=" << i;
        ASSERT_NEAR(1,               quaternion_norm, EPS)       << "Time step: i=" << i;
    }
}

TEST_F(SimTest, hydrostatic_test_on_anthineas)
{
    const auto yaml = SimulatorYamlParser(test_data::anthineas_fast_hydrostatic_test()).parse();
    const auto res = simulate<ssc::solver::RK4Stepper>(yaml, anthineas_stl, 0, 4.79, 0.479);
    ASSERT_EQ(11, res.size());
    ASSERT_NEAR(0, res[0].x[XIDX(0)], EPS);
    ASSERT_NEAR(0, res[0].x[YIDX(0)], EPS);
    ASSERT_NEAR(-5, res[0].x[ZIDX(0)], EPS);

    for (size_t i = 0 ; i < 5 ; ++i)
    {
        ASSERT_NEAR(res[i].x[ZIDX(0)],res[10-i].x[ZIDX(0)], 0.1) << " i = " << i;
    }
    ASSERT_NEAR(3.84, res[5].x[ZIDX(0)], 1E-2);
    for (size_t i = 0 ; i < 11 ; ++i)
    {
        ASSERT_NEAR(0, fabs(res[i].x[XIDX(0)]), 0.2) << " i = " << i;
        ASSERT_NEAR(0, fabs(res[i].x[YIDX(0)]), 1E-2) << " i = " << i;
    }
}

TEST_F(SimTest, exact_hydrostatic_test_on_anthineas)
{
    const auto yaml = SimulatorYamlParser(test_data::anthineas_exact_hydrostatic_test()).parse();
    const auto res = simulate<ssc::solver::RK4Stepper>(yaml, anthineas_stl, 0, 0.1, 0.4);
}

TEST_F(SimTest, should_throw_if_wave_output_mesh_does_not_exist)
{
    auto input = SimulatorYamlParser(test_data::waves()).parse();
    boost::replace_all(input.environment[0].yaml, "frame of reference: NED", "frame of reference: foo");
    const Sim sys = get_system(input, 0);
    ASSERT_THROW(sys.get_waves(a.random<double>()),SimException);
}

TEST_F(SimTest, can_generate_wave_height_on_mesh)
{
    const Sim sys = get_system(test_data::waves(), 0);
    const double t = 9951191801.8523445;
    const std::vector<ssc::kinematics::Point> w = sys.get_waves(t);

    ASSERT_EQ(10, w.size());
    ASSERT_DOUBLE_EQ(1, (double)w[0].x());
    ASSERT_DOUBLE_EQ(1, (double)w[0].y());
    EXPECT_NEAR(5.3843063341769382, (double)w[0].z(), 1E-5);

    ASSERT_DOUBLE_EQ(2, (double)w[1].x());
    ASSERT_DOUBLE_EQ(1, (double)w[1].y());
    EXPECT_NEAR(5.3847660459188535, (double)w[1].z(), 1E-5);

    ASSERT_DOUBLE_EQ(3, (double)w[2].x());
    ASSERT_DOUBLE_EQ(1, (double)w[2].y());
    EXPECT_NEAR(5.3852257763390483, (double)w[2].z(), 1E-5);

    ASSERT_DOUBLE_EQ(4, (double)w[3].x());
    ASSERT_DOUBLE_EQ(1, (double)w[3].y());
    EXPECT_NEAR(5.3856852968972957, (double)w[3].z(), 1E-5);

    ASSERT_DOUBLE_EQ(5, (double)w[4].x());
    ASSERT_DOUBLE_EQ(1, (double)w[4].y());
    EXPECT_NEAR(5.3861450323603179, (double)w[4].z(), 1E-5);

    ASSERT_DOUBLE_EQ(1, (double)w[5].x());
    ASSERT_DOUBLE_EQ(2, (double)w[5].y());
    EXPECT_NEAR(5.314654694054445, (double)w[5].z(), 1E-5);

    ASSERT_DOUBLE_EQ(2, (double)w[6].x());
    ASSERT_DOUBLE_EQ(2, (double)w[6].y());
    EXPECT_NEAR(5.3151144417756662, (double)w[6].z(), 1E-5);

    ASSERT_DOUBLE_EQ(3, (double)w[7].x());
    ASSERT_DOUBLE_EQ(2, (double)w[7].y());
    EXPECT_NEAR(5.3155740110918153, (double)w[7].z(), 1E-5);

    ASSERT_DOUBLE_EQ(4, (double)w[8].x());
    ASSERT_DOUBLE_EQ(2, (double)w[8].y());
    EXPECT_NEAR(5.316033768387376, (double)w[8].z(), 1E-5);
}

TEST_F(SimTest, can_generate_wave_height_on_mesh_for_default_wave_model)
{
    const Sim sys = get_system(test_data::stable_rolling_cube_test(), test_data::cube(), 0);
    const std::vector<ssc::kinematics::Point> w = sys.get_waves(a.random<double>());
    ASSERT_EQ(200, w.size());
    for (size_t i = 0 ; i < 200 ; ++i)
    {
        ASSERT_DOUBLE_EQ(0, (double)w[i].z());
    }
}

TEST_F(SimTest, waves_test_on_anthineas)
{
    const auto yaml = SimulatorYamlParser(test_data::anthineas_waves_test()).parse();
    const auto res = simulate<ssc::solver::RK4Stepper>(yaml, anthineas_stl, 0, 4.79, 0.479);
    ASSERT_EQ(11, res.size());
    ASSERT_NEAR(0, res[0].x[XIDX(0)], EPS);
    ASSERT_NEAR(0, res[0].x[YIDX(0)], EPS);
    ASSERT_NEAR(-5, res[0].x[ZIDX(0)], EPS);
}


TEST_F(SimTest, froude_krylov)
{
    const auto yaml = SimulatorYamlParser(test_data::anthineas_froude_krylov()).parse();
    const auto res = simulate<ssc::solver::RK4Stepper>(yaml, anthineas_stl, 0, 4.79, 0.479);
}

TEST_F(SimTest, anthineas_damping)
{
    const auto yaml = SimulatorYamlParser(test_data::anthineas_damping()).parse();
    const auto res = simulate<ssc::solver::RK4Stepper>(yaml, anthineas_stl, 0, 20, 1);
    ASSERT_EQ(21, res.size());
    ASSERT_FALSE(std::isnan(res.back().x[ZIDX(0)]));
    ASSERT_EQ(res.back().x[ZIDX(0)],res.back().x[ZIDX(0)]); // Check if nan
}

TEST_F(SimTest, propulsion_and_resistance)
{
    const auto yaml = SimulatorYamlParser(test_data::propulsion_and_resistance()).parse();
    ssc::data_source::DataSource commands;
    commands.set<double>("propeller(rpm)", 100*(2*PI)/60.);
    commands.set<double>("propeller(P/D)", 1.064935);
    const size_t N = 250;
    const auto res = simulate<ssc::solver::EulerStepper>(yaml, anthineas_stl, 0, N, 1, commands);
    ASSERT_EQ(N+1, res.size());
    for (size_t i = 0 ; i <= N ; ++i)
    {
        const double t = res.at(i).t;
        ASSERT_LE(2.79963*(t-40), res.at(i).x[XIDX(0)]);
        ASSERT_DOUBLE_EQ(0, res.at(i).x[YIDX(0)]);
        ASSERT_DOUBLE_EQ(1, res.at(i).x[ZIDX(0)]);
        ASSERT_LE(2.79963*(1-exp(-0.012*t)), res.at(i).x[UIDX(0)]) << i;
        ASSERT_DOUBLE_EQ(0, res.at(i).x[VIDX(0)]);
        ASSERT_DOUBLE_EQ(0, res.at(i).x[WIDX(0)]);
        ASSERT_NEAR(-0.00109667*t, res.at(i).x[PIDX(0)], 8E-2);
        ASSERT_DOUBLE_EQ(0, res.at(i).x[QIDX(0)]);
        ASSERT_DOUBLE_EQ(0, res.at(i).x[RIDX(0)]);
    }
}

TEST_F(SimTest, bug_2641)
{
    const auto yaml = SimulatorYamlParser(test_data::bug_2641()).parse();
    ssc::data_source::DataSource commands;
    commands.set<double>("propeller(rpm)", 5*(2*PI));
    commands.set<double>("propeller(P/D)", 0.67);
    const auto res = simulate<ssc::solver::RK4Stepper>(yaml, anthineas_stl, 0, 5, 1, commands);
    ASSERT_LT(res.back().x[VIDX(0)], -0.003);
}
