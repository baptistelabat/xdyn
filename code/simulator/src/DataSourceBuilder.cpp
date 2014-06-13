/*
 * DataSourceBuilder.cpp
 *
 *  Created on: 18 avr. 2014
 *      Author: cady
 */

#include "DataSourceBuilder.hpp"
#include "DataSourceBuilderException.hpp"
#include "EulerAngles.hpp"
#include "KinematicsModule.hpp"
#include "GravityModule.hpp"
#include "force_parsers.hpp"
#include "MeshBuilder.hpp"
#include "TextFileReader.hpp"
#include "StlReader.hpp"
#include "HydrostaticModule.hpp"
#include "WaveModule.hpp"
#include "environment_parsers.hpp"
#include "DefaultWaveModel.hpp"
#include "StateDerivativesModule.hpp"

#include <Eigen/Geometry>

#include <functional> // std::bind1st
#include <boost/bind.hpp>

#include "rotation_matrix_builders.hpp"
using namespace kinematics;

typedef Eigen::Matrix<double,6,6> Matrix6x6;

#define FOR_EACH(v,f)  std::for_each(v.begin(),\
                                     v.end(),\
                                     boost::bind(&DataSourceBuilder::f, boost::ref(*this), _1));

MODULE(PointMatrixBuilder, const std::string name = get_name();\
                           const TR1(shared_ptr)<Mesh> T = ds->get<TR1(shared_ptr)<Mesh> >(name);\
                           const TR1(shared_ptr)<PointMatrix> pm = ds->read_only() ? TR1(shared_ptr)<PointMatrix>() : TR1(shared_ptr)<PointMatrix>(new PointMatrix(T->nodes,std::string("mesh(")+name+")"));\
                           ds->set<TR1(shared_ptr)<PointMatrix> >(name, pm);
      )

DataSourceBuilder::DataSourceBuilder(const YamlSimulatorInput& in) : input(in),
                                                                     ds(DataSource()),
                                                                     rotations(in.rotations),
                                                                     mesh_data(std::map<std::string,VectorOfVectorOfPoints>())
{
    std::vector<YamlBody>::const_iterator that_body = in.bodies.begin();
    for (;that_body!=in.bodies.end() ; ++that_body)
    {
        if (not(that_body->mesh.empty()))
        {
            const TextFileReader reader(std::vector<std::string>(1, that_body->mesh));
            mesh_data[that_body->name] = read_stl(reader.get_contents());
        }
    }
}

DataSourceBuilder::DataSourceBuilder(
        const YamlSimulatorInput& input_yaml,
        const std::map<std::string, VectorOfVectorOfPoints>& input_meshes): input(input_yaml),
                                                                            ds(DataSource()),
                                                                            rotations(input_yaml.rotations),
                                                                            mesh_data(input_meshes)
{
}

void DataSourceBuilder::add_initial_conditions(const YamlBody& body)
{
    ds.check_in(__PRETTY_FUNCTION__);
    ds.set(std::string("x(")+body.name+")", body.initial_position_of_body_frame_relative_to_NED_projected_in_NED.coordinates.x);
    ds.set(std::string("y(")+body.name+")", body.initial_position_of_body_frame_relative_to_NED_projected_in_NED.coordinates.y);
    ds.set(std::string("z(")+body.name+")", body.initial_position_of_body_frame_relative_to_NED_projected_in_NED.coordinates.z);
    ds.set(std::string("u(")+body.name+")", body.initial_velocity_of_body_frame_relative_to_NED_projected_in_body.u);
    ds.set(std::string("v(")+body.name+")", body.initial_velocity_of_body_frame_relative_to_NED_projected_in_body.v);
    ds.set(std::string("w(")+body.name+")", body.initial_velocity_of_body_frame_relative_to_NED_projected_in_body.w);
    ds.set(std::string("p(")+body.name+")", body.initial_velocity_of_body_frame_relative_to_NED_projected_in_body.p);
    ds.set(std::string("q(")+body.name+")", body.initial_velocity_of_body_frame_relative_to_NED_projected_in_body.q);
    ds.set(std::string("r(")+body.name+")", body.initial_velocity_of_body_frame_relative_to_NED_projected_in_body.r);
    ds.set(std::string("x(")+body.name+"/mesh)", body.position_of_body_frame_relative_to_mesh.coordinates.x);
    ds.set(std::string("y(")+body.name+"/mesh)", body.position_of_body_frame_relative_to_mesh.coordinates.y);
    ds.set(std::string("z(")+body.name+"/mesh)", body.position_of_body_frame_relative_to_mesh.coordinates.z);
    const EulerAngles a(body.position_of_body_frame_relative_to_mesh.angle.phi, body.position_of_body_frame_relative_to_mesh.angle.theta, body.position_of_body_frame_relative_to_mesh.angle.psi);
    ds.set(body.name+"/mesh", angle2matrix(a));
    ds.check_out();
}

bool match(const std::vector<std::string>& convention, const std::string& first, const std::string& second, const std::string& third);
bool match(const std::vector<std::string>& convention, const std::string& first, const std::string& second, const std::string& third)
{
    return (convention.at(0) == first) and (convention.at(1) == second) and (convention.at(2) == third);
}

RotationMatrix DataSourceBuilder::angle2matrix(const EulerAngles& a) const
{
    if (rotations.order_by == "angle")
    {
        if (match(rotations.convention, "z", "y'", "x''"))
            return rotation_matrix<INTRINSIC, CHANGING_ANGLE_ORDER, CARDAN, 3, 2, 1>(a);
        std::stringstream ss;
        ss << "Rotation convention '" << rotations.convention.at(0) << "," << rotations.convention.at(1) << "," << rotations.convention.at(2) << "' is not currently supported.";
        THROW(__PRETTY_FUNCTION__, DataSourceBuilderException, ss.str());
    }
    else
    {
        THROW(__PRETTY_FUNCTION__, DataSourceBuilderException, std::string("Ordering rotations by '") + rotations.order_by + "' is not currently supported");
    }
    return RotationMatrix();
}

void DataSourceBuilder::add_initial_quaternions(const YamlBody& body)
{
    ds.check_in(__PRETTY_FUNCTION__);
    const YamlAngle angle = body.initial_position_of_body_frame_relative_to_NED_projected_in_NED.angle;
    const EulerAngles a(angle.phi, angle.theta, angle.psi);
    const RotationMatrix R = angle2matrix(a);
    const Eigen::Quaternion<double> q(R);
    ds.set(std::string("qr(")+body.name+")", q.w());
    ds.set(std::string("qi(")+body.name+")", q.x());
    ds.set(std::string("qj(")+body.name+")", q.y());
    ds.set(std::string("qk(")+body.name+")", q.z());
    ds.check_out();
}

void DataSourceBuilder::add_states(const YamlBody& body)
{
    ds.check_in(__PRETTY_FUNCTION__);
    ds.define_derivative(std::string("x(")+body.name+")", std::string("dx/dt(")+body.name+")");
    ds.define_derivative(std::string("y(")+body.name+")", std::string("dy/dt(")+body.name+")");
    ds.define_derivative(std::string("z(")+body.name+")", std::string("dz/dt(")+body.name+")");
    ds.define_derivative(std::string("u(")+body.name+")", std::string("du/dt(")+body.name+")");
    ds.define_derivative(std::string("v(")+body.name+")", std::string("dv/dt(")+body.name+")");
    ds.define_derivative(std::string("w(")+body.name+")", std::string("dw/dt(")+body.name+")");
    ds.define_derivative(std::string("p(")+body.name+")", std::string("dp/dt(")+body.name+")");
    ds.define_derivative(std::string("q(")+body.name+")", std::string("dq/dt(")+body.name+")");
    ds.define_derivative(std::string("r(")+body.name+")", std::string("dr/dt(")+body.name+")");
    ds.define_derivative(std::string("qr(")+body.name+")", std::string("dqr/dt(")+body.name+")");
    ds.define_derivative(std::string("qi(")+body.name+")", std::string("dqi/dt(")+body.name+")");
    ds.define_derivative(std::string("qj(")+body.name+")", std::string("dqj/dt(")+body.name+")");
    ds.define_derivative(std::string("qk(")+body.name+")", std::string("dqk/dt(")+body.name+")");
    ds.check_out();
}

void DataSourceBuilder::add_forces(const YamlBody& body)
{
    ds.check_in(__PRETTY_FUNCTION__);
    std::vector<YamlModel>::const_iterator that_model = body.external_forces.begin();
    for (;that_model!=body.external_forces.end();++that_model)
    {
        if (that_model->model == "gravity") add_gravity(body.name, that_model->yaml, body.dynamics.mass);
        if (that_model->model == "non-linear hydrostatic") add_hydrostatic(body.name, that_model->yaml);
    }
    ds.check_out();
}

Matrix6x6 convert(const YamlInertiaMatrix& M);
Matrix6x6 convert(const YamlInertiaMatrix& M)
{
    Matrix6x6 ret;
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

void DataSourceBuilder::add_inertia(const YamlBody& body)
{
    Matrix6x6 Mrb = convert(body.dynamics.rigid_body_inertia);
    Matrix6x6 Ma = convert(body.dynamics.added_mass);
    if (fabs((Mrb+Ma).determinant())<1E-10)
    {
        std::stringstream ss;
        ss << "Unable to compute the inverse of the total inertia matrix (rigid body inertia + added mass): " << std::endl
           << "Mrb = " << std::endl
           << Mrb << std::endl
           << "Ma = " << std::endl
           << Ma << std::endl
           << "Mrb+Ma = " << std::endl
           << Mrb+Ma << std::endl;
        THROW(__PRETTY_FUNCTION__, DataSourceBuilderException, ss.str());
    }
    Matrix6x6 M_inv = (Mrb+Ma).inverse();
    ds.check_in(__PRETTY_FUNCTION__);
    ds.set<Matrix6x6>(std::string("total inertia(") + body.name + ")", Ma+Mrb);
    ds.set<Matrix6x6>(std::string("solid body inertia(") + body.name + ")", Mrb);
    ds.set<Matrix6x6>(std::string("inverse of the total inertia(") + body.name + ")", M_inv);
    ds.check_out();
}

void DataSourceBuilder::add_gravity(const std::string& body_name, const std::string& yaml, const double mass)
{
    GravityModule g(&ds, "gravity", body_name);
    ds.check_in(__PRETTY_FUNCTION__);
    ds.add(g);
    ds.set<double>(std::string("m(") + body_name + ")", mass);
    ds.set<double>("g", parse_gravity(yaml).g);
    ds.check_out();
}

void DataSourceBuilder::add_hydrostatic(const std::string& body_name, const std::string& yaml)
{
    ds.check_in(__PRETTY_FUNCTION__);
    HydrostaticModule hs(&ds, "hydrostatic", body_name);
    ds.set<double>("rho", parse_hydrostatic(yaml));
    ds.add(hs);
    ds.check_out();
}

void DataSourceBuilder::add_mesh(const YamlBody& body)
{
    ds.check_in(__PRETTY_FUNCTION__);
    MeshBuilder builder(mesh_data[body.name]);
    TR1(shared_ptr)<Mesh> mesh(new Mesh(builder.build()));
    ds.set<TR1(shared_ptr)<Mesh> >(body.name, mesh);
    ds.add<PointMatrixBuilder>(body.name);
    ds.check_out();
}

void DataSourceBuilder::add_kinematics(const std::vector<YamlBody>& bodies)
{
    ds.check_in(__PRETTY_FUNCTION__);
    std::vector<std::string> body_names;
    for (size_t i = 0 ; i < bodies.size() ; ++i)
    {
        body_names.push_back(bodies[i].name);
    }

    ds.add(KinematicsModule(&ds, body_names, "kinematics"));
    ds.check_out();
}

void DataSourceBuilder::add_centre_of_gravity(const YamlBody& body)
{
    ds.check_in(__PRETTY_FUNCTION__);
    const Point G(body.dynamics.centre_of_inertia.frame,
                  body.dynamics.centre_of_inertia.x,
                  body.dynamics.centre_of_inertia.y,
                  body.dynamics.centre_of_inertia.z);
    ds.set(std::string("G(")+body.name+")", G);
    ds.check_out();
}

void DataSourceBuilder::add_wave_height_module(const YamlBody& body)
{
    ds.check_in(__PRETTY_FUNCTION__);
    ds.add(WaveModule(&ds, std::string("wave module(") + body.name + ")", body.name));
    ds.check_out();
}

void DataSourceBuilder::add_sum_of_forces_module(const YamlBody& body)
{
    ds.check_in(__PRETTY_FUNCTION__);
    std::vector<YamlModel>::const_iterator it = body.external_forces.begin();
    std::vector<std::string> forces;
    for (;it!=body.external_forces.end() ; ++it)
    {
        forces.push_back(it->model);
    }
    ds.add(StateDerivativesModule(&ds, body.name, forces));
    ds.check_out();
}

void DataSourceBuilder::add_default_wave_model(const std::string& yaml)
{
    ds.check_in(__PRETTY_FUNCTION__);
    const double zwave = parse_default_wave_model(yaml);
    ds.set<TR1(shared_ptr)<WaveModelInterface> >("wave model", TR1(shared_ptr)<WaveModelInterface>(new DefaultWaveModel(zwave)));
    ds.check_out();
}

void DataSourceBuilder::add_environment_model(const YamlModel& model)
{
    ds.check_in(__PRETTY_FUNCTION__);
    if (model.model=="no waves") add_default_wave_model(model.yaml);
    ds.check_out();
}

DataSource DataSourceBuilder::build_ds()
{
    ds.check_in(__PRETTY_FUNCTION__);
    FOR_EACH(input.bodies, add_initial_conditions);
    FOR_EACH(input.bodies, add_initial_quaternions);
    FOR_EACH(input.bodies, add_states);
    FOR_EACH(input.bodies, add_forces);
    FOR_EACH(input.bodies, add_centre_of_gravity);
    FOR_EACH(input.environment, add_environment_model);
    FOR_EACH(input.bodies, add_mesh);
    FOR_EACH(input.bodies, add_wave_height_module);
    FOR_EACH(input.bodies, add_sum_of_forces_module);
    FOR_EACH(input.bodies, add_inertia);

    add_kinematics(input.bodies);
    ds.check_out();
    return ds;
}
