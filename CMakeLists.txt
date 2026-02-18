cmake_minimum_required(VERSION 3.16)
project(TelegramNotifierPlugin VERSION 1.1.1 LANGUAGES CXX)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../tools/workspace.cmake")
create_target(plugin)

dependency(".." EasyHttpLib "https://gitlab.bubki.zip/d3156/EasyHttpLib.git")
dependency(".." MetricsModel "https://gitlab.bubki.zip/d3156/MetricsModel.git")
dependency(".." ConfiguratorModel "https://gitlab.bubki.zip/d3156/PluginConfigurator.git")

find_package(Boost 1.8 REQUIRED COMPONENTS json thread system)
target_link_libraries(TelegramNotifierPlugin PRIVATE Boost::json Boost::system Boost::thread)

find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)
target_link_libraries(TelegramNotifierPlugin PUBLIC OpenSSL::SSL OpenSSL::Crypto)