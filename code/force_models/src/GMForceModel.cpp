/*
 * GMForceModel.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: cady
 */

#include "calculate_gz.hpp"
#include "GMForceModel.hpp"
#include "yaml.h"
#include "parse_unit_value.hpp"
#include "environment_parsers.hpp"
#include "Body.hpp"
#include "ExactHydrostaticForceModel.hpp"
#include "FastHydrostaticForceModel.hpp"
#include "HydrostaticForceModel.hpp"
#include "Observer.hpp"

#include <ssc/exception_handling.hpp>

class GMForceModelException: public ssc::exception_handling::Exception
{
    public:
        GMForceModelException(const char* s) :
               Exception(s)
        {
        }
};

const std::string GMForceModel::model_name = "GM";

GMForceModel::Yaml::Yaml() : name_of_hydrostatic_force_model(), roll_step(0)
{
}

GMForceModel::GMForceModel(const Yaml& data, const std::string& body_name_, const EnvironmentAndFrames& env_) :
        ImmersedSurfaceForceModel(model_name, body_name_, env_)
, underlying_hs_force_model()
, dphi(data.roll_step)
, env(env_)
, GM(new double(0))
{
    boost::optional<ForcePtr> f = data.try_to_parse(data.name_of_hydrostatic_force_model, "", get_body_name(), env);
    if (f)
    {
        underlying_hs_force_model = f.get();
    }
    else
    {
        std::stringstream ss;
        ss << "Unable to find a parser to parse model '" << data.name_of_hydrostatic_force_model << "'";
        THROW(__PRETTY_FUNCTION__, GMForceModelException, ss.str());
    }
}

GMForceModel::Yaml GMForceModel::parse(const std::string& yaml)
{
    std::stringstream stream(yaml);
    YAML::Parser parser(stream);
    YAML::Node node;
    parser.GetNextDocument(node);
    Yaml ret;
    node["name of hydrostatic force model"] >> ret.name_of_hydrostatic_force_model;
    parse_uv(node["roll step"], ret.roll_step);
    bool managed_to_parse = false;
    if (ret.name_of_hydrostatic_force_model == "hydrostatic")
    {
        ret.try_to_parse = ForceModel::build_parser<HydrostaticForceModel>();
        managed_to_parse = true;
    }
    if (ret.name_of_hydrostatic_force_model == "non-linear hydrostatic (exact)")
    {
        ret.try_to_parse = ForceModel::build_parser<ExactHydrostaticForceModel>();
        managed_to_parse = true;
    }
    if (ret.name_of_hydrostatic_force_model == "non-linear hydrostatic (fast)")
    {
        ret.try_to_parse = ForceModel::build_parser<FastHydrostaticForceModel>();
        managed_to_parse = true;
    }
    if (not(managed_to_parse))
    {
        std::stringstream ss;
        ss << "Couldn't find any suitable hydrostatic force model: "
           << "received '" << ret.name_of_hydrostatic_force_model << "', expected one of 'non-linear hydrostatic (exact)', 'non-linear hydrostatic (fast)' or 'hydrostatic'";
        THROW(__PRETTY_FUNCTION__, GMForceModelException, ss.str());
    }
    return ret;
}

BodyStates GMForceModel::get_shifted_states(const BodyStates& states,
        const double t) const
{
    auto euler = states.get_angles(states.convention);
    euler.phi -= dphi;
    const auto quaternions = states.convert(euler, states.convention);
    BodyStates new_states = states;
    new_states.qr.record(t + 1, std::get<0>(quaternions));
    new_states.qi.record(t + 1, std::get<1>(quaternions));
    new_states.qj.record(t + 1, std::get<2>(quaternions));
    new_states.qk.record(t + 1, std::get<3>(quaternions));
    return new_states;
}

double GMForceModel::get_gz_for_shifted_states(const BodyStates& states, const double t) const
{
    BodyStates new_states = get_shifted_states(states, t);
    underlying_hs_force_model->update(new_states, t);
    return calculate_gz(*underlying_hs_force_model, env);
}

ssc::kinematics::Wrench GMForceModel::operator()(const BodyStates& states, const double t) const
{
    ssc::kinematics::Vector6d tau = ssc::kinematics::Vector6d::Zero();
    underlying_hs_force_model->update(states, t);
    const auto ret = underlying_hs_force_model->get_force_in_body_frame();
    const double gz1 = calculate_gz(*underlying_hs_force_model, env);
    const double gz2 = get_gz_for_shifted_states(states, t);
    *GM = (gz1-gz2)/dphi;
    return ret;
}

void GMForceModel::extra_observations(Observer& observer) const
{
    observer.write(*GM,DataAddressing(std::vector<std::string>{"efforts",get_body_name(),get_name(),"GM"},std::string("GM(") + get_body_name() + ")"));
}

double GMForceModel::pe(const BodyStates& , const std::vector<double>& , const EnvironmentAndFrames& ) const
{
    return 0;
}

GMForceModel::DF GMForceModel::dF(const FacetIterator& ,
                      const EnvironmentAndFrames& ,
                      const BodyStates& ,
                      const double
                      ) const
{
    return GMForceModel::DF(EPoint(),EPoint());
}
