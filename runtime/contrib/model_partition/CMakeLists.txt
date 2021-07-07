if(NOT BUILD_MODEL_PARTITION_TOOL)
  return()
endif(NOT BUILD_MODEL_PARTITION_TOOL)


list(APPEND SOURCES "GraphSplitting.cc")
list(APPEND SOURCES "partition_writer.cc")

add_executable(model_partition ${SOURCES})

target_link_libraries(model_partition jsoncpp)