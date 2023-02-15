cmake_minimum_required(VERSION 3.21)

project(
	Rumble
	VERSION 1.0.0
	LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(XSEPlugin)

find_package(magic_enum CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(
${PROJECT_NAME} 
PUBLIC 
	CommonLibSSE::CommonLibSSE
PRIVATE
	magic_enum::magic_enum
	nlohmann_json::nlohmann_json
)