add_library(xenia-hid-kinect STATIC)
xe_platform_sources(xenia-hid-kinect ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(xenia-hid-kinect PUBLIC xenia-base xenia-hid)
xe_target_defaults(xenia-hid-kinect)