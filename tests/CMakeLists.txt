ENABLE_TESTING()

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

pkg_check_modules(EIGEN3 REQUIRED eigen3)
find_path(SVA_INCLUDE_DIRS SpaceVecAlg/SpaceVecAlg
                           /usr/include
                           /usr/local/include)

find_path(RBD_INCLUDE_DIRS RBDyn/
                           /usr/include
                           /usr/local/include)

include_directories(${EIGEN3_INCLUDE_DIRS})
include_directories(${SVA_INCLUDE_DIRS}/SpaceVecAlg)
include_directories(${RBD_INCLUDE_DIRS}/RBDyn)
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories(${Boost_INCLUDE_DIRS})

add_executable(QPSolverTest QPSolverTest.cpp)
target_link_libraries(QPSolverTest ${Boost_LIBRARIES} RBDyn Tasks)
add_test(QPSolverTestUnit QPSolverTest)
