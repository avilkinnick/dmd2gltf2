#include "Application.h"

#include "cmake_defines.h"
#include "logging.h"

#include "tiny_gltf.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cstddef>
#include <cstdint>

struct Vec2
{
    float x;
    float y;
};

struct Vec3
{
    float x;
    float y;
    float z;
};

struct Vertex
{
    Vec3 pos;
    Vec2 texcoord;
};

struct ObjectRef
{
    std::string label;
    std::string model_path;
    std::string texture_path;
};

bool is_slash(char ch);
void replace_chars(std::string& str, char ch1, char ch2);
void string_to_lower(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char ch) {
        return std::tolower(ch);
    });
}

void print_valid_usage();

void convert_model(
    const std::string& in_model_path,
    const std::string& out_model_name,
    const std::string& out_model_directory,
    const std::string& texture_path
);

Application::Application(int argc, char* argv[])
{
    parse_arguments(argc, argv);
}

void Application::run()
{
    if (route_name.empty())
    {
        for (const std::string& model_name : model_names)
        {
            // convert_model(std::string(DMD_MODELS_DIR) + '/' + model_name, model_name, GLTF_MODELS_DIR);
        }
    }
    else
    {
        convert_route();
    }
}

void Application::parse_arguments(int argc, char* argv[])
{
    if (argc < 2)
    {
        LOG_FATAL("Invalid arguments");
        print_valid_usage();
        throw 1;
    }

    std::string arg2 = argv[1];
    if (arg2 == "-r")
    {
        if (argc != 3)
        {
            LOG_FATAL("Invalid arguments");
            print_valid_usage();
            throw 1;
        }

        route_name = argv[2];
    }
    else
    {
        model_names.reserve(argc - 1);
        for (int i = 1; i < argc; ++i)
        {
            std::string model_name = argv[i];
            model_names.emplace_back(model_name.substr(0, model_name.length() - 4));
        }
    }
}

void to_lower_dir(const std::filesystem::path& path)
{
    std::filesystem::directory_iterator dir_it(path);
    for (auto& entry : dir_it)
    {
        std::string filename = entry.path().filename().string();
        string_to_lower(filename);
        std::string new_path_str = entry.path().parent_path().string() + '/' + filename;
        std::filesystem::path new_path(new_path_str);
        std::filesystem::rename(entry.path(), new_path);

        if (entry.is_directory())
        {
            to_lower_dir(new_path);
        }
    }
}

void Application::convert_route()
{
    to_lower_dir(std::string(DMD_ROUTES_DIR) + '/' + route_name + "/models");
    to_lower_dir(std::string(DMD_ROUTES_DIR) + '/' + route_name + "/textures");

    std::ifstream objects_ref(std::string(DMD_ROUTES_DIR) + '/' + route_name + "/objects.ref");
    if (!objects_ref)
    {
        LOG_ERROR("Failed to open objects.ref");
        return;
    }

    std::vector<ObjectRef> object_refs;

    std::string line;
    while (std::getline(objects_ref, line))
    {
        std::istringstream line_stream(line);
        ObjectRef ref;
        line_stream >> ref.label >> ref.model_path >> ref.texture_path;
        if (line_stream
            && !ref.texture_path.empty()
            && is_slash(ref.model_path.front())
            && is_slash(ref.texture_path.front())
        )
        {
            replace_chars(ref.model_path, '\\', '/');
            replace_chars(ref.texture_path, '\\', '/');
            string_to_lower(ref.model_path);
            string_to_lower(ref.texture_path);
            object_refs.emplace_back(std::move(ref));
        }
    }

    objects_ref.close();

    std::string new_route_dir = std::string(GLTF_ROUTES_DIR) + '/' + route_name;

    if (!std::filesystem::create_directory(new_route_dir)
        && !std::filesystem::exists(new_route_dir))
    {
        LOG_ERROR("Failed to create folder for route %s", route_name.c_str());
        return;
    }

    if (!std::filesystem::create_directory(new_route_dir + "/models")
        && !std::filesystem::exists(new_route_dir + "/models"))
    {
        LOG_ERROR("Failed to create folder for models for route %s", route_name.c_str());
        return;
    }

    if (!std::filesystem::create_directory(new_route_dir + "/textures")
        && !std::filesystem::exists(new_route_dir + "/models"))
    {
        LOG_ERROR("Failed to create folder for textures for route %s", route_name.c_str());
        return;
    }

    for (ObjectRef& ref : object_refs)
    {
        auto second_slash_pos = ref.model_path.substr(1).find_first_of('/') + 1;
        std::string out_model_name = ref.model_path.substr(second_slash_pos + 1);
        out_model_name = out_model_name.substr(0, out_model_name.length() - 4);

        std::ifstream texture_file(std::string(DMD_ROUTES_DIR) + '/' + route_name + ref.texture_path);
        if (texture_file)
        {
            if (!std::filesystem::exists(new_route_dir + ref.texture_path))
            {
                std::filesystem::copy_file(
                    std::string(DMD_ROUTES_DIR) + '/' + route_name + ref.texture_path,
                    new_route_dir + ref.texture_path
                );
            }
        }
        else
        {
            LOG_ERROR("No texture %s in original route", ref.texture_path.c_str());
            continue;
        }

        // auto last_slash_pos = ref.model_path.find_last_of('/');
        // std::string path1 = ref.model_path.substr(0, last_slash_pos);
        // std::string model_name =
        replace_chars(out_model_name, '/', '_');
        convert_model(
            std::string(DMD_ROUTES_DIR) + '/' + route_name + ref.model_path,
            out_model_name,
            new_route_dir + "/models",
            ref.texture_path
        );
        ref.model_path = "/models/" + out_model_name + ".gltf";
    }

    std::ofstream new_objects_ref(std::ofstream(new_route_dir + "/objects.ref"));
    if (!new_objects_ref)
    {
        LOG_ERROR("Failed to crete new objects.ref for route %s", route_name.c_str());
        return;
    }

    for (const ObjectRef& ref : object_refs)
    {
        new_objects_ref << ref.label << "    " << ref.model_path << '\n';
    }

    new_objects_ref.close();
}

bool is_slash(char ch)
{
    return ch == '/' || ch == '\\';
}

void replace_chars(std::string& str, char ch1, char ch2)
{
    std::replace(str.begin(), str.end(), ch1, ch2);
}

void print_valid_usage()
{
    LOG_INFO(
        "Valid usage:\n"
        "    dmd2gltf model1.dmd model2.dmd ...\n"
        "    dmd2gltf -r route_name"
    );
}

void convert_model(
    const std::string& in_model_path,
    const std::string& out_model_name,
    const std::string& out_model_directory,
    const std::string& texture_path
)
{
    std::ifstream dmd_file(in_model_path);
    if (!dmd_file)
    {
        LOG_ERROR("Failed to open %s", in_model_path.c_str());
        return;
    }

    std::string buffer;
    while (buffer != "TriMesh()")
    {
        dmd_file >> buffer;
        if (!dmd_file)
        {
            LOG_ERROR("Failed to find TriMesh() in %s", in_model_path.c_str());
            return;
        }
    }

    dmd_file >> buffer >> buffer;

    std::uint32_t position_count, position_face_count;
    dmd_file >> position_count >> position_face_count;

    dmd_file >> buffer >> buffer;

    std::vector<Vec3> positions(position_count);
    for (Vec3& position : positions)
    {
        dmd_file >> position.x >> position.y >> position.z;
    }

    if (!dmd_file)
    {
        LOG_ERROR("Failed to read position values from %s", in_model_path.c_str());
        return;
    }

    dmd_file >> buffer >> buffer >> buffer >> buffer;

    std::vector<std::uint32_t> position_indices(position_face_count * 3);
    for (std::uint32_t& index : position_indices)
    {
        dmd_file >> index;
        --index;
    }

    if (!dmd_file)
    {
        LOG_ERROR("Failed to read position indices from %s", in_model_path.c_str());
        return;
    }

    while (buffer != "Texture:")
    {
        dmd_file >> buffer;
        if (!dmd_file)
        {
            LOG_ERROR("Failed to find Texture: in %s", in_model_path.c_str());
            return;
        }
    }

    dmd_file >> buffer >> buffer;

    std::uint32_t texcoord_count, texcoord_face_count;
    dmd_file >> texcoord_count >> texcoord_face_count;

    if (position_face_count != texcoord_face_count)
    {
        LOG_ERROR("Position face count and texture face count are not equal");
        return;
    }

    std::uint32_t face_count = position_face_count;

    dmd_file >> buffer >> buffer;

    std::vector<Vec2> texcoords(texcoord_count);
    for (Vec2& texcoord : texcoords)
    {
        dmd_file >> texcoord.x >> texcoord.y >> buffer;
    }

    if (!dmd_file)
    {
        LOG_ERROR("Failed to read texcoord values from %s", in_model_path.c_str());
        return;
    }

    dmd_file >> buffer >> buffer >> buffer >> buffer >> buffer;

    std::vector<std::uint32_t> textcoord_indices(face_count * 3);
    for (std::uint32_t& index : textcoord_indices)
    {
        dmd_file >> index;
        --index;
    }

    if (!dmd_file)
    {
        LOG_ERROR("Failed to read texture indices from %s", in_model_path.c_str());
        return;
    }

    dmd_file.close();

    for (Vec3& position : positions)
    {
        std::swap(position.y, position.z);
        position.z = -position.z;
    }

    std::vector<Vertex> vertices;
    vertices.reserve(face_count * 3);

    std::vector<std::uint32_t> indices;
    indices.reserve(face_count * 3);

    std::map<std::pair<std::uint32_t, std::uint32_t>, std::uint32_t> unique_indices;

    for (std::uint32_t i = 0; i < face_count * 3; ++i)
    {
        std::uint32_t position_index = position_indices[i];
        std::uint32_t texcoord_index = textcoord_indices[i];

        auto found_it = unique_indices.find(std::make_pair(position_index, texcoord_index));
        if (found_it == unique_indices.end())
        {
            vertices.emplace_back(Vertex{positions[position_index], texcoords[texcoord_index]});
            indices.emplace_back(vertices.size() - 1);
            unique_indices.insert({std::make_pair(position_index, texcoord_index), vertices.size() - 1});
        }
        else
        {
            indices.emplace_back(found_it->second);
        }
    }

    vertices.shrink_to_fit();

    std::string bin_file_path = out_model_directory + '/' + out_model_name + ".bin";

    std::ofstream bin_file(bin_file_path, std::ios::binary);
    if (!bin_file)
    {
        LOG_ERROR("Failed to open bin file for %s", out_model_name.c_str());
        return;
    }

    for (const Vertex& vertex : vertices)
    {
        bin_file.write(reinterpret_cast<const char*>(&vertex.pos), sizeof(Vec3));
    }
    std::size_t positions_byte_length = bin_file.tellp();

    for (const Vertex& vertex : vertices)
    {
        bin_file.write(reinterpret_cast<const char*>(&vertex.texcoord), sizeof(Vec2));
    }
    std::size_t texcoords_byte_length = bin_file.tellp();
    texcoords_byte_length -= positions_byte_length;

    for (std::uint32_t index : indices)
    {
        bin_file.write(reinterpret_cast<const char*>(&index), sizeof(std::uint32_t));
    }

    std::size_t indices_byte_length = bin_file.tellp();
    indices_byte_length -= positions_byte_length + texcoords_byte_length;

    bin_file.close();

    tinygltf::Model gltf_model;

    auto& gltf_buffer = gltf_model.buffers.emplace_back();
    gltf_buffer.data.reserve(positions_byte_length + texcoords_byte_length + indices_byte_length);
    gltf_buffer.uri = out_model_name + ".bin";

    std::ifstream bin_data(bin_file_path, std::ios::binary);
    unsigned char byte;
    while (bin_data.read(reinterpret_cast<char*>(&byte), sizeof(unsigned char)))
    {
        gltf_buffer.data.emplace_back(byte);
    }

    bin_data.close();

    auto& positions_buffer_view = gltf_model.bufferViews.emplace_back();
    positions_buffer_view.buffer = 0;
    positions_buffer_view.byteLength = positions_byte_length;
    positions_buffer_view.byteOffset = 0;
    positions_buffer_view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    auto& texcoord_buffer_view = gltf_model.bufferViews.emplace_back();
    texcoord_buffer_view.buffer = 0;
    texcoord_buffer_view.byteLength = texcoords_byte_length;
    texcoord_buffer_view.byteOffset = positions_byte_length;
    texcoord_buffer_view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    auto& indices_buffer_view = gltf_model.bufferViews.emplace_back();
    indices_buffer_view.buffer = 0;
    indices_buffer_view.byteLength = indices_byte_length;
    indices_buffer_view.byteOffset = positions_byte_length + texcoords_byte_length;
    indices_buffer_view.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

    auto& positions_accessor = gltf_model.accessors.emplace_back();
    positions_accessor.bufferView = 0;
    positions_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    positions_accessor.count = vertices.size();
    positions_accessor.type = TINYGLTF_TYPE_VEC3;

    auto& texcoords_accessor = gltf_model.accessors.emplace_back();
    texcoords_accessor.bufferView = 1;
    texcoords_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    texcoords_accessor.count = vertices.size();
    texcoords_accessor.type = TINYGLTF_TYPE_VEC2;

    auto& indices_accessor = gltf_model.accessors.emplace_back();
    indices_accessor.bufferView = 2;
    indices_accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    indices_accessor.count = indices.size();
    indices_accessor.type = TINYGLTF_TYPE_SCALAR;

    auto& image = gltf_model.images.emplace_back();
    image.uri = std::string("..") + texture_path;

    auto& sampler = gltf_model.samplers.emplace_back();
    sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
    sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
    sampler.wrapS = TINYGLTF_TEXTURE_WRAP_REPEAT;
    sampler.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;

    auto& texture = gltf_model.textures.emplace_back();
    texture.sampler = 0;
    texture.source = 0;

    auto& material = gltf_model.materials.emplace_back();
    material.pbrMetallicRoughness.baseColorTexture.index = 0;
    material.pbrMetallicRoughness.baseColorTexture.texCoord = 0;
    material.alphaMode = "MASK";

    auto& mesh = gltf_model.meshes.emplace_back();
    auto& primitive = mesh.primitives.emplace_back();
    primitive.attributes.insert({"POSITION", 0});
    primitive.attributes.insert({"TEXCOORD_0", 1});
    primitive.material = 0;
    primitive.indices = 2;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    tinygltf::Scene a;

    auto& node = gltf_model.nodes.emplace_back();
    node.name = out_model_name;
    node.mesh = 0;

    auto& scene = gltf_model.scenes.emplace_back();
    scene.nodes = {0};

    gltf_model.defaultScene = 0;

    tinygltf::TinyGLTF gltf;
    gltf.WriteGltfSceneToFile(&gltf_model, out_model_directory + '/' + out_model_name + ".gltf", true, false, true, false);
}
