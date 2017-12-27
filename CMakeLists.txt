project (AGEA)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
cmake_minimum_required (VERSION 3.1)

set (CMAKE_CXX_STANDARD 17)
set (AGEA_VERSION_MAJOR 0)
set (AGEA_VERSION_MINOR 1)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(AGEA_SRC src/main.cpp)
set(AGEA_INCLUDE
	include/common/range/view.h
	include/functional/functional.h
	include/input/event.h
	include/math/integration.h
	include/math/vector.h
	include/meta/detected.h
	include/model/entity.h
	include/physics/body.h
	include/physics/time.h
	include/view/sdl/sdl.h
)	

add_executable(AGEA ${AGEA_SRC} ${AGEA_INCLUDE})

source_group(include\\common\\range REGULAR_EXPRESSION include/common/range/*)
source_group(include\\functional REGULAR_EXPRESSION include/functional/*)
source_group(include\\input REGULAR_EXPRESSION include/input/*)
source_group(include\\math REGULAR_EXPRESSION include/math/*)
source_group(include\\meta REGULAR_EXPRESSION include/meta/*)
source_group(include\\model REGULAR_EXPRESSION include/model/*)
source_group(include\\physics REGULAR_EXPRESSION include/physics/*)
source_group(include\\view\\sdl REGULAR_EXPRESSION include/view/sdl/*)

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/ext/include")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
target_link_libraries(AGEA ${SDL2_LIBRARY})
list(GET SDL2_LIBRARY 0 SDL2_FIRST_LIB)
get_filename_component(SDL2_LIBRARY_PATH ${SDL2_FIRST_LIB} DIRECTORY)

add_custom_command(TARGET AGEA PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${SDL2_LIBRARY_PATH}/sdl2.dll" "${SDL2_LIBRARY_PATH}/sdl2d.dll" $<TARGET_FILE_DIR:AGEA>)