add_executable (edje2yuv edje2yuv.c)
target_link_libraries (edje2yuv ${CLI_LIBS})
install (TARGETS edje2yuv DESTINATION bin)
