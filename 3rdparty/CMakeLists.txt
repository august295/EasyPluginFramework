set(THIRDPARTY_FOLDER "3rdparty")

####################################################################################################
# endecode
####################################################################################################
set(BUILD_SINGLE_LIBRARY ON)
add_subdirectory(endecode)
set_property(TARGET "endecode" PROPERTY FOLDER "${THIRDPARTY_FOLDER}/endecode")
