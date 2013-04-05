cmake_minimum_required (VERSION 2.6)

project (SQLParser)
set (SQLParser_VERSION_MAJOR 0)
set (SQLParser_VERSION_MINOR 0)
set (LOG_LEVEL 9)

set(CMAKE_CXX_FLAGS "-std=c++11")

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

add_executable(lexer
	src/bin/lexer.C
	src/lib/Token.C
	${FLEX_scanner_OUTPUTS}
)

add_custom_command(
	OUTPUT ${PROJECT_BINARY_DIR}/Parser.y
	COMMAND cp ${PROJECT_SOURCE_DIR}/src/lib/Parser.y Parser.y
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	DEPENDS src/lib/Parser.y
)

add_custom_command(
	OUTPUT Parser.C
	COMMAND lemon Parser.y
	COMMAND cat Parser.c ${PROJECT_SOURCE_DIR}/src/bin/parser.C > Parser.C
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	DEPENDS ${PROJECT_BINARY_DIR}/Parser.y
)

add_executable(parser
	${PROJECT_BINARY_DIR}/Parser.C
)

	