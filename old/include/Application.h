#pragma once

#include <string>
#include <vector>

class Application
{
public:
    Application(int argc, char* argv[]);

    void run();

private:
    void parse_arguments(int argc, char* argv[]);
    void convert_route();

private:
    std::vector<std::string> model_names;
    std::string route_name;
};
