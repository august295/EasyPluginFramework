set(THIRDPARTY_FOLDER "3rdparty")

####################################################################################################
# endecode
####################################################################################################
set(BUILD_SINGLE_LIBRARY ON CACHE BOOL "" FORCE)
set(BUILD_VCPKG ON CACHE BOOL "" FORCE)
add_subdirectory(endecode)
set_property(TARGET "endecode" PROPERTY FOLDER "${THIRDPARTY_FOLDER}")

####################################################################################################
# cpptrace
####################################################################################################
add_subdirectory(cpptrace)
set_property(TARGET "cpptrace-lib" PROPERTY FOLDER "${THIRDPARTY_FOLDER}")
