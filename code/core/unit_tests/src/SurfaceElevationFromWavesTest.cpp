/*
 * WaveModelFromSpectraTest.cpp
 *
 *  Created on: Aug 7, 2014
 *      Author: cady
 */

#include "discretize.hpp"
#include "DiracDirectionalSpreading.hpp"
#include "DiracSpectralDensity.hpp"
#include <ssc/kinematics.hpp>
#include "SurfaceElevationFromWavesTest.hpp"
#include "SurfaceElevationFromWaves.hpp"

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI

SurfaceElevationFromWavesTest::SurfaceElevationFromWavesTest() : a(ssc::random_data_generator::DataGenerator(45454))
{
}

SurfaceElevationFromWavesTest::~SurfaceElevationFromWavesTest()
{
}

void SurfaceElevationFromWavesTest::SetUp()
{
}

void SurfaceElevationFromWavesTest::TearDown()
{
}

TR1(shared_ptr)<WaveModel> SurfaceElevationFromWavesTest::get_model(const size_t nfreq) const
{
    const double psi0 = PI/4;
    const double Hs = 3;
    const double Tp = 10;
    const double omega0 = 2*PI/Tp;
    const double omega_min = a.random<double>().greater_than(0);
    const double omega_max = a.random<double>().greater_than(omega_min);
    const DiscreteDirectionalWaveSpectrum A = discretize(DiracSpectralDensity(omega0, Hs), DiracDirectionalSpreading(psi0), omega_min, omega_max, nfreq);
    int random_seed = 0;
    return TR1(shared_ptr)<WaveModel>(new Airy(A, random_seed));
}

TR1(shared_ptr)<WaveModel> SurfaceElevationFromWavesTest::get_model() const
{
    return get_model(a.random<size_t>().between(2,100));
}

TEST_F(SurfaceElevationFromWavesTest, default_constructor_contains_an_empty_output_wave_mesh)
{
    const auto ms = SurfaceElevationFromWaves(get_model()).get_output_mesh_size();
    ASSERT_EQ(0,ms.first);
    ASSERT_EQ(0,ms.second);
}

TEST_F(SurfaceElevationFromWavesTest, relative_wave_height)
{
//! [SurfaceElevationFromWavesTest relative_wave_height example]
    const double psi0 = PI/4;
    const double Hs = 3;
    const double Tp = 10;
    const double k_ = 4.*PI*PI/Tp/Tp/9.81;
    TR1(shared_ptr)<ssc::kinematics::Kinematics> k(new ssc::kinematics::Kinematics());
    SurfaceElevationFromWaves wave(get_model());
    const double phi = 3.4482969340598712549;
//! [SurfaceElevationFromWavesTest relative_wave_height example]
//! [SurfaceElevationFromWavesTest relative_wave_height expected output]
    for (double t = 0 ; t < 30 ; t+=0.1)
    {
        const ssc::kinematics::Point P("NED", a.random<double>(), a.random<double>(), a.random<double>());
        const double x = P.x();
        const double y = P.y();
        ASSERT_NEAR(Hs/2*cos(2*PI/Tp*t - k_*(x*cos(psi0)+y*sin(psi0)) +phi), wave.wave_height(x, y, t), 1E-5);
    }
//! [SurfaceElevationFromWavesTest relative_wave_height expected output]
}

TEST_F(SurfaceElevationFromWavesTest, dynamic_pressure)
{
//! [SurfaceElevationFromWavesTest dynamic_pressure example]
    const double psi0 = PI/4;
    const double Hs = 3;
    const double Tp = 10;
    const double k_ = 4.*PI*PI/Tp/Tp/9.81;
    TR1(shared_ptr)<ssc::kinematics::Kinematics> k(new ssc::kinematics::Kinematics());
    SurfaceElevationFromWaves wave(get_model());
    const double phi = 3.4482969340598712549;
//! [SurfaceElevationFromWavesTest dynamic_pressure example]
//! [SurfaceElevationFromWavesTest dynamic_pressure expected output]
    for (double t = 0 ; t < 30 ; t+=0.1)
    {
        const ssc::kinematics::Point P("NED", a.random<double>().between(-100,100), a.random<double>().between(-100,100), a.random<double>().between(-100,100));
        const double rho = a.random<double>().between(0,100);
        const double g = a.random<double>().between(0,10);
        const double x = P.x();
        const double y = P.y();
        const double z = P.z();
        ASSERT_NEAR(Hs/2*rho*g*exp(-k_*z)*cos(2*PI/Tp*t - k_*(x*cos(psi0)+y*sin(psi0)) +phi), wave.get_dynamic_pressure(rho, g, P, k, 0, t), 1E-6);
    }
//! [SurfaceElevationFromWavesTest dynamic_pressure expected output]
}

TEST_F(SurfaceElevationFromWavesTest, orbital_velocity)
{
    TR1(shared_ptr)<ssc::kinematics::Kinematics> k(new ssc::kinematics::Kinematics());
    SurfaceElevationFromWaves wave(get_model());
    const double g = 4.57;
    const double x = 3.7;
    const double y = 5.3;
    const double z = 1.62;
    const double t = 664.2;
    ssc::kinematics::Point v = wave.orbital_velocity(g, x, y, z, t);
    ASSERT_NEAR(0.26164706746726851, v.x(),1e-10);
    ASSERT_NEAR(0.26164706746726851, v.y(),1e-10);
    ASSERT_NEAR(-0.17968143538987366, v.z(),1e-10);
}

