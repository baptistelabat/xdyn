/*
 * discretize.cpp
 *
 *  Created on: Aug 1, 2014
 *      Author: cady
 */
#include "discretize.hpp"
#include "WaveDirectionalSpreading.hpp"
#include "WaveSpectralDensity.hpp"
#include "Stretching.hpp"
#include "InternalErrorException.hpp"
#include "InvalidInputException.hpp"

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI
#include <list>
#include <utility> //std::pair
#include <cmath> // For isnan

DiscreteDirectionalWaveSpectrum common(
        const WaveSpectralDensity& S,      //!< Frequency spectrum
        const WaveDirectionalSpreading& D, //!< Spatial spectrum
        const double omega_min,            //!< Lower bound of the angular frequency range (in rad/s)
        const double omega_max,            //!< Upper bound of the angular frequency range (in rad/s)
        const size_t nfreq                 //!< Number of frequencies & number of directions in discrete spectrum
        );
DiscreteDirectionalWaveSpectrum common(
        const WaveSpectralDensity& S,      //!< Frequency spectrum
        const WaveDirectionalSpreading& D, //!< Spatial spectrum
        const double omega_min,            //!< Lower bound of the angular frequency range (in rad/s)
        const double omega_max,            //!< Upper bound of the angular frequency range (in rad/s)
        const size_t nfreq                 //!< Number of frequencies & number of directions in discrete spectrum
        )
{
    DiscreteDirectionalWaveSpectrum ret;
    ret.omega = S.get_angular_frequencies(omega_min, omega_max, nfreq);
    ret.psi = D.get_directions(nfreq);
    if (ret.omega.size()>1) ret.domega = ret.omega[1]-ret.omega[0];
    if (ret.psi.size()>1)   ret.dpsi = ret.psi[1]-ret.psi[0];
    ret.Si.reserve(ret.omega.size());
    ret.Dj.reserve(ret.psi.size());
    for (const auto omega:ret.omega) ret.Si.push_back(S(omega));
    for (const auto psi:ret.psi) ret.Dj.push_back(D(psi));
    return ret;
}

DiscreteDirectionalWaveSpectrum discretize(
        const WaveSpectralDensity& S,      //!< Frequency spectrum
        const WaveDirectionalSpreading& D, //!< Spatial spectrum
        const double omega_min,            //!< Lower bound of the angular frequency range (in rad/s)
        const double omega_max,            //!< Upper bound of the angular frequency range (in rad/s)
        const size_t nfreq,                //!< Number of frequencies & number of directions in discrete spectrum
        const Stretching& stretching       //!< Dilate z-axis to properly compute orbital velocities (delta-stretching)
        )
{
    DiscreteDirectionalWaveSpectrum ret = common(S,D,omega_min,omega_max,nfreq);
    ret.k.reserve(ret.omega.size());
    for (const auto omega:ret.omega) ret.k.push_back(S.get_wave_number(omega));
    ret.pdyn_factor = [stretching](const double k, const double z, const double eta){return dynamic_pressure_factor(k,z,eta,stretching);};
    ret.pdyn_factor_sh = [stretching](const double k, const double z, const double eta){return dynamic_pressure_factor(k,z,eta,stretching);};
    return ret;
}

/**  \author cec
  *  \date Aug 1, 2014, 5:04:24 PM
  *  \brief Discretize a wave spectrum
  *  \details Finite depth hypothesis (for the wave number)
  *  \snippet environment_models/unit_tests/src/discretizeTest.cpp discretizeTest discretize_example
  */
DiscreteDirectionalWaveSpectrum discretize(
        const WaveSpectralDensity& S,      //!< Frequency spectrum
        const WaveDirectionalSpreading& D, //!< Spatial spectrum
        const double omega_min,            //!< Lower bound of the angular frequency range (in rad/s)
        const double omega_max,            //!< Upper bound of the angular frequency range (in rad/s)
        const size_t nfreq,                //!< Number of frequencies & number of directions in discrete spectrum
        const double h,                    //!< Water depth (in meters)
        const Stretching& stretching       //!< Dilate z-axis to properly compute orbital velocities (delta-stretching)
        )
{
    DiscreteDirectionalWaveSpectrum ret = common(S,D,omega_min,omega_max,nfreq);
    ret.k.reserve(ret.omega.size());
    for (const auto omega:ret.omega) ret.k.push_back(S.get_wave_number(omega,h));
    for (size_t i = 0 ; i < ret.k.size() ; ++i)
    {
        const double k = ret.k.at(i);
        if (k*h<PI/10.)
        {
            THROW(__PRETTY_FUNCTION__, InvalidInputException, "You should be using a shallow water model (currently none exist in X-DYN) because the water depth (h = " << h << " m) is lower than the wave length divided by twenty (lambda/20 = " << 2*PI/k << ") for omega = " << ret.omega.at(i));
        }
    }
    ret.pdyn_factor = [h,stretching](const double k, const double z, const double eta){return dynamic_pressure_factor(k,z,h,eta,stretching);};
    ret.pdyn_factor_sh = [h,stretching](const double k, const double z, const double eta){return dynamic_pressure_factor_sh(k,z,h,eta,stretching);};
    return ret;
}

typedef std::pair<double,size_t> ValIdx;
bool comparator ( const ValIdx& l, const ValIdx& r);
bool comparator ( const ValIdx& l, const ValIdx& r)
   { return l.first > r.first; }


/**  \brief Only select the most important spectrum components & create single vector
  *  \details No need to loop on all frequencies & all directions: we only select
  *  the most important ones (i.e. those representing a given ratio of the total
  *  energy in the spectrum.
  *  \returns A flat spectrum (i.e. one where the freq & direct. loops have been unrolled)
  *  \snippet environment_models/unit_tests/src/discretizeTest.cpp discretizeTest flatten_example
  */
FlatDiscreteDirectionalWaveSpectrum flatten(
        const DiscreteDirectionalWaveSpectrum& spectrum, //!< Spectrum to flatten
        const double ratio //!< Between 0 & 1: where should we cut off the spectra?
        )
{
    FlatDiscreteDirectionalWaveSpectrum ret;
    double S = 0;
    const size_t nOmega = spectrum.omega.size();
    const size_t nPsi = spectrum.psi.size();
    std::list<ValIdx> SiDj;
    std::vector<size_t> i_idx(nOmega*nPsi);
    std::vector<size_t> j_idx(nOmega*nPsi);
    size_t k = 0;
    for (size_t i = 0 ; i < nOmega ; ++i)
    {
        for (size_t j = 0 ; j < nPsi ; ++j)
        {
            const double s = spectrum.Si[i]*spectrum.Dj[j];
            S += s;
            SiDj.push_back(std::make_pair(s,k));
            i_idx[k] = i;
            j_idx[k] = j;
            k++;
        }
    }
    SiDj.sort(comparator);

    double cumsum = 0;
    const double max_energy = ratio*S;
    const size_t n = SiDj.size();
    for (size_t k = 0 ; k < n ; ++k)
    {
        const ValIdx sidj = SiDj.front();
        SiDj.pop_front();
        cumsum += sidj.first;
        if (cumsum>max_energy) return ret;
        DiscreteDirectionalWaveDensity x;
        const size_t i = i_idx[sidj.second];
        const size_t j = j_idx[sidj.second];
        x.k = spectrum.k[i];
        x.omega = spectrum.omega[i];
        x.psi = spectrum.psi[j];
        x.a = sqrt(2 * sidj.first * spectrum.domega * spectrum.dpsi);
        ret.spectrum.push_back(x);
    }
    return ret;
}

/**
 * \param spectrum
 * \return flattened spectrum
 * \note We keep all components for the time being
 * If we filter and discard some rays, we should also take into account rao...
 */
FlatDiscreteDirectionalWaveSpectrum2 flatten2(
        const DiscreteDirectionalWaveSpectrum& spectrum //!< Spectrum to flatten
        )
{
    FlatDiscreteDirectionalWaveSpectrum2 ret;
    const size_t nOmega = spectrum.omega.size();
    const size_t nPsi = spectrum.psi.size();
    for (size_t i = 0 ; i < nOmega ; ++i)
    {
        for (size_t j = 0 ; j < nPsi ; ++j)
        {
            ret.k.push_back(spectrum.k[i]);
            ret.omega.push_back(spectrum.omega[i]);
            ret.psi.push_back(spectrum.psi[j]);
            ret.cos_psi.push_back(cos(spectrum.psi[j]));
            ret.sin_psi.push_back(sin(spectrum.psi[j]));
            const double s = spectrum.Si[i] * spectrum.Dj[j];
            ret.a.push_back(sqrt(2 * s * spectrum.domega * spectrum.dpsi));
        }
    }
    ret.pdyn_factor = spectrum.pdyn_factor;
    ret.pdyn_factor_sh = spectrum.pdyn_factor_sh;
    return ret;
}

double dynamic_pressure_factor(const double k,              //!< Wave number (in 1/m)
                               const double z,              //!< z-position in the NED frame (in meters)
                               const double eta,            //!< Wave elevation at (x,y) in the NED frame (in meters) for stretching
                               const Stretching& stretching //!< Dilate z-axis to properly compute orbital velocities (delta-stretching)
                              )
{
    if (std::isnan(z))
    {
        THROW(__PRETTY_FUNCTION__, InternalErrorException, "z (value to rescale, in meters) was NaN");
    }
    if (std::isnan(eta))
    {
        THROW(__PRETTY_FUNCTION__, InternalErrorException, "eta (wave height, in meters) was NaN");
    }
    if (eta != 0 && z<eta) return 0;
    return exp(-k*stretching.rescaled_z(z,eta));
}

double dynamic_pressure_factor(const double k,              //!< Wave number (in 1/m)
                               const double z,              //!< z-position in the NED frame (in meters)
                               const double h,              //!< Average water depth (in meters)
                               const double eta,            //!< Wave elevation at (x,y) in the NED frame (in meters) for stretching
                               const Stretching& stretching //!< Dilate z-axis to properly compute orbital velocities (delta-stretching)
                              )
{
    if (std::isnan(z))
    {
        THROW(__PRETTY_FUNCTION__, InternalErrorException, "z (value to rescale, in meters) was NaN");
    }
    if (std::isnan(eta))
    {
        THROW(__PRETTY_FUNCTION__, InternalErrorException, "eta (wave height, in meters) was NaN");
    }
    if (eta != 0 && z<eta) return 0;
    if (z>h) return 0;
    return cosh(k*(h-stretching.rescaled_z(z,eta)))/cosh(k*h);
}

double dynamic_pressure_factor_sh(const double k,              //!< Wave number (in 1/m)
                                  const double z,              //!< z-position in the NED frame (in meters)
                                  const double h,              //!< Average water depth (in meters)
                                  const double eta,            //!< Wave elevation at (x,y) in the NED frame (in meters) for stretching
                                  const Stretching& stretching //!< Dilate z-axis to properly compute orbital velocities (delta-stretching)
                              )
{
    if (eta != 0 && z<eta) return 0;
    if (z>h) return 0;
    return sinh(k*(h-stretching.rescaled_z(z,eta)))/cosh(k*h);
}
