/*
 * DiffractionInterpolator.cpp
 *
 *  Created on: Dec 17, 2014
 *      Author: cady
 */

#define _USE_MATH_DEFINE
#include <cmath>
#define PI M_PI

#include "HDBParser.hpp"
#include "DiffractionInterpolator.hpp"

DiffractionInterpolator::DiffractionInterpolator(const HDBParser& data, //<! Data read from the HDB file
                                             const std::vector<double>& omega, //<! Angular frequencies in the wave spectrum (points at which to interpolate the HDB data)
                                             const std::vector<double>& psi, //<! Wave directions (points at which to interpolate the HDB data)
                                             const bool mirror_ //<! Should the RAO for psi between 180° and 360° be calculated by mirroring the RAO between 0° and 180°?
        ) : module(), phase(), mirror(mirror_), omegas(omega), psis(psi)
{
    const auto M_module = data.get_diffraction_module_tables();
    const auto M_phase = data.get_diffraction_phase_tables();

    for (size_t i = 0 ; i < 6 ; ++i)
    {
        module.at(i) = Interpolator(data.get_diffraction_module_omegas(),data.get_diffraction_module_psis(),M_module.at(i));
        phase.at(i) = Interpolator(data.get_diffraction_phase_omegas(),data.get_diffraction_phase_psis(),M_phase.at(i));
    }
}

std::vector<std::vector<double> > DiffractionInterpolator::get_array(Interpolator& i) const
{
    std::vector<std::vector<double> > ret;
    for (auto omega:omegas)
    {
        std::vector<double> v;
        for (auto psi:psis)
        {
            if (mirror and (psi>PI)) v.push_back(i.f(omega,2*PI-psi));
            else                     v.push_back(i.f(omega,psi));
        }
        ret.push_back(v);
    }
    return ret;
}

std::vector<std::vector<double> > DiffractionInterpolator::get_modules(const size_t k //<! Axis index (0 for Fx, 1 for Fy, 2 for Fz, 3 for Mx, 4 for My and 5 for Mz)
                                                     )
{
    return get_array(module.at(k));
}

std::vector<std::vector<double> > DiffractionInterpolator::get_phases(const size_t k //<! Axis index (0 for Fx, 1 for Fy, 2 for Fz, 3 for Mx, 4 for My and 5 for Mz)
                                         )
{
    return get_array(phase.at(k));
}