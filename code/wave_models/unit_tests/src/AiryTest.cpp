/*
 * AiryTest.cpp
 *
 *  Created on: Aug 1, 2014
 *      Author: cady
 */

#include "AiryTest.hpp"
#include "Airy.hpp"
#include "BretschneiderSpectrum.hpp"
#include "Cos2sDirectionalSpreading.hpp"
#include "DiracSpectralDensity.hpp"
#include "DiracDirectionalSpreading.hpp"
#include "SumOfWaveSpectralDensities.hpp"
#include "SumOfWaveDirectionalSpreadings.hpp"

#include "discretize.hpp"

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI

AiryTest::AiryTest() : a(ssc::random_data_generator::DataGenerator(54200))
{
}

AiryTest::~AiryTest()
{
}

void AiryTest::SetUp()
{
}

void AiryTest::TearDown()
{
}

#define EPS 1E-10
#define BIG_EPS 1E-4


TEST_F(AiryTest, single_frequency_single_direction_at_one_point)
{
    //! [AiryTest example]
    const double psi0 = PI/4;
    const double Hs = 3;
    const double Tp = 10;
    const double omega0 = 2*PI/Tp;
    const double omega_min = a.random<double>().greater_than(0);
    const double omega_max = a.random<double>().greater_than(omega_min);
    const size_t nfreq = a.random<size_t>().between(2,100);
    const DiscreteDirectionalWaveSpectrum A = discretize(DiracSpectralDensity(omega0, Hs), DiracDirectionalSpreading(psi0), omega_min, omega_max, nfreq);
    int random_seed = 0;
    const Airy wave(A, random_seed);

    const double x = a.random<double>();
    const double y = a.random<double>();
    const double phi = 3.4482969340598712549;
    const double k = 4.*PI*PI/Tp/Tp/9.81;
    //! [AiryTest example]
    //! [AiryTest expected output]
    for (double t = 0 ; t < 3*Tp ; t+=0.1)
    {
        ASSERT_NEAR(sqrt(2*Hs)*cos(k*(x*cos(psi0)+y*sin(psi0))-2*PI/Tp*t +phi), wave.elevation(x,y,t), 1E-6);
    }
    //! [AiryTest expected output]
}

TEST_F(AiryTest, two_frequencies_single_direction_at_one_point)
{
    const double psi0 = PI/4;
    const double Hs = 3;
    const double Tp = 10;
    const double omega0 = 2*PI/Tp;
    const double omega_min = 0.01;
    const double omega_max = 10;
    const size_t nfreq = 50;
    const DiscreteDirectionalWaveSpectrum A = discretize(DiracSpectralDensity(omega0, Hs)+DiracSpectralDensity(omega0, Hs), DiracDirectionalSpreading(psi0), omega_min, omega_max, nfreq);
    int random_seed = 0;
    const Airy wave(A, random_seed);

    const double x = a.random<double>();
    const double y = a.random<double>();
    const double phi = 3.4482969340598712549;
    const double k = 4.*PI*PI/Tp/Tp/9.81;
    for (double t = 0 ; t < 3*Tp ; t+=0.1)
    {
        ASSERT_NEAR(2*sqrt(Hs)*cos(k*(x*cos(psi0)+y*sin(psi0))-2*PI/Tp*t +phi), wave.elevation(x,y,t), 1E-6);
    }
}

TEST_F(AiryTest, one_frequency_two_directions_at_one_point)
{
    const double psi0 = PI/4;
    const double Hs = 3;
    const double Tp = 10;
    const double omega0 = 2*PI/Tp;
    const double omega_min = 0.01;
    const double omega_max = 10;
    const size_t nfreq = 50;
    const DiscreteDirectionalWaveSpectrum A = discretize(DiracSpectralDensity(omega0, Hs), DiracDirectionalSpreading(psi0)+DiracDirectionalSpreading(psi0), omega_min, omega_max, nfreq);
    int random_seed = 0;
    const Airy wave(A, random_seed);

    const double x = a.random<double>();
    const double y = a.random<double>();
    const double phi = 3.4482969340598712549;
    const double k = 4.*PI*PI/Tp/Tp/9.81;
    for (double t = 0 ; t < 3*Tp ; t+=0.1)
    {
        ASSERT_NEAR(2*sqrt(Hs)*cos(k*(x*cos(psi0)+y*sin(psi0))-2*PI/Tp*t +phi), wave.elevation(x,y,t), 1E-6);
    }
}

TEST_F(AiryTest, bug)
{
    const double psi0 = PI;
    const double Hs = 2;
    const double Tp = 7;
    const double s = 1;
    const double omega_min = 0.1;
    const double omega_max = 6;
    const size_t nfreq = 101;
    const DiscreteDirectionalWaveSpectrum A = discretize(BretschneiderSpectrum(Hs, Tp), Cos2sDirectionalSpreading(psi0, s), omega_min, omega_max, nfreq);
    int random_seed = 0;
    const Airy wave(A, random_seed);

    const double x = 0;
    const double y = 0;
    const double t = 0;

    ASSERT_LT(fabs(wave.elevation(x,y,t)), 5);

}

TEST_F(AiryTest, dynamic_pressure)
{
    const double psi0 = PI/4;
    const double Hs = 3;
    const double Tp = 10;
    const double omega0 = 2*PI/Tp;
    const double omega_min = a.random<double>().greater_than(0);
    const double omega_max = a.random<double>().greater_than(omega_min);
    const size_t nfreq = a.random<size_t>().between(2,100);
    const DiscreteDirectionalWaveSpectrum A = discretize(DiracSpectralDensity(omega0, Hs), DiracDirectionalSpreading(psi0), omega_min, omega_max, nfreq);
    int random_seed = 0;
    const Airy wave(A, random_seed);

    const double x = a.random<double>().between(-100,100);
    const double y = a.random<double>().between(-100,100);
    const double z = a.random<double>().between(0,100);
    const double phi = 3.4482969340598712549;
    const double rho = 1024;
    const double g = 9.81;
    const double k = 4.*PI*PI/Tp/Tp/g;

    for (double t = 0 ; t < 3*Tp ; t+=0.1)
    {
        ASSERT_NEAR(sqrt(2*Hs)*rho*g*exp(-k*z)*cos(k*(x*cos(psi0)+y*sin(psi0))-2*PI/Tp*t +phi), wave.dynamic_pressure(rho,g,x,y,z,t), 1E-6);
    }
}

TEST_F(AiryTest, validate_formula_against_results_from_sos)
{
    const double Hs = 0.1;
    const double Tp = 5;
    const double omega0 = 2*PI/Tp;
    double psi = 0;
    double phi = 5.8268;
    double t = 0;
    const double g = 9.81;
    const double k = omega0*omega0/g;

    double x=-0.1; double y=0;
    ASSERT_NEAR(0.045232, Hs/2*cos(omega0*t-k*(x*cos(psi)+y*sin(psi))+phi), BIG_EPS);
    x=0.1;y=0;
    ASSERT_NEAR(0.044522, Hs/2*cos(omega0*t-k*(x*cos(psi)+y*sin(psi))+phi), BIG_EPS);
    x=0;y=-0.1;
    ASSERT_NEAR(0.044883, Hs/2*cos(omega0*t-k*(x*cos(psi)+y*sin(psi))+phi), BIG_EPS);
    x=0;y=0.1;
    ASSERT_NEAR(0.044883, Hs/2*cos(omega0*t-k*(x*cos(psi)+y*sin(psi))+phi), BIG_EPS);
    x=0;y=0;
    ASSERT_NEAR(0.044883, Hs/2*cos(omega0*t-k*(x*cos(psi)+y*sin(psi))+phi), BIG_EPS);
}

TEST_F(AiryTest, dynamic_pressure_compare_with_sos_stab)
{
    const double psi0 = PI;
    const double Hs = 0.1;
    const double Tp = 5;
    const double omega0 = 2*PI/Tp;
    const double omega_min = a.random<double>().greater_than(0);
    const double omega_max = a.random<double>().greater_than(omega_min);
    const size_t nfreq = a.random<size_t>().between(2,100);
    const DiscreteDirectionalWaveSpectrum A = discretize(DiracSpectralDensity(omega0, Hs), DiracDirectionalSpreading(psi0), omega_min, omega_max, nfreq);
    int random_seed = 0;
    const Airy wave(A, random_seed);

    double x = -0.1;
    double y = 0;
    double z = 0.2;
    const double rho = 1024;
    const double g = 9.81;

    const double k = omega0*omega0/g;

    double psi = 0;
    double phi = 5.8268;
    //double t = 0;

    COUT(sqrt(2*Hs)*cos(omega0*0-k*(x*cos(psi)+y*sin(psi))+phi));
    COUT(sqrt(2*Hs)*cos(omega0*0.2-k*(x*cos(psi)+y*sin(psi))+phi));
    COUT(sqrt(2*Hs)*cos(omega0*0.4-k*(x*cos(psi)+y*sin(psi))+phi));


    x=0.1;y=0;z=0.2;
    COUT(sqrt(2*Hs)*cos(omega0*0-k*(x*cos(psi)+y*sin(psi))+phi));
    COUT(sqrt(2*Hs)*cos(omega0*0.2-k*(x*cos(psi)+y*sin(psi))+phi));
    COUT(sqrt(2*Hs)*cos(omega0*0.4-k*(x*cos(psi)+y*sin(psi))+phi));

    (void) z;
    (void) rho;
/*
    double theta= 1.46948;//omega0*t-k*(x*cos(psi)+y*sin(psi))+phi;
    const double zeta = 0.045232;//sqrt(2*Hs)*sin(theta);
    COUT(theta);
    COUT(zeta);
    const double dS = 0.2*0.2;
    COUT(rho*g*(z-zeta)*dS);
    COUT(dS*rho*g*sqrt(2*Hs)*exp(-k*(z-zeta))*cos(theta));
    //ASSERT_NEAR(0.045232, zeta, 1E-6);
    // Facet facing negative x
    //ASSERT_NEAR(-0.044118, 0.2*0.2*0.2*wave.dynamic_pressure(rho,g,x,y,z,t), 1E-6);
*/
}
