/*
 * hydrostatic.cpp
 *
 *  Created on: Apr 30, 2014
 *      Author: cady
 */


#include "hydrostatic.hpp"
#include "HydrostaticException.hpp"
#include "kahan_sum.hpp"

#include <algorithm> // std::count_if
#include <iterator>  // std::distance

using namespace hydrostatic;

bool negative (const double x);
bool negative (const double x) { return x<0; }
bool positive (const double x);
bool positive (const double x) { return not(negative(x)); }
size_t get_nb_of_immerged_points(const std::vector<size_t>& idx, const std::vector<double>& delta_z);
size_t get_nb_of_immerged_points(const std::vector<size_t>& idx, const std::vector<double>& delta_z)
{
    const size_t n = idx.size();
    size_t nb_of_immerged_points = 0;
    for (size_t i = 0 ; i < n ; ++i)
    {
        if (delta_z.at(idx[i]) >= 0) nb_of_immerged_points++;
    }
    return nb_of_immerged_points;
}

bool hydrostatic::partially_immerged(const std::vector<size_t>& idx, const std::vector<double>& delta_z)
{
    const size_t nb_of_immerged_points = get_nb_of_immerged_points(idx,delta_z);
    return (nb_of_immerged_points!=idx.size()) and (nb_of_immerged_points != 0);
}

double hydrostatic::average_immersion(const std::vector<size_t>& idx, const std::vector<double>& delta_z)
{
    return sum::kahan(idx,delta_z)/idx.size();
}

std::pair<size_t,size_t> hydrostatic::first_and_last_emerged_points(const std::vector<double>& z)
{
    if (z.empty())
    {
        THROW(__PRETTY_FUNCTION__, HydrostaticException, "z is empty");
    }
    std::vector<double>::const_iterator it = std::find_if(z.begin(), z.end(), negative);
    if (it == z.end())
    {
        THROW(__PRETTY_FUNCTION__, HydrostaticException, "All points are immerged");
    }
    const size_t first = std::distance(z.begin(),it);
    it  = std::find_if(it, z.end(), positive);
    const size_t last = it==z.end() ? z.size() : std::distance(z.begin(),it);
    return std::make_pair(first,last-1);
}

void make_sure_some_points_are_immerged_and_some_are_not(const std::vector<size_t>& idx, const std::vector<double>& v);
void make_sure_some_points_are_immerged_and_some_are_not(const std::vector<size_t>& idx, const std::vector<double>& v)
{
    const size_t n = idx.size();
    const size_t number_of_immerged_nodes = get_nb_of_immerged_points(idx, v);

    if (number_of_immerged_nodes == 0)
    {
        THROW(__PRETTY_FUNCTION__, HydrostaticException, "None of the points are immerged.");
    }
    if (number_of_immerged_nodes == n)
    {
        THROW(__PRETTY_FUNCTION__, HydrostaticException, "All the points are immerged.");
    }
}

Matrix3x hydrostatic::immerged_polygon(const Matrix3x& M,
                                       const std::vector<size_t>& idx,
                                       const std::vector<double>& v)
{
    make_sure_some_points_are_immerged_and_some_are_not(idx, v);
    const size_t n = idx.size();
    std::vector<double> dz(n,0);
    for (size_t i = 0 ; i < n ; ++i)
    {
        dz[i] = v.at(idx[i]);
    }
    const std::pair<size_t,size_t> first_and_last = first_and_last_emerged_points(dz);
    const size_t idxA = first_and_last.first;
    const size_t idxB = first_and_last.second;
    const size_t idxA1 = previous(idx, idxA);
    const size_t idxB1 = next(idx, idxB);
    const EPoint A = M.col(idxA);
    const EPoint A1 = M.col(idxA1);
    const EPoint B = M.col(idxB);
    const EPoint B1 = M.col(idxB1);
    const EPoint P = intersection(A,v[idxA],A1,v[idxA1]);
    const EPoint Q = intersection(B,v[idxB],B1,v[idxB1]);
    const size_t N = (idxB>=idxA) ? n-(idxB-idxA-1) : idxA-idxB+2;
    Eigen::Matrix<double,3,Eigen::Dynamic> ret;
    ret.resize(3,N);
    size_t k = 0;

    if (idxA<idxB)
    {
        for (size_t i = 0 ; i < idxA ; ++i)
        {
            ret.col(k++) = M.col(i);
        }
        ret.col(k++) = P;
        ret.col(k++) = Q;
        for (size_t i = idxB+1 ; i < n ; ++i)
        {
            ret.col(k++) = M.col(i);
        }
    }
    else
    {
        ret.col(k++) = Q;
        for (size_t i = idxB1 ; i <= idxA1 ; ++i)
        {
            ret.col(k++) = M.col(i);
        }
        ret.col(k++) = P;
    }
    return ret;
}

EPoint hydrostatic::intersection(const EPoint& A, const double dzA, const EPoint& B, const double dzB)
{
    if (dzA*dzB>=0)
    {
        THROW(__PRETTY_FUNCTION__, HydrostaticException, "zB & zA must have different signs");
    }
    const double xA = A(0);
    const double xB = B(0);
    const double yA = A(1);
    const double yB = B(1);
    const double zA = A(2);
    const double zB = B(2);
    const double k = dzA/(dzA-dzB);
    return EPoint(xA + k*(xB-xA),
                  yA + k*(yB-yA),
                  zA + k*(zB-zA)
                  );

}

size_t hydrostatic::next(const std::vector<size_t>& idx, const size_t i0)
{
    const size_t n = idx.size();
    for (size_t i = 0 ; i < n ; ++i)
    {
        if (i0 == idx[i])
        {
            if (i== n-1) return idx[0];
            else         return idx[i+1];
        }
    }
    std::stringstream ss;
    ss << "Unable to find index " << i0 << " in list.";
    THROW(__PRETTY_FUNCTION__, HydrostaticException, ss.str());
    return 0;
}

size_t hydrostatic::previous(const std::vector<size_t>& idx, const size_t i0)
{
    const size_t n = idx.size();
    for (size_t i = 0 ; i < n ; ++i)
    {
        if (i0 == idx[i])
        {
            if (i== 0) return idx[n-1];
            else       return idx[i-1];
        }
    }
    std::stringstream ss;
    ss << "Unable to find index " << i0 << " in list.";
    THROW(__PRETTY_FUNCTION__, HydrostaticException, ss.str());
    return 0;
}

UnsafeWrench hydrostatic::dF(const Point& O,           //!< Point at which the Wrench will be given (eg. the body's centre of gravity)
                             const Eigen::Vector3d& C, //!< Point where the force is applied (barycentre of the facet)
                             const double rho,         //!< Density of the fluid (in kg/m^3)
                             const double g,           //!< Earth's standard acceleration due to gravity (eg. 9.80665 m/s^2)
                             const double z,           //!< Relative immersion (in metres)
                             const Eigen::Vector3d& dS //!< Unit normal vector multiplied by the surface of the facet
                           )
{
    const Eigen::Vector3d F = rho*g*z*dS;
    return UnsafeWrench(O.get_frame(), F, (C-O.v).cross(F));
}
