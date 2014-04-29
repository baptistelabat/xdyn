#include <cmath>

#include "MeshException.hpp"
#include "TriMesh.hpp"
#include "TriMeshBuilderTest.hpp"
#include "TriMeshTestData.hpp"
#include "TriMeshBuilder.hpp"

TEST_F(TriMeshBuilderTest, should_be_able_to_build_a_mesh_from_a_single_triangle)
{
	const TriMesh m = TriMeshBuilder(one_triangle()).build();
	ASSERT_EQ(1,m.facets.size());
	ASSERT_EQ(3,m.nodes.cols());
}

TEST_F(TriMeshBuilderTest, adding_twice_a_triangle_should_not_change_the_number_of_nodes)
{
	VectorOfPoint3dTriplet triangles;
	triangles.push_back(one_triangle());
	triangles.push_back(one_triangle());
	TriMesh m = TriMeshBuilder(triangles).build();
	ASSERT_EQ(2,m.facets.size());
	ASSERT_EQ(3,m.nodes.cols());
}

TEST_F(TriMeshBuilderTest, should_be_able_to_evaluate_the_barycenter_of_a_single_triangle)
{
	const TriMesh m = TriMeshBuilder(one_triangle()).build();
	ASSERT_EQ(1,m.facets.size());
	ASSERT_EQ(3,m.nodes.cols());
	ASSERT_DOUBLE_EQ((1.0+2.1+3.1)/3.0,m.facets.at(0).barycenter(0));
	ASSERT_DOUBLE_EQ((2.1+3.7+4.5)/3.0,m.facets.at(0).barycenter(1));
	ASSERT_DOUBLE_EQ((3.2+4.5+6.7)/3.0,m.facets.at(0).barycenter(2));
}

TEST_F(TriMeshBuilderTest, should_be_able_to_evaluate_the_area_of_a_single_triangle)
{
	const TriMesh m = TriMeshBuilder(one_triangle()).build();
	Eigen::Vector3d n1,n2,v;
	ASSERT_EQ(1,m.facets.size());
	ASSERT_EQ(3,m.nodes.cols());
	n1(0)= 2.1-1.0;
	n1(1)= 3.7-2.1;
	n1(2)= 4.5-3.2;
	n2(0)= 3.1-1.0;
	n2(1)= 4.5-2.1;
	n2(2)= 6.7-3.2;
	v(0) = n1(1)*n2(2)-n1(2)*n2(1);
	v(1) = n1(2)*n2(0)-n1(0)*n2(2);
	v(2) = n1(0)*n2(1)-n1(1)*n2(0);
	ASSERT_DOUBLE_EQ(0.5*sqrt(v(0)*v(0)+v(1)*v(1)+v(2)*v(2)),m.facets.at(0).area);
}

TEST_F(TriMeshBuilderTest, should_throw_if_a_triangle_is_degenerated)
{
    TriMeshBuilder builder(degenerated_triangle());
    ASSERT_THROW(builder.build(), MeshException);
}

TEST_F(TriMeshBuilderTest, should_be_able_to_evaluate_the_normal_of_a_single_triangle)
{
	const TriMesh m = TriMeshBuilder(one_triangle()).build();

	ASSERT_EQ(1,m.facets.size());
	ASSERT_EQ(3,m.nodes.cols());
	ASSERT_DOUBLE_EQ(+2.48/sqrt(2.48*2.48+1.12*1.12+0.72*0.72),m.facets.at(0).unit_normal(0));
	ASSERT_DOUBLE_EQ(-1.12/sqrt(2.48*2.48+1.12*1.12+0.72*0.72),m.facets.at(0).unit_normal(1));
	ASSERT_DOUBLE_EQ(-0.72/sqrt(2.48*2.48+1.12*1.12+0.72*0.72),m.facets.at(0).unit_normal(2));
}

TEST_F(TriMeshBuilderTest, should_be_able_to_represent_a_cube)
{
	const TriMesh m = TriMeshBuilder(unit_cube()).build();
	ASSERT_EQ(12,m.facets.size());
	ASSERT_EQ(8,m.nodes.cols());
	for (size_t i=0;i<12;++i)
		ASSERT_DOUBLE_EQ(0.5,m.facets.at(i).area)<<i;
}

