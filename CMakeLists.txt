cmake_minimum_required(VERSION 2.6)


project (ToolBox C CXX)

set(CMAKE_CXX_FLAGS "-O3 -std=c++0x -Wall -Wextra -Wunused -pedantic")



set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)

FIND_PACKAGE(Boost 1.46 COMPONENTS regex system program_options REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
ADD_DEFINITIONS( "-DHAS_BOOST" )


IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  FIND_PACKAGE(Git)
  IF(GIT_FOUND)
    EXECUTE_PROCESS(
      COMMAND ${GIT_EXECUTABLE} describe --tags --always
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE "MY_BUILD_VERSION"
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    MESSAGE( STATUS "Git version: ${MY_BUILD_VERSION}" )
  ELSE(GIT_FOUND)
    SET(MY_BUILD_VERSION 0)
  ENDIF(GIT_FOUND)
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/Version.hpp @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


################################################
#                    colsum                    #
################################################

SET(colsum_src ./src/colsum.cpp)
SET(colsum_exe colsum)
ADD_EXECUTABLE(${colsum_exe} ${colsum_src})
target_link_libraries(${colsum_exe} 
	${Boost_LIBRARIES}
)


################################################
#                   rowmerge                   #
################################################

SET(rowmerge_src ./src/rowmerge.cpp)
SET(rowmerge_exe rowmerge)
ADD_EXECUTABLE(${rowmerge_exe} ${rowmerge_src})
target_link_libraries(${rowmerge_exe}
	${Boost_LIBRARIES}
)

################################################
#                   rowFilter                   #
################################################
SET(rowFilter_src ./src/rowFilter.cpp)
SET(rowFilter_exe rowFilter)
ADD_EXECUTABLE(${rowFilter_exe} ${rowFilter_src})
target_link_libraries(${rowFilter_exe}
	${Boost_LIBRARIES}
)

################################################
#                   csv2tex                    #
################################################
SET(csv2tex_src ./src/csv2tex.cpp)
SET(csv2tex_exe csv2tex)
ADD_EXECUTABLE(${csv2tex_exe} ${csv2tex_src})
target_link_libraries(${csv2tex_exe}
	${Boost_LIBRARIES}
)

install(TARGETS ${colsum_exe} ${timesum_exe} DESTINATION .)



