add_library(thirdparty_concurrentqueue INTERFACE
        include/concurrentqueue/blockingconcurrentqueue.h
        include/concurrentqueue/concurrentqueue.h
        include/concurrentqueue/lightweightsemaphore.h
        include/concurrentqueue/internal/concurrentqueue_internal_debug.h
)
add_library(thirdparty::concurrentqueue ALIAS thirdparty_concurrentqueue)
target_include_directories(thirdparty_concurrentqueue INTERFACE include)
set_target_properties(thirdparty_concurrentqueue PROPERTIES VERSION 1.0.5)