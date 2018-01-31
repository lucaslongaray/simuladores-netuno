rock_testsuite(normalDepthMap_core NormalDepthMap_test.cpp
    DEPS normal_depth_map
    LIBS ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
    DEPS_PKGCONFIG opencv)

rock_testsuite(imageViewerCaptureTool_core ImageViewerCaptureTool_test.cpp
    DEPS normal_depth_map
    LIBS ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
    DEPS_PKGCONFIG opencv)

rock_testsuite(NormalMapping_core NormalMapping_test.cpp
    DEPS normal_depth_map
    LIBS ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
    DEPS_PKGCONFIG opencv)

rock_testsuite(Material_core Material_test.cpp
    DEPS normal_depth_map
    LIBS ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
    DEPS_PKGCONFIG opencv)
