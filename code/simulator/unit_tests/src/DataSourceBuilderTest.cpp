/*
 * DataSourceBuilderTest.cpp
 *
 *  Created on: 18 avr. 2014
 *      Author: cady
 */


#include "DataSourceBuilderTest.hpp"
#include "DataSourceBuilder.hpp"
#include "SimulatorYamlParser.hpp"
#include "yaml_data.hpp"
#include "STL_data.hpp"
#include "EulerAngles.hpp"
#include "rotation_matrix_builders.hpp"
#include "Wrench.hpp"
#include "Mesh.hpp"
#include "PointMatrix.hpp"
#include "WaveModelInterface.hpp"
#include "Kinematics.hpp"

#include <fstream>
#include <Eigen/Geometry>

DataSourceBuilderTest::DataSourceBuilderTest() : a(DataGenerator(12)), ds(DataSource())
{
	const SimulatorYamlParser parser(test_data::full_example());
	DataSourceBuilder builder(parser.parse());
	ds = builder.build_ds();
}

DataSourceBuilderTest::~DataSourceBuilderTest()
{
}

void DataSourceBuilderTest::SetUp()
{
}

void DataSourceBuilderTest::TearDown()
{
}

TEST_F(DataSourceBuilderTest, DataSource_should_contain_initial_states)
{
	ASSERT_DOUBLE_EQ(4,ds.get<double>("x(body 1)"));
	ASSERT_DOUBLE_EQ(8,ds.get<double>("y(body 1)"));
	ASSERT_DOUBLE_EQ(12,ds.get<double>("z(body 1)"));
	ASSERT_DOUBLE_EQ(-8,ds.get<double>("u(body 1)"));
	ASSERT_DOUBLE_EQ(-9,ds.get<double>("v(body 1)"));
	ASSERT_DOUBLE_EQ(14,ds.get<double>("w(body 1)"));
	ASSERT_DOUBLE_EQ(56,ds.get<double>("p(body 1)"));
	ASSERT_DOUBLE_EQ(7,ds.get<double>("q(body 1)"));
	ASSERT_DOUBLE_EQ(6,ds.get<double>("r(body 1)"));
}

TEST_F(DataSourceBuilderTest, DataSource_should_contain_initial_quaternions)
{
	const EulerAngles expected(13,14,15);
	const Eigen::Quaternion<double> q(kinematics::rotation_matrix<kinematics::INTRINSIC, kinematics::ORDERED_BY_ANGLE, kinematics::CARDAN, 3, 2, 1>(expected));

	ASSERT_DOUBLE_EQ(q.w(),ds.get<double>("qr(body 1)"));
	ASSERT_DOUBLE_EQ(q.x(),ds.get<double>("qi(body 1)"));
	ASSERT_DOUBLE_EQ(q.y(),ds.get<double>("qj(body 1)"));
	ASSERT_DOUBLE_EQ(q.z(),ds.get<double>("qk(body 1)"));
}

TEST_F(DataSourceBuilderTest, DataSource_should_contain_the_right_number_of_states)
{
	ASSERT_EQ(13, ds.get_state_names().size());
}

TEST_F(DataSourceBuilderTest, DataSource_should_contain_the_gravity_constant)
{
    ASSERT_DOUBLE_EQ(9.81, ds.get<double>("g"));
}

TEST_F(DataSourceBuilderTest, DataSource_should_contain_the_gravity_force_of_each_body)
{
    const Wrench Fg = ds.get<Wrench>("gravity(body 1)");
    ASSERT_DOUBLE_EQ(0,Fg.X);
    ASSERT_DOUBLE_EQ(0,Fg.Y);
    ASSERT_DOUBLE_EQ(9.81E6,Fg.Z);
    ASSERT_DOUBLE_EQ(0,Fg.K);
    ASSERT_DOUBLE_EQ(0,Fg.M);
    ASSERT_DOUBLE_EQ(0,Fg.N);
}
#include <sys/stat.h>
inline bool file_exists(const std::string& name);
inline bool file_exists(const std::string& name)
{
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

void DataSourceBuilderTest::make_stl_file(const std::string& data, const std::string& filename) const
{
    if (not(file_exists(filename)))
    {
        std::ofstream file;
        file.open(filename.c_str());
        file << data;
    }
}

TEST_F(DataSourceBuilderTest, DataSource_should_contain_mesh_of_each_body)
{
    make_stl_file(test_data::three_facets(), "anthineas.stl");
    const std::tr1::shared_ptr<Mesh> m = ds.get<std::tr1::shared_ptr<Mesh> >("body 1");
    ASSERT_EQ(3, m->facets.size());
}

TEST_F(DataSourceBuilderTest, DataSource_should_contain_a_PointMatrix_for_each_body)
{
    make_stl_file(test_data::three_facets(), "anthineas.stl");
    const std::tr1::shared_ptr<PointMatrix> P = ds.get<std::tr1::shared_ptr<PointMatrix> >("body 1");
    ASSERT_EQ(3, P->m.rows());
    ASSERT_EQ(7, P->m.cols());
    ASSERT_EQ("body 1", P->get_frame());
}

TEST_F(DataSourceBuilderTest, DataSource_should_contain_centre_of_gravity_of_each_body)
{
    const Point G = ds.get<Point>("G(body 1)");
    ASSERT_EQ("body 1", G.get_frame());
    ASSERT_DOUBLE_EQ(4, G.x);
    ASSERT_DOUBLE_EQ(7, G.y);
    ASSERT_DOUBLE_EQ(-10, G.z);
}

TEST_F(DataSourceBuilderTest, DataSource_should_contain_wave_model)
{
    const std::tr1::shared_ptr<WaveModelInterface> wave_model = ds.get<std::tr1::shared_ptr<WaveModelInterface> >("wave model");
    const std::tr1::shared_ptr<Kinematics> k = ds.get<std::tr1::shared_ptr<Kinematics> >("kinematics");
    for (size_t i = 0 ; i < 100 ; ++i)
    {
        const double z0 = a.random<double>();
        const Point P("NED",a.random<double>(),a.random<double>(),z0);
        const Point Q("body 1",a.random<double>(),a.random<double>(),z0);
        ASSERT_DOUBLE_EQ(12-z0, wave_model->get_relative_wave_height(P,k));
        ASSERT_NO_THROW(wave_model->get_relative_wave_height(Q,k));
    }
}
