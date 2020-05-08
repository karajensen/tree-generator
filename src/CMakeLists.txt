cmake_minimum_required(VERSION 3.17)
project(TreeGenerator)

add_definitions(-DNT_PLUGIN)
add_definitions(-DTESTING_EXPORTS)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRC_LIST
    ../readme.txt
    TreeGeneratorGUI.mel
    vector3.h
    common.h
    matrix.h
    treeGeneratorGUI.h
    treeGeneratorGUI.cpp
    treeComponents.h
    treeHelpers.h
    treeGenerator.h
    treeGenerator.cpp
    randomGenerator.h
    randomGenerator.cpp
    pluginEntry.cpp
)

set(MAYA_SDK_DIR $ENV{MAYA_SDK_DIR})

include_directories(${MAYA_SDK_DIR}/include)

add_library(TreeGenerator SHARED ${SRC_LIST})
set_target_properties(TreeGenerator PROPERTIES SUFFIX ".mll")

target_link_libraries(TreeGenerator ${MAYA_SDK_DIR}/lib/Foundation.lib)
target_link_libraries(TreeGenerator ${MAYA_SDK_DIR}/lib/OpenMaya.lib)
target_link_libraries(TreeGenerator ${MAYA_SDK_DIR}/lib/OpenMayaUI.lib)
target_link_libraries(TreeGenerator ${MAYA_SDK_DIR}/lib/OpenMayaAnim.lib)
target_link_libraries(TreeGenerator ${MAYA_SDK_DIR}/lib/OpenMayaFX.lib)
target_link_libraries(TreeGenerator ${MAYA_SDK_DIR}/lib/OpenMayaRender.lib)
target_link_libraries(TreeGenerator ${MAYA_SDK_DIR}/lib/Image.lib)