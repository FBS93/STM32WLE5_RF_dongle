add_library(events INTERFACE)
target_include_directories(events INTERFACE inc)
target_link_libraries(events INTERFACE edf rf_types)
