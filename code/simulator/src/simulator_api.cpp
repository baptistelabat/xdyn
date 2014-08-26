#include "SimulatorYamlParser.hpp"
#include "check_input_yaml.hpp"
#include "simulator_api.hpp"

#include "BretschneiderSpectrum.hpp"
#include "GravityForceModel.hpp"
#include "DefaultSurfaceElevation.hpp"
#include "HydrostaticForceModel.hpp"
#include "JonswapSpectrum.hpp"
#include "StlReader.hpp"
#include "SimulatorBuilder.hpp"

SimulatorBuilder get_builder(const YamlSimulatorInput& yaml)
{
    SimulatorBuilder builder(yaml);
    builder.can_parse<GravityForceModel>()
           .can_parse<DefaultSurfaceElevation>()
           .can_parse<HydrostaticForceModel>()
           .can_parse<BretschneiderSpectrum>()
           .can_parse<JonswapSpectrum>();
    return builder;
}

Sim get_system(const YamlSimulatorInput& yaml)
{
    return get_builder(yaml).build();
}

Sim get_system(const YamlSimulatorInput& input, const std::string& mesh)
{
    const auto name = input.bodies.front().name;
    MeshMap meshes;
    meshes[name] = read_stl(mesh);
    return get_system(input, meshes);
}

Sim get_system(const YamlSimulatorInput& yaml, const MeshMap& meshes)
{
    return get_builder(yaml).build(meshes);
}

Sim get_system(const std::string& yaml)
{
    return get_system(SimulatorYamlParser(yaml).parse());
}

Sim get_system(const std::string& yaml, const std::string& mesh)
{
    const auto input = SimulatorYamlParser(yaml).parse();
    return get_system(input, mesh);
}

Sim get_system(const std::string& yaml, const MeshMap& meshes)
{
    const auto input = SimulatorYamlParser(yaml).parse();
    return get_system(input, meshes);
}

MeshMap make_mesh_map(const YamlSimulatorInput& yaml, const std::string& mesh)
{
    const auto name = yaml.bodies.front().name;
    MeshMap meshes;
    meshes[name] = read_stl(mesh);
    return meshes;
}
