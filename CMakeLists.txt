cmake_minimum_required(VERSION 3.0.0)
project(WeatherApp VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 20)

include(CTest)
enable_testing()

add_executable(WeatherApp main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


find_package(raylib CONFIG REQUIRED)
target_link_libraries(WeatherApp PRIVATE raylib)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(WeatherApp PRIVATE nlohmann_json::nlohmann_json)

find_package(cpr CONFIG REQUIRED)
target_link_libraries(WeatherApp PRIVATE cpr::cpr)

find_package(Boost COMPONENTS thread REQUIRED)
target_link_libraries(WeatherApp PRIVATE Boost::thread)




add_custom_target(
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIRP}

)