set(HAVIT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../src")

idf_component_register(
    SRCS "${HAVIT_SOURCE_DIR}/havit_app.c"
    INCLUDE_DIRS "${HAVIT_SOURCE_DIR}")
