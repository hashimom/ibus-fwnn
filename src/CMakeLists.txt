include_directories(
    ${IBUS_INCLUDE_DIRS}
)

add_executable(ibus-engine-fwnn
  main.c
  engine.c
  convert.c
  fwnnserver.c
)

target_link_libraries(ibus-engine-fwnn
    ${IBUS_LIBRARIES}
    wnn
)

install(TARGETS ibus-engine-fwnn DESTINATION ${IBUS_FWNN_LIB_DIR})
