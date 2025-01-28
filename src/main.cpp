#include "Application.h"
#include "logging.h"

#include <exception>

int main(int argc, char* argv[])
{
    try
    {
        Application application(argc, argv);
    }
    catch (const std::exception& exception)
    {
        LOG_FATAL("%s", exception.what());
        return 1;
    }
    catch (...)
    {
        LOG_FATAL("Unknown exception");
        return 1;
    }

    return 0;
}
