#include <hasenpfote/log/console_appender.h>
#include <hasenpfote/log/rolling_file_appender.h>
#include "../../Common/logger.h"
#include "mywindow.h"

int main(void)
{
    using namespace hasenpfote::log;
    ::Logger::GetMutableInstance().AddAppender<ConsoleAppender>(std::make_shared<ConsoleAppender>());
    //::Logger::GetMutableInstance().AddAppender<RollingFileAppender>(std::make_shared<RollingFileAppender>("Logs/log.log", 3, 1024 * 1024));
    //::Logger::GetMutableInstance().RemoveAppender<RollingFileAppender>();

    try{
        MyWindow w;
        if(w.Initialize(640, 480)){
            w.MainLoop();
            return EXIT_SUCCESS;
        }
    }
    catch(const std::exception& e){
        LOG_F("Exception: " << e.what());
    }
    catch(...){
        throw;
    }
    return EXIT_FAILURE;
}