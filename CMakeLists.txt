cmake_minimum_required(VERSION 2.6)


project (ToolBox C CXX)

set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Wunused -pedantic")



set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)

FIND_PACKAGE(Boost 1.52 COMPONENTS regex system program_options REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
ADD_DEFINITIONS( "-DHAS_BOOST" )

FIND_PACKAGE(OpenMP)
 find_package(OpenMP)
 if (OPENMP_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
 endif (OPENMP_FOUND)

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


install(TARGETS ${colsum_exe} ${timesum_exe} DESTINATION .)



