#
# Main Makefile. This is basically the same as a component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

#COMPONENT_SRCDIRS := . src
#COMPONENT_ADD_INCLUDEDIRS := . src

COMPONENT_SRCDIRS := src src/utility src/sensors src/sensors/ir src/web src/web/libb64 src/web/detail src/web/libsha1 src/Fonts src/Fonts/GFXFF src/Fonts/Custom src/Fonts/TrueType
COMPONENT_ADD_INCLUDEDIRS := src src/utility src/sensors src/sensors/ir src/web src/web/libb64 src/web/detail src/web/libsha1 src/Fonts src/Fonts/GFXFF src/Fonts/Custom src/Fonts/TrueType

