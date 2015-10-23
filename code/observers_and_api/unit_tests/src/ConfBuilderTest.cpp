#include "ConfBuilderTest.hpp"
#include "yaml_data.hpp"
#include "ConfBuilder.hpp"


ConfBuilderTest::ConfBuilderTest() : a(ssc::random_data_generator::DataGenerator(78910))
{
}

ConfBuilderTest::~ConfBuilderTest()
{
}

void ConfBuilderTest::SetUp()
{
}

void ConfBuilderTest::TearDown()
{
}

TEST_F(ConfBuilderTest, compute_right_value_of_Tmax_falling_ball)
{
//! [ConfBuilderTest example]
//! [ConfBuilderTest example]
    std::map<std::string,double> u = {};
    ConfBuilder builder(test_data::falling_ball_example(),u);

	ASSERT_EQ(builder.Tmax, 0);


//! [ConfBuilderTest expected output]
//! [ConfBuilderTest expected output]


}