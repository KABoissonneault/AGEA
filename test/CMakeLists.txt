enable_testing()
set(AGEA_TEST_SRC 
	src/main.cpp
	src/math/vector.cpp
)
add_executable(AGEA_TEST ${AGEA_TEST_SRC})

source_group(src\\math REGULAR_EXPRESSION src/math/*)
source_group(src REGULAR_EXPRESSION src/*)

include_directories("ext/include")