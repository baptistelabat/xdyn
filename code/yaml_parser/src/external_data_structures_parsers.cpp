/*
 * external_data_structures_parsers.cpp
 *
 *  Created on: Oct 3, 2014
 *      Author: cady
 */

#include "external_data_structures_parsers.hpp"
#include "yaml.h"
#include "SimulatorYamlParserException.hpp"
#include <ssc/yaml_parser.hpp>

template <typename T> void try_to_parse(const YAML::Node& node, const std::string& key, T& value)
{
    const YAML::Node * n = node.FindValue(key);
    if (n) (*n) >> value;
}

void parse_point_with_name(const YAML::Node& node, YamlPoint& p, const std::string& name)
{
    YamlCoordinates c;
    node >> c;
    p = c;
    node["frame"] >> p.frame;
    p.name = name;
}

void inline search_and_replace(std::string& source, const std::string& find, const std::string& replace)
{
    const size_t fLen = find.size();
    const size_t rLen = replace.size();
    for (size_t pos=0; (pos=source.find(find, pos))!=std::string::npos; pos+=rLen)
    {
        source.replace(pos, fLen, replace);
    }
}

void operator >> (const YAML::Node& node, YamlRotation& g)
{
    std::vector<std::string> s;
    node >> s;
    if (s.size() != 3)
    {
        std::stringstream ss;
        ss << "Could not parse rotations: there should be exactly three elements in the 'rotations convention' list, but " << s.size() << " were detected.";
        THROW(__PRETTY_FUNCTION__, SimulatorYamlParserException, ss.str());
    }
    bool detected_axis = false;
    bool detected_angle = false;
    std::vector<std::string> ret;
    for (size_t i = 0 ; i < 3 ; ++i)
    {
        std::string si = s.at(i);
        if (si.size() > 7)
        {
            std::stringstream ss;
            ss << "Could not parse rotations: element nb " << i << " (starting from 0) in vector was " << si;
            THROW(__PRETTY_FUNCTION__, SimulatorYamlParserException, ss.str());
        }
        const std::string si_with_no_apostrophs = si.substr(0,si.find_first_of('\''));
        if ((si_with_no_apostrophs == "x") or (si_with_no_apostrophs == "y") or (si_with_no_apostrophs == "z"))
        {
            detected_axis = true;
        }
        if ((si_with_no_apostrophs == "phi") or (si_with_no_apostrophs == "theta") or (si_with_no_apostrophs == "psi"))
        {
            detected_angle = true;
            search_and_replace(si, "phi", "x");
            search_and_replace(si, "theta", "y");
            search_and_replace(si, "psi", "z");
        }
        if (not(detected_axis or detected_angle))
        {
            std::stringstream ss;
            ss << "Could not parse rotations: could not detect convention for element nb " << i << " (starting from 0) in vector, which was " << si;
            THROW(__PRETTY_FUNCTION__, SimulatorYamlParserException, ss.str());
        }
        g.convention.push_back(si);
    }
    if (detected_axis) g.order_by = "axis";
    if (detected_angle) g.order_by = "angle";
}

void operator >> (const YAML::Node& node, YamlBody& b)
{
    node["name"] >> b.name;
    try_to_parse(node, "mesh", b.mesh);
    try_to_parse(node, "external forces", b.external_forces);
    try_to_parse(node, "controlled forces", b.controlled_forces);
    node["position of body frame relative to mesh"]             >> b.position_of_body_frame_relative_to_mesh;
    node["initial position of body frame relative to NED"]      >> b.initial_position_of_body_frame_relative_to_NED_projected_in_NED;
    node["initial velocity of body frame relative to NED"]      >> b.initial_velocity_of_body_frame_relative_to_NED_projected_in_body;
    node["dynamics"]                                            >> b.dynamics;
}

void operator >> (const YAML::Node& node, YamlModel& m)
{
    node["model"] >> m.model;
    YAML::Emitter out;
    out << node;
    m.yaml = out.c_str();
}

void operator >> (const YAML::Node& node, YamlPosition& p)
{
    node >> p.angle;
    node >> p.coordinates;
    node["frame"] >> p.frame;
}

void operator >> (const YAML::Node& node, YamlAngle& a)
{
    ssc::yaml_parser::parse_uv(node["phi"], a.phi);
    ssc::yaml_parser::parse_uv(node["theta"], a.theta);
    ssc::yaml_parser::parse_uv(node["psi"], a.psi);
}

void operator >> (const YAML::Node& node, YamlCoordinates& c)
{
    ssc::yaml_parser::parse_uv(node["x"], c.x);
    ssc::yaml_parser::parse_uv(node["y"], c.y);
    ssc::yaml_parser::parse_uv(node["z"], c.z);
}

void operator >> (const YAML::Node& node, YamlSpeed& s)
{
    node["frame"] >> s.frame;
    ssc::yaml_parser::parse_uv(node["u"], s.u);
    ssc::yaml_parser::parse_uv(node["v"], s.v);
    ssc::yaml_parser::parse_uv(node["w"], s.w);
    ssc::yaml_parser::parse_uv(node["p"], s.p);
    ssc::yaml_parser::parse_uv(node["q"], s.q);
    ssc::yaml_parser::parse_uv(node["r"], s.r);
}

void operator >> (const YAML::Node& node, YamlDynamics& d)
{
    parse_point_with_name(node["centre of inertia"], d.centre_of_inertia, "centre of inertia");
    ssc::yaml_parser::parse_uv(node["mass"], d.mass);
    node["rigid body inertia matrix at the center of buoyancy projected in the body frame"] >> d.rigid_body_inertia;
    node["added mass matrix at the center of buoyancy projected in the body frame"] >> d.added_mass;
    node["hydrodynamic forces calculation point in body frame"] >> d.hydrodynamic_forces_calculation_point_in_body_frame;
}

void operator >> (const YAML::Node& node, YamlPoint& p)
{
    std::string name;
    node["name"] >> name;
    parse_point_with_name(node, p,name);
}


void operator >> (const YAML::Node& node, YamlDynamics6x6Matrix& m)
{
    if (const YAML::Node *parameter = node.FindValue("from hdb file"))
    {
        if (   node.FindValue("frame")
            or node.FindValue("row 1")
            or node.FindValue("row 2")
            or node.FindValue("row 3")
            or node.FindValue("row 4")
            or node.FindValue("row 5")
            or node.FindValue("row 6"))
        {
            THROW(__PRETTY_FUNCTION__, SimulatorYamlParserException, "In node 'added mass matrix at the center of buoyancy projected in the body frame': cannot specify both an HDB filename & a matrix.");
        }
        m.read_from_file = true;
        *parameter >> m.hdb_filename;
    }
    else
    {
        node["frame"] >> m.frame;
        node["row 1"] >> m.row_1;
        node["row 2"] >> m.row_2;
        node["row 3"] >> m.row_3;
        node["row 4"] >> m.row_4;
        node["row 5"] >> m.row_5;
        node["row 6"] >> m.row_6;
    }
}

void operator >> (const YAML::Node& node, YamlEnvironmentalConstants& f)
{
    ssc::yaml_parser::parse_uv(node["g"], f.g);
    ssc::yaml_parser::parse_uv(node["rho"], f.rho);
    ssc::yaml_parser::parse_uv(node["nu"], f.nu);
}

void operator >> (const YAML::Node& node, BlockableState& g);
void operator >> (const YAML::Node& node, BlockableState& g)
{
    std::string t;
    node >> t;
    if      (t == "u") g = BlockableState::U;
    else if (t == "v") g = BlockableState::V;
    else if (t == "w") g = BlockableState::W;
    else if (t == "p") g = BlockableState::P;
    else if (t == "q") g = BlockableState::Q;
    else if (t == "r") g = BlockableState::R;
    else
    {
        THROW(__PRETTY_FUNCTION__, SimulatorYamlParserException, "Unrecognized state: '" << t << "'. Has to be one of 'u', 'v', 'w', 'p', 'q' or 'r'.");
    }
}

void operator >> (const YAML::Node& node, InterpolationType& g);
void operator >> (const YAML::Node& node, InterpolationType& g)
{
    std::string t;
    node >> t;
    if      (t == "piecewise constant") g = InterpolationType::PIECEWISE_CONSTANT;
    else if (t == "linear")             g = InterpolationType::LINEAR;
    else if (t == "spline")             g = InterpolationType::SPLINE;
    else
    {
        THROW(__PRETTY_FUNCTION__, SimulatorYamlParserException, "Unrecognized interpolation type: '" << t << "'. Has to be one of 'piecewise constant', 'linear', 'spline'");
    }
}

void operator >> (const YAML::Node& node, YamlCSVDOF& g);
void operator >> (const YAML::Node& node, YamlCSVDOF& g)
{
    node["filename"]      >> g.filename;
    node["interpolation"] >> g.interpolation;
    node["state"]         >> g.state;
    node["t"]             >> g.t;
    node["value"]         >> g.value;
}

void operator >> (const YAML::Node& node, YamlDOF<std::vector<double> >& g);
void operator >> (const YAML::Node& node, YamlDOF<std::vector<double> >& g)
{
    node["interpolation"] >> g.interpolation;
    node["state"]         >> g.state;
    node["t"]             >> g.t;
    node["value"]         >> g.value;
}

YamlBlockedDOF parse(const std::string& yaml)
{
    std::stringstream stream(yaml);
    YAML::Parser parser(stream);
    YAML::Node node;
    parser.GetNextDocument(node);
    YamlBlockedDOF ret;

    if (node.FindValue("blocked dof"))
    {
        if (node["blocked dof"].FindValue("from CSV"))  node["blocked dof"]["from CSV"]  >> ret.from_csv;
        if (node["blocked dof"].FindValue("from YAML")) node["blocked dof"]["from YAML"] >> ret.from_yaml;
    }
    return ret;
}
