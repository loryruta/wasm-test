cmake_minimum_required(VERSION 3.22)

project(voronoi-bg)

add_executable(kdtree-test
    src/kdtree_test.cpp
    src/kdtree.hpp
)

add_executable(app
    src/app.cpp
    src/kdtree.hpp
)
