
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(ZeekPluginAMQPWriter)

include(ZeekPlugin)
find_package(LIB_RABBITMQ_C)

if (LIB_RABBITMQ_C_FOUND)
	include_directories(BEFORE ${LIB_RABBITMQ_C_ROOT_DIR})
	zeek_plugin_begin(Trapmine AMQPWriter)
	zeek_plugin_cc(src/TaggedJSON.cc)
	zeek_plugin_cc(src/AMQP.cc)
	zeek_plugin_cc(src/Plugin.cc)
	zeek_plugin_bif(src/amqpwriter.bif)
	zeek_plugin_dist_files(README CHANGES COPYING VERSION)
	zeek_plugin_link_library(${LIB_RABBITMQ_C_LIBRARIES})
	zeek_plugin_end()

elseif (NOT LIB_RABBITMQ_C_FOUND)
	message(FATAL_ERROR "rabbitmq-c not found.")

endif ()


file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION LIMIT_COUNT 1)

if ("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    # Allows building rpm/deb packages via "make package" in build dir.
    include(ConfigurePackaging)
    ConfigurePackaging(${VERSION})
endif ()
