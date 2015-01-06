/*
 * BodyBuilder.cpp
 *
 *  Created on: Jun 17, 2014
 *      Author: cady
 */

#include <ssc/kinematics.hpp>
#include <ssc/text_file_reader.hpp>

#include "BodyBuilder.hpp"
#include "BodyBuilderException.hpp"
#include "HDBParser.hpp"
#include "MeshBuilder.hpp"
#include "YamlBody.hpp"
#include "yaml2eigen.hpp"

BodyBuilder::BodyBuilder(const YamlRotation& convention) : rotations(convention)
{
}

void BodyBuilder::change_mesh_ref_frame(Body& body, const VectorOfVectorOfPoints& mesh) const
{
    ssc::kinematics::Point translation(body.name, body.x_relative_to_mesh, body.y_relative_to_mesh, body.z_relative_to_mesh);
    ssc::kinematics::Transform transform(translation, body.mesh_to_body, "mesh("+body.name+")");
    body.mesh = MeshPtr(new Mesh(MeshBuilder(mesh).build()));
    const auto T = transform.inverse();
    body.mesh->nodes = (T*ssc::kinematics::PointMatrix(body.mesh->nodes, "mesh("+body.name+")")).m;
    body.mesh->all_nodes = (T*ssc::kinematics::PointMatrix(body.mesh->all_nodes, "mesh("+body.name+")")).m;
    for (size_t i = 0 ; i < body.mesh->facets.size() ; ++i)
    {
        body.mesh->facets[i].barycenter = T*body.mesh->facets[i].barycenter;
        body.mesh->facets[i].unit_normal = T.get_rot()*body.mesh->facets[i].unit_normal;
    }
    body.M = ssc::kinematics::PointMatrixPtr(new ssc::kinematics::PointMatrix(body.mesh->nodes, body.name));
}

Body BodyBuilder::build(const YamlBody& input, const VectorOfVectorOfPoints& mesh) const
{
    Body ret;
    ret.name = input.name;
    ret.G = make_point(input.dynamics.centre_of_inertia);
    ret.m = input.dynamics.mass;

    ret.hydrodynamic_forces_calculation_point = make_point(input.dynamics.hydrodynamic_forces_calculation_point_in_body_frame, input.name);

    ret.x_relative_to_mesh = input.position_of_body_frame_relative_to_mesh.coordinates.x;
    ret.y_relative_to_mesh = input.position_of_body_frame_relative_to_mesh.coordinates.y;
    ret.z_relative_to_mesh = input.position_of_body_frame_relative_to_mesh.coordinates.z;
    ret.mesh_to_body = angle2matrix(input.position_of_body_frame_relative_to_mesh.angle, rotations);
    change_mesh_ref_frame(ret, mesh);
    add_inertia(ret, input.dynamics.rigid_body_inertia, input.dynamics.added_mass);
    ret.u = input.initial_velocity_of_body_frame_relative_to_NED_projected_in_body.u;
    ret.v = input.initial_velocity_of_body_frame_relative_to_NED_projected_in_body.v;
    ret.w = input.initial_velocity_of_body_frame_relative_to_NED_projected_in_body.w;
    ret.p = input.initial_velocity_of_body_frame_relative_to_NED_projected_in_body.p;
    ret.q = input.initial_velocity_of_body_frame_relative_to_NED_projected_in_body.q;
    ret.r = input.initial_velocity_of_body_frame_relative_to_NED_projected_in_body.r;
    ret.intersector = MeshIntersectorPtr(new MeshIntersector(ret.mesh));
    return ret;
}

void BodyBuilder::add_inertia(Body& body, const YamlDynamics6x6Matrix& rigid_body_inertia, const YamlDynamics6x6Matrix& added_mass) const
{
    const Eigen::Matrix<double,6,6> Mrb = convert(rigid_body_inertia);
    Eigen::Matrix<double,6,6> Ma;
    if (added_mass.read_from_file)
    {
        const std::string hdb = ssc::text_file_reader::TextFileReader(std::vector<std::string>(1,added_mass.hdb_filename)).get_contents();
        Ma = HDBParser(hdb).get_added_mass();
    }
    else
    {
        Ma = convert(added_mass);
    }
    const Eigen::Matrix<double,6,6> Mt = Mrb + Ma;
    if (fabs(Mt.determinant())<1E-10)
    {
        std::stringstream ss;
        ss << "Unable to compute the inverse of the total inertia matrix (rigid body inertia + added mass): " << std::endl
           << "Mrb = " << std::endl
           << Mrb << std::endl
           << "Ma = " << std::endl
           << Ma << std::endl
           << "Mrb+Ma = " << std::endl
           << Mt << std::endl;
        THROW(__PRETTY_FUNCTION__, BodyBuilderException, ss.str());
    }
    Eigen::Matrix<double,6,6> M_inv = Mt.inverse();
    body.inverse_of_the_total_inertia = MatrixPtr(new Eigen::Matrix<double,6,6>(M_inv));
    body.solid_body_inertia = MatrixPtr(new Eigen::Matrix<double,6,6>(Mrb));
    body.total_inertia = MatrixPtr(new Eigen::Matrix<double,6,6>(Mt));
}

Eigen::Matrix<double,6,6> BodyBuilder::convert(const YamlDynamics6x6Matrix& M) const
{
    Eigen::Matrix<double,6,6> ret;
    for (size_t j = 0 ; j < 6 ; ++j)
    {
        ret(0,(int)j) = M.row_1.at(j);
        ret(1,(int)j) = M.row_2.at(j);
        ret(2,(int)j) = M.row_3.at(j);
        ret(3,(int)j) = M.row_4.at(j);
        ret(4,(int)j) = M.row_5.at(j);
        ret(5,(int)j) = M.row_6.at(j);
    }
    return ret;
}

Body BodyBuilder::build(const std::string& name, const VectorOfVectorOfPoints& mesh) const
{
    YamlBody input;
    input.name = name;
    input.dynamics.centre_of_inertia.frame = name;
    input.dynamics.rigid_body_inertia.frame = name;
    input.dynamics.rigid_body_inertia.row_1 = {1,0,0,0,0,0};
    input.dynamics.rigid_body_inertia.row_2 = {0,1,0,0,0,0};
    input.dynamics.rigid_body_inertia.row_3 = {0,0,1,0,0,0};
    input.dynamics.rigid_body_inertia.row_4 = {0,0,0,1,0,0};
    input.dynamics.rigid_body_inertia.row_5 = {0,0,0,0,1,0};
    input.dynamics.rigid_body_inertia.row_6 = {0,0,0,0,0,1};
    input.dynamics.added_mass = input.dynamics.rigid_body_inertia;
    return build(input, mesh);
}