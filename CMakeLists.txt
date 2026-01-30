cmake_minimum_required(VERSION 3.16)
project(TelegramNotifierPlugin
  VERSION 1.0.3
  LANGUAGES CXX
)
add_compile_definitions(TelegramNotifierPlugin_VERSION="${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(TelegramNotifierPlugin SHARED
  src/TelegramNotifierPlugin.cpp
)

target_compile_definitions(TelegramNotifierPlugin PRIVATE $<$<COMPILE_LANGUAGE:CXX>:LOG_NAME="TelegramNotifier">)

# Put resulting .so into workspace/Plugins
set_target_properties(TelegramNotifierPlugin PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Plugins"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Plugins"
  OUTPUT_NAME "TelegramNotifierPlugin"
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../EasyHttpLib" "${CMAKE_CURRENT_SOURCE_DIR}/../EasyHttpLib/build")
target_link_libraries(TelegramNotifierPlugin PRIVATE EasyHttpLib)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../MetricsModel" "${CMAKE_CURRENT_SOURCE_DIR}/../MetricsModel/build")
target_link_libraries(TelegramNotifierPlugin PRIVATE MetricsModel)

find_package(PluginCore REQUIRED)
target_link_libraries(TelegramNotifierPlugin PRIVATE d3156::PluginCore)

find_package(Boost 1.8 REQUIRED COMPONENTS json thread system)

target_include_directories(TelegramNotifierPlugin PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(TelegramNotifierPlugin PRIVATE
  Boost::json
  Boost::system
  Boost::thread
)
