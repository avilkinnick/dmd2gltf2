#include "Application.h"
#include "logging.h"

#include <exception>

int main(int argc, char* argv[])
{
    try
    {
        Application application(argc, argv);
        application.run();
    }
    catch (const std::exception& exception)
    {
        LOG_FATAL("%s", exception.what());
        return 1;
    }
    catch (...)
    {
        return 1;
    }

    return 0;
}
