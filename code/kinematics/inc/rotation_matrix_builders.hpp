/*
 * rotation_matrix_builders.hpp
 *
 *  Created on: 7 avr. 2014
 *      Author: maroff
 */

#ifndef ROTATION_MATRIX_BUILDERS_HPP_
#define ROTATION_MATRIX_BUILDERS_HPP_

#include <Eigen/Dense>

typedef Eigen::Matrix<double, 3, 3> RotationMatrix;

/** \author cec
 *  \brief 36 different rotation matrices
 *  \details Intrinsic/Extrinsic, axis/angle order convention, Proper Euler/Cardan
 *  \ingroup kinematics
 *  \section ex1 Example
 *  \snippet kinematics/unit_tests/src/rotation_matrix_buildersTest.cpp rotation_matrix_buildersTest example
 *  \section ex2 Expected output
 *  \snippet kinematics/unit_tests/src/rotation_matrix_buildersTest.cpp rotation_matrix_buildersTest expected output
 */

namespace kinematics
{
    typedef enum {EXTRINSIC, INTRINSIC} IntrinsicOrExtrinsic;
    typedef enum {AXIS, ANGLE} OrderConvention;
    typedef enum {PROPER_EULER, CARDAN} EulerOrCardan;
    template <IntrinsicOrExtrinsic,
              OrderConvention,
              EulerOrCardan,
              int I,
              int J,
              int K> RotationMatrix rotation_matrix(const double phi, const double theta, const double psi);
}

#endif /* ROTATION_MATRIX_BUILDERS_HPP_ */
