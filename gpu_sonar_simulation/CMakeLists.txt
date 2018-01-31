cmake_minimum_required(VERSION 2.8.3)
project(gpu_sonar_simulation)
set(CMAKE_AUTOMOC ON)
set(CMAKE_BUILD_TYPE Debug)
find_package(catkin REQUIRED COMPONENTS
    cv_bridge
    qt_build 
    geometry_msgs
    message_generation
    message_runtime
    std_msgs
    roscpp
    image_transport
    tf
)
find_package(gazebo REQUIRED)
find_package(OpenCV REQUIRED)
find_package(OpenSceneGraph REQUIRED COMPONENTS
    osgDB
    osgFX
    osgGA
    osgTerrain
    osgViewer
    osgText
    osgWidget
    osgManipulator
    osg
    osgShadow
    osgQt
)
find_package(Eigen3 REQUIRED)

#find_package(Qt4 REQUIRED COMPONENTS
#    QtCore
#    QtGui)

rosbuild_prepare_qt4(QtCore QtGui)
add_definitions(-DQT_NO_KEYWORDS)
#add_definitions(${QT_DEFINITIONS})

catkin_package(
  INCLUDE_DIRS include
)

include_directories(include
    ${catkin_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    ${QT_INCLUDE_DIR}
    ${QT_QFRAME_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=gnu++0x)
endif()

add_executable(gpu_sonar_simulation
    src/main.cpp
    src/Sonar.cpp
    src/Utils.cpp
    include/widget_collection/sonar_widget/SonarPlot.cc
    include/widget_collection/sonar_widget/SonarWidget.cc
    include/frame_helper/src/ColorGradient.cpp
    include/vizkit3d_normal_depth_map/src/NormalDepthMap.cpp
    include/vizkit3d_normal_depth_map/src/ImageViewerCaptureTool.cpp
)

target_link_libraries(gpu_sonar_simulation
    ${OpenCV_LIBS}
    ${catkin_LIBRARIES}
    ${OPENSCENEGRAPH_LIBRARIES}
    ${QT_LIBRARIES}
    ${QT_QFRAME_LIBRARY}
    ${Qwt_LIBRARIES}
)

add_executable(test_sonar
    test/test_Sonar.cpp
    src/Utils.cpp
    include/vizkit3d_normal_depth_map/src/NormalDepthMap.cpp
    include/vizkit3d_normal_depth_map/src/ImageViewerCaptureTool.cpp
)

target_link_libraries(test_sonar
    ${OpenCV_LIBS}
    ${catkin_LIBRARIES}
    ${OPENSCENEGRAPH_LIBRARIES}
)

#################################
set(MOC_HEADERS
    include/widget_collection/sonar_widget/SonarPlot.h
    include/widget_collection/sonar_widget/SonarWidget.cc
    include/base-types/base/samples/Sonar.hpp
    include/base-types/base/samples/SonarBeam.hpp
    include/base-types/base/samples/SonarScan.hpp
    include/frame_helper/src/ColorGradient.h
    include/frame_helper/src/ColorGradientTypes.h
    include/base-types/base/Float.hpp
    include/base-types/base/Time.hpp
    include/base-types/base/Angle.hpp
    include/base-types/base/Deprecated.hpp
    include/base-types/base/Eigen.hpp
)

QT4_WRAP_CPP(FOO_MOC_OUTFILES ${MOC_HEADERS})

set(SOURCE_FILES
    include/widget_collection/sonar_widget/SonarPlot.cc
    include/widget_collection/sonar_widget/SonarWidget.cc
    include/frame_helper/src/ColorGradient.cpp
)

add_library(SonarPlot SHARED ${SOURCE_FILES} ${MOC_OUTFILES})
