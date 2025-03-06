#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>

struct Geometry;

class Application
{
enum ConvertMode
{
    CONVERT_ROUTE,
    CONVERT_MODEL
};

public:
    bool parse_args(int argc, char* argv[]);
    bool convert();

private:
    bool convert_route();
    bool convert_model();

    bool get_dmd_model_data(Geometry& model_data);
    bool generate_gltf_model(Geometry& model_data);

private:
    ConvertMode convert_mode;

    std::string in_dmd_route_path;
    std::string in_dmd_model_path;
    std::string in_texture_path;

    std::string out_gltf_route_path;
    std::string out_gltf_model_path;
    std::string out_relative_bin_path;
    std::string out_relative_texture_path;

    std::string gltf_directory_path;
};

#endif // APPLICATION_H
