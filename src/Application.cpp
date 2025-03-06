#include "Application.h"

#include "Geometry.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using std::string_literals::operator""s;

static bool is_slash(char ch)
{
    return ch == '/' || ch == '\\';
}

bool Application::parse_args(int argc, char* argv[])
{
    switch (argc)
    {
        case 3:
        {
            convert_mode = CONVERT_ROUTE;
            in_dmd_route_path = argv[1];
            out_gltf_route_path = argv[2];

            return true;
        }
        case 6:
        {
            convert_mode = CONVERT_MODEL;
            in_dmd_model_path = argv[1];
            in_texture_path = argv[2];
            out_gltf_model_path = argv[3];
            out_relative_bin_path = argv[4];
            out_relative_texture_path = argv[5];

            return true;
        }
        default:
        {
            std::cerr << "Valid usage:\n"
                "    dmd2gltf in_dmd_route_path out_gltf_route_path\n"
                "    dmd2gltf in_dmd_model_path in_texture_path out_gltf_model_path out_relative_bin_path out_relative_texture_path" << std::endl;

            return false;
        }
    }
}

bool Application::convert()
{
    return (convert_mode == CONVERT_ROUTE) ? convert_route() : convert_model();
}

bool Application::convert_route()
{
    std::replace(in_dmd_route_path.begin(), in_dmd_route_path.end(), '\\', '/');
    std::replace(out_gltf_route_path.begin(), out_gltf_route_path.end(), '\\', '/');

    while (in_dmd_route_path.back() == '/')
    {
        in_dmd_route_path.pop_back();
    }

    while (out_gltf_route_path.back() == '/')
    {
        out_gltf_route_path.pop_back();
    }

    std::ifstream objects_ref(in_dmd_route_path + "/objects.ref");
    if (!objects_ref)
    {
        std::cerr << "Failed to open objects.ref" << std::endl;
        return false;
    }

    using Label = std::string;
    using RelativePath = std::string;
    std::map<Label, std::pair<RelativePath, RelativePath>> objects;

    std::string buffer;
    while (std::getline(objects_ref, buffer))
    {
        std::istringstream ss(buffer);
        std::string label, relative_dmd_model_path, relative_texture_path;
        ss >> label >> relative_dmd_model_path >> relative_texture_path;
        if (ss && !is_slash(label.front()) && is_slash(relative_dmd_model_path.front()) && is_slash(relative_texture_path.front()))
        {
            std::replace(relative_dmd_model_path.begin(), relative_dmd_model_path.end(), '\\', '/');
            std::replace(relative_texture_path.begin(), relative_texture_path.end(), '\\', '/');

            objects.insert({label, {relative_dmd_model_path, relative_texture_path}});
        }
    }

    objects_ref.close();

    if (objects.empty())
    {
        std::cerr << "Failed to find objects in objects.ref" << std::endl;
        return false;
    }

    std::filesystem::create_directories(out_gltf_route_path);
    std::filesystem::create_directory(out_gltf_route_path + "/textures");

    if (!std::filesystem::exists(out_gltf_route_path + "/topology"))
    {
        std::filesystem::copy(in_dmd_route_path + "/topology", out_gltf_route_path + "/topology");
    }

    if (!std::filesystem::exists(out_gltf_route_path + "/description.xml"))
    {
        std::filesystem::copy(in_dmd_route_path + "/description.xml", out_gltf_route_path + "/description.xml");
    }

    if (!std::filesystem::exists(out_gltf_route_path + "/route1.map"))
    {
        std::filesystem::copy(in_dmd_route_path + "/route1.map", out_gltf_route_path + "/route1.map");
    }

    std::map<Label, RelativePath> new_objects;

    for (const auto& [label, paths] : objects)
    {
        in_dmd_model_path = in_dmd_route_path + paths.first;
        in_texture_path = in_dmd_route_path + paths.second;

        // std::string lower_model_path = paths.first;
        // std::string lower_texture_path = paths.second;

        // for (char& ch : lower_model_path)
        // {
        //     ch = std::tolower(ch);
        // }
        // for (char& ch : lower_texture_path)
        // {
        //     ch = std::tolower(ch);
        // }

        std::filesystem::path model_path = paths.first;
        std::filesystem::path texture_path = paths.second;

        std::filesystem::create_directories(out_gltf_route_path + model_path.parent_path().string() + "/bin");
        out_gltf_model_path = out_gltf_route_path + model_path.parent_path().string() + '/' + model_path.stem().string() + ".gltf";
        out_relative_bin_path = "bin/"s + model_path.stem().string() + ".bin";

        std::string mps = model_path.string();
        auto slash_count = std::count(mps.begin(), mps.end(), '/');
        out_relative_texture_path = "";
        for (int i = 1; i < slash_count; ++i)
        {
            out_relative_texture_path += "../";
        }
        out_relative_texture_path += "textures/" + texture_path.filename().string();

        if (convert_model())
        {
            new_objects.insert({label, model_path.parent_path().string() + '/' + model_path.stem().string() + ".gltf"});
        }

        // std::cout << in_dmd_model_path << '\n'
        //     << in_texture_path << '\n'
        //     << out_gltf_model_path << '\n'
        //     << out_relative_bin_path << '\n'
        //     << out_relative_texture_path << "\n\n";

        // std::filesystem::path z(paths.first);
        // std::cout <<
        //     "Root name: " << z.root_name() << "\n"
        //     "Root directory: " << z.root_directory() << "\n"
        //     "Root path: " << z.root_path() << "\n"
        //     "Relative path: " << z.relative_path() << "\n"
        //     "Parent path: " << z.parent_path() << "\n"
        //     "Filename: " << z.filename() << "\n"
        //     "Stem: " << z.stem() << "\n"
        //     "Extension: " << z.extension() << "\n\n";

        // std::cout << label << "    " << paths.first << "    " << paths.second << std::endl;
    }

    std::ofstream new_objects_ref(out_gltf_route_path + "/objects.ref");
    if (!new_objects_ref)
    {
        std::cerr << "Failed to open new objects.ref" << std::endl;
        return false;
    }

    for (const auto& [label, path] : new_objects)
    {
        new_objects_ref << label << "    " << path << '\n';
    }

    return true;
}

bool Application::convert_model()
{
    std::ifstream texture(in_texture_path);
    if (!texture)
    {
        std::cerr << "Failed to open " << in_texture_path << std::endl;
        return false;
    }
    // std::cout <<
        // "\nin_model: " << in_dmd_model_path << "\n"
        // "in_texture: " << in_texture_path << "\n"
        // "out_gltf: " << out_gltf_model_path << "\n"
        // "out_bin: " << out_relative_bin_path << "\n"
        // "out_texture: " << out_relative_texture_path << "\n";

    auto last_slash_pos = out_gltf_model_path.find_last_of('/');
    if (last_slash_pos == std::string::npos)
    {
        gltf_directory_path = ".";
    }
    else
    {
        gltf_directory_path = out_gltf_model_path.substr(0, last_slash_pos);
    }

    Geometry model_data;
    if (!get_dmd_model_data(model_data))
    {
        return false;
    }

    return generate_gltf_model(model_data);
}

bool Application::get_dmd_model_data(Geometry& model_data)
{
    using PosIndex = std::uint32_t;
    using TexIndex = std::uint32_t;
    using VertexIndex = std::uint32_t;

    std::ifstream model_file(in_dmd_model_path);
    if (!model_file)
    {
        std::cerr << "Failed to open " << in_dmd_model_path << std::endl;
        return false;
    }

    std::string buffer;
    while (buffer != "TriMesh()")
    {
        model_file >> buffer;
        if (!model_file)
        {
            std::cerr << "Failed to find \"TriMesh()\" in " << in_dmd_model_path << std::endl;
            return false;
        }
    }

    model_file >> buffer >> buffer;

    std::uint32_t pos_count, pos_face_count;
    model_file >> pos_count >> pos_face_count;

    model_file >> buffer >> buffer;

    std::vector<Vec3> positions(pos_count);
    for (auto& pos : positions)
    {
        model_file >> pos.x >> pos.y >> pos.z;
    }

    if (!model_file)
    {
        std::cerr << "Failed to read positions from " << in_dmd_model_path << std::endl;
    }

    model_file >> buffer >> buffer >> buffer >> buffer;

    std::vector<PosIndex> pos_indices(pos_face_count * 3);
    for (auto& index : pos_indices)
    {
        model_file >> index;
        --index;
    }

    if (!model_file)
    {
        std::cerr << "Failed to read position indices from " << in_dmd_model_path << std::endl;
        return false;
    }

    while (buffer != "Texture:")
    {
        model_file >> buffer;
        if (!model_file)
        {
            std::cerr << "Failed to find \"Texture:\" in " << in_dmd_model_path << std::endl;
            return false;
        }
    }

    model_file >> buffer >> buffer;

    std::uint32_t tex_coord_count, tex_face_count;
    model_file >> tex_coord_count >> tex_face_count;

    if (pos_face_count != tex_face_count)
    {
        std::cerr << "Position face count is not equal to texture face count in " << in_dmd_model_path << std::endl;
        return false;
    }

    std::uint32_t face_count = pos_face_count;

    model_file >> buffer >> buffer;

    std::vector<Vec2> tex_coords(tex_coord_count);
    for (auto& tex_coord : tex_coords)
    {
        model_file >> tex_coord.x >> tex_coord.y >> buffer;
    }

    if (!model_file)
    {
        std::cerr << "Failed to read texture coordinates from " << in_dmd_model_path << std::endl;
        return false;
    }

    model_file >> buffer >> buffer >> buffer >> buffer >> buffer;

    std::vector<TexIndex> tex_indices(face_count * 3);
    for (auto& index : tex_indices)
    {
        model_file >> index;
        --index;
    }

    if (!model_file)
    {
        std::cerr << "Failed to read texture indices from " << in_dmd_model_path << std::endl;
        return false;
    }

    model_file.close();

    model_data.vertices.reserve(face_count * 3);
    model_data.indices.resize(face_count * 3);

    std::map<std::pair<PosIndex, TexIndex>, VertexIndex> unique_indices;

    for (std::uint32_t i = 0; i < face_count * 3; ++i)
    {
        PosIndex pos_index = pos_indices[i];
        TexIndex tex_index = tex_indices[i];

        auto found_it = unique_indices.find({pos_index, tex_index});
        if (found_it == unique_indices.end())
        {
            model_data.vertices.emplace_back(Vertex{positions[pos_index], tex_coords[tex_index]});

            VertexIndex new_vertex_index = model_data.vertices.size() - 1;
            model_data.indices[i] = new_vertex_index;
            unique_indices.insert({{pos_index, tex_index}, new_vertex_index});
        }
        else
        {
            model_data.indices[i] = found_it->second;
        }
    }

    model_data.vertices.shrink_to_fit();

    return true;
}

bool Application::generate_gltf_model(Geometry& model_data)
{
    for (auto& vertex : model_data.vertices)
    {
        std::swap(vertex.pos.y, vertex.pos.z);
        vertex.pos.z = -vertex.pos.z;
    }

    std::string full_bin_path = gltf_directory_path + '/' + out_relative_bin_path;

    std::ofstream bin_file(full_bin_path, std::ios::binary);
    if (!bin_file)
    {
        std::cerr << "Failed to open " << full_bin_path << std::endl;
        return false;
    }

    for (const auto& vertex : model_data.vertices)
    {
        bin_file.write(reinterpret_cast<const char*>(&vertex.pos), sizeof(vertex.pos));
    }

    auto positions_byte_length = bin_file.tellp();

    for (const auto& vertex : model_data.vertices)
    {
        bin_file.write(reinterpret_cast<const char*>(&vertex.tex_coord), sizeof(vertex.tex_coord));
    }

    auto tex_coords_byte_length = bin_file.tellp() - positions_byte_length;

    for (auto index : model_data.indices)
    {
        bin_file.write(reinterpret_cast<const char*>(&index), sizeof(index));
    }

    auto indices_byte_length = bin_file.tellp() - tex_coords_byte_length - positions_byte_length;

    bin_file.close();

    Vec3 min_pos, max_pos;
    Vec2 min_tex, max_tex;
    min_pos = max_pos = model_data.vertices[0].pos;
    min_tex = max_tex = model_data.vertices[0].tex_coord;

    for (const auto& vertex : model_data.vertices)
    {
        min_pos.x = std::min(min_pos.x, vertex.pos.x);
        min_pos.y = std::min(min_pos.y, vertex.pos.y);
        min_pos.z = std::min(min_pos.z, vertex.pos.z);

        max_pos.x = std::max(max_pos.x, vertex.pos.x);
        max_pos.y = std::max(max_pos.y, vertex.pos.y);
        max_pos.z = std::max(max_pos.z, vertex.pos.z);

        min_tex.x = std::min(min_tex.x, vertex.tex_coord.x);
        min_tex.y = std::min(min_tex.y, vertex.tex_coord.y);

        max_tex.x = std::max(max_tex.x, vertex.tex_coord.x);
        max_tex.y = std::max(max_tex.y, vertex.tex_coord.y);
    }

    std::ofstream gltf_file(out_gltf_model_path);
    if (!gltf_file)
    {
        std::cerr << "Failed to open " << out_gltf_model_path << std::endl;
        return false;
    }

    gltf_file << "{\n"
        "    \"asset\": {\n"
        "        \"version\": \"2.0\"\n"
        "    },\n"
        "    \"buffers\": [\n"
        "        {\n"
        "            \"uri\": \"" << out_relative_bin_path << "\",\n"
        "            \"byteLength\": " << positions_byte_length + tex_coords_byte_length + indices_byte_length << "\n"
        "        }\n"
        "    ],\n"
        "    \"bufferViews\": [\n"
        "        {\n"
        "            \"buffer\": 0,\n"
        "            \"byteOffset\": 0,\n"
        "            \"byteLength\": " << positions_byte_length << ",\n"
        "            \"target\": 34962\n"
        "        },\n"
        "        {\n"
        "            \"buffer\": 0,\n"
        "            \"byteOffset\": " << positions_byte_length << ",\n"
        "            \"byteLength\": " << tex_coords_byte_length << ",\n"
        "            \"target\": 34962\n"
        "        },\n"
        "        {\n"
        "            \"buffer\": 0,\n"
        "            \"byteOffset\": " << positions_byte_length + tex_coords_byte_length << ",\n"
        "            \"byteLength\": " << indices_byte_length << ",\n"
        "            \"target\": 34963\n"
        "        }\n"
        "    ],\n"
        "    \"accessors\": [\n"
        "        {\n"
        "            \"bufferView\": 0,\n"
        "            \"componentType\": 5126,\n"
        "            \"count\": " << model_data.vertices.size() << ",\n"
        "            \"type\": \"VEC3\",\n"
        "            \"max\": [\n"
        "                " << max_pos.x << ",\n"
        "                " << max_pos.y << ",\n"
        "                " << max_pos.z << "\n"
        "            ],\n"
        "            \"min\": [\n"
        "                " << min_pos.x << ",\n"
        "                " << min_pos.y << ",\n"
        "                " << min_pos.z << "\n"
        "            ]\n"
        "        },\n"
        "        {\n"
        "            \"bufferView\": 1,\n"
        "            \"componentType\": 5126,\n"
        "            \"count\": " << model_data.vertices.size() << ",\n"
        "            \"type\": \"VEC2\",\n"
        "            \"max\": [\n"
        "                " << max_tex.x << ",\n"
        "                " << max_tex.y << "\n"
        "            ],\n"
        "            \"min\": [\n"
        "                " << min_tex.x << ",\n"
        "                " << min_tex.y << "\n"
        "            ]\n"
        "        },\n"
        "        {\n"
        "            \"bufferView\": 2,\n"
        "            \"componentType\": 5125,\n"
        "            \"count\": " << model_data.indices.size() << ",\n"
        "            \"type\": \"SCALAR\",\n"
        "            \"max\": [\n"
        "                " << model_data.vertices.size() - 1 << "\n"
        "            ],\n"
        "            \"min\": [\n"
        "                0\n"
        "            ]\n"
        "        }\n"
        "    ],\n"
        "    \"images\": [\n"
        "        {\n"
        "            \"uri\": \"" << out_relative_texture_path << "\"\n"
        "        }\n"
        "    ],\n"
        "    \"samplers\": [\n"
        "        {\n"
        "            \"magFilter\": 9729,\n"
        "            \"minFilter\": 9987,\n"
        "            \"wrapS\": 10497,\n"
        "            \"wrapT\": 10497\n"
        "        }\n"
        "    ],\n"
        "    \"textures\": [\n"
        "        {\n"
        "            \"sampler\": 0,\n"
        "            \"source\": 0\n"
        "        }\n"
        "    ],\n"
        "    \"materials\": [\n"
        "        {\n"
        "            \"pbrMetallicRoughness\": {\n"
        "                \"baseColorTexture\": {\n"
        "                    \"index\": 0,\n"
        "                    \"texCoord\": 0\n"
        "                }\n"
        "            },\n"
        "            \"alphaMode\": \"BLEND\"\n"
        "        }\n"
        "    ],\n"
        "    \"meshes\": [\n"
        "        {\n"
        "            \"primitives\": [\n"
        "                {\n"
        "                    \"attributes\": {\n"
        "                        \"POSITION\": 0,\n"
        "                        \"TEXCOORD_0\": 1\n"
        "                    },\n"
        "                    \"indices\": 2,\n"
        "                    \"material\": 0,\n"
        "                    \"mode\": 4\n"
        "                }\n"
        "            ]\n"
        "        }\n"
        "    ],\n"
        "    \"nodes\": [\n"
        "        {\n"
        "            \"mesh\": 0\n"
        "        }\n"
        "    ],\n"
        "    \"scenes\": [\n"
        "        {\n"
        "            \"nodes\": [ 0 ]\n"
        "        }\n"
        "    ],\n"
        "    \"scene\": 0\n"
        "}\n";

    gltf_file.close();

    if (!std::filesystem::exists(gltf_directory_path + '/' + out_relative_texture_path))
    {
        std::filesystem::copy(in_texture_path, gltf_directory_path + '/' + out_relative_texture_path);
    }

    return true;
}
