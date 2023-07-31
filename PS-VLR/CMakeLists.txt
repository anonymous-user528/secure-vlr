

cmake_minimum_required (VERSION 3.5.1)


project(demo CXX)
set(CMAKE_CXX_STANDARD 20)
option( BUILD_STATIC "Set to ON to include static versions of the library" OFF)

find_package(Boost REQUIRED COMPONENTS) 
message(STATUS "Boost version: ${Boost_VERSION}")

message(STATUS "fmt version: ${fmt_VERSION} ${FMT_LIBRARY}")

find_library(ProgramOptions boost_program_options)

find_package(OpenFHE REQUIRED COMPONENTS) 
message(STATUS "OpenFHE version: ${OpenFHE_VERSION}")

find_package(HEXL)
find_package(Tcmalloc)
message(STATUS "OpenFHE version: ${Tcmalloc_VERSION}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive --std=c++2a -w -DOPENFHE_VERSION=0.9.3 -DMATHBACKEND=4 -fopenmp -fopenmp")

include_directories( 
    "src")


include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( ${OPENMP_INCLUDES} )
include_directories( ${OpenFHE_INCLUDE} )
include_directories( ${OpenFHE_INCLUDE}/third-party/include )
include_directories( ${OpenFHE_INCLUDE}/core )
include_directories( ${OpenFHE_INCLUDE}/pke )

file(GLOB DATATYPES src/datatypes/*.cpp)
file(GLOB SERIALIZATION src/serialization/*.cpp)
file(GLOB TOOLS src/tools/*vector.cpp)
file(GLOB NETWORK src/network/*.cpp)
file(GLOB CONFIG src/config/*.cpp)
file(GLOB MPC src/mpc/*.cpp)

add_library(LIB_PPPU OBJECT
    ${DATATYPES}
    ${SERIALIZATION}
    ${NETWORK}
    ${TOOLS}
    ${MPC}
    ${CONFIG})

## Relic
target_link_libraries(LIB_PPPU relic)
## GMP
target_link_libraries(LIB_PPPU gmp gmpxx)
## openssl
target_link_libraries(LIB_PPPU ssl crypto)
## other
target_link_libraries(LIB_PPPU fmt pthread)


link_directories( ${OpenFHE_LIBDIR} )
link_directories( ${OPENMP_LIBRARIES} )


if(BUILD_STATIC)
    set( CMAKE_EXE_LINKER_FLAGS "${OpenFHE_EXE_LINKER_FLAGS} -static")
    link_libraries( ${OpenFHE_STATIC_LIBRARIES} )
    
else()
    set( CMAKE_EXE_LINKER_FLAGS ${OpenFHE_EXE_LINKER_FLAGS} )
    link_libraries( ${OpenFHE_SHARED_LIBRARIES} )
endif()

set( SOURCE_FILES test.cpp
                 src/client/client.cpp
                 src/models/psvlr.cpp)
add_executable(test ${SOURCE_FILES})
target_link_libraries(test
    LIB_PPPU
    ${ProgramOptions})
