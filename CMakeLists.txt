cmake_minimum_required(VERSION 3.21)

project(
	Rumble
	VERSION 0.9.0
	LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(XSEPlugin)

target_include_directories(
	"${PROJECT_NAME}"
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/cmake
		${CMAKE_CURRENT_SOURCE_DIR}/src
		nlohmann_json::nlohmann_json
)

find_package(magic_enum CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(
${PROJECT_NAME} 
PUBLIC 
CommonLibSSE::CommonLibSSE
PRIVATE
	magic_enum::magic_enum)