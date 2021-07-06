if(NOT BUILD_MULTISESSION_SAMPLE)
  return()
endif(NOT BUILD_MULTISESSION_SAMPLE)

list(APPEND MULTISESSION_SRCS "src/multisession.cc")

add_executable(onert-multisession-app ${MULTISESSION_SRCS})
target_link_libraries(onert-multisession-app nnfw-dev pthread dl)

install(TARGETS onert-multisession-app DESTINATION bin)
