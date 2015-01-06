/*
 * force_parsersTest.hpp
 *
 *  Created on: Apr 28, 2014
 *      Author: cady
 */

#ifndef FORCE_PARSERSTEST_HPP_
#define FORCE_PARSERSTEST_HPP_

#include "gtest/gtest.h"
#include <ssc/random_data_generator.hpp>

class force_parsersTest : public ::testing::Test
{
    protected:
        force_parsersTest();
        virtual ~force_parsersTest();
        virtual void SetUp();
        virtual void TearDown();
        ssc::random_data_generator::DataGenerator a;

};

#endif  /* FORCE_PARSERSTEST_HPP_ */