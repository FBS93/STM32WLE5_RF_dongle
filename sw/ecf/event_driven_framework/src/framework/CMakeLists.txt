target_sources(edf PRIVATE
    src/edf_active_object.c
    src/edf_event_queue.c
    src/edf_event.c
    src/edf_framework.c
    src/edf_hsm.c
    src/edf_pool.c
    src/edf_time_event.c)
target_include_directories(edf
    PRIVATE inc)
target_link_libraries(edf
    PUBLIC ebf # ebf .h files are included in edf .h files, so link needs to be public.
    PRIVATE eaf
    PRIVATE emf)
