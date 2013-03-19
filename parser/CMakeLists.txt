cmake_minimum_required (VERSION 2.6)

project (SQLParser)
set (SQLParser_VERSION_MAJOR 0)
set (SQLParser_VERSION_MINOR 0)
set (LOG_LEVEL 9)

find_package(FLEX)
FLEX_TARGET(scanner
	${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Scanner.l
	${CMAKE_CURRENT_BINARY_DIR}/Scanner.C
)

add_custom_target(flex_h ALL
	COMMAND flex --header-file=${CMAKE_CURRENT_BINARY_DIR}/flex.h ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Scanner.l
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Scanner.l
)

include_directories(
	${PROJECT_SOURCE_DIR}/src/lib
	${PROJECT_SOURCE_DIR}/src/bin
	${PROJECT_BINARY_DIR}
)

add_executable(parser
	src/bin/parser.C
	src/lib/Token.C
	${FLEX_scanner_OUTPUTS}
)


