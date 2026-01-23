cmake_minimum_required(VERSION 3.16)
project(TelegramNotifierPlugin
  VERSION 1.0.1
  LANGUAGES CXX
)
add_compile_definitions(TelegramNotifierPlugin_VERSION="${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(TelegramNotifierPlugin SHARED
  src/TelegramNotifierPlugin.cpp
)

target_include_directories(TelegramNotifierPlugin
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Put resulting .so into workspace/Plugins
set_target_properties(TelegramNotifierPlugin PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Plugins"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Plugins"
  OUTPUT_NAME "TelegramNotifierPlugin"
)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../EasyHttpLib/CMakeLists.txt")
  message(STATUS "EasyHttpLib not found, downloading...")

  include(FetchContent)
  FetchContent_Declare(
    EasyHttpLib
    GIT_REPOSITORY https://github.com/d3156/EasyHttpLib.git
    GIT_TAG master # или конкретный тег/коммит
  )
  FetchContent_MakeAvailable(EasyHttpLib)
else()
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../EasyHttpLib"
    "${CMAKE_BINARY_DIR}/_deps/EasyHttpLib")
endif()

target_link_libraries(TelegramNotifierPlugin PRIVATE EasyHttpLib)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../MetricsModel/CMakeLists.txt")
  message(STATUS "MetricsModel not found, downloading...")

  include(FetchContent)
  FetchContent_Declare(
    MetricsModel
    GIT_REPOSITORY https://github.com/d3156/MetricsModel.git
    GIT_TAG master # или конкретный тег/коммит
  )
  FetchContent_MakeAvailable(MetricsModel)
else()
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../MetricsModel"
    "${CMAKE_BINARY_DIR}/_deps/MetricsModel")
endif()

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
