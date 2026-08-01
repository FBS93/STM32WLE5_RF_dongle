if(DEFINED ECF_TARGET_PLATFORM)
    # Add target platform specific source directory.
    add_subdirectory(platform/${ECF_TARGET_PLATFORM})
endif()

add_subdirectory(embedded_assert_framework)
add_subdirectory(embedded_base_framework)
add_subdirectory(embedded_fake_framework)
add_subdirectory(embedded_middleware_framework)
add_subdirectory(embedded_platform_framework)
add_subdirectory(embedded_test_framework)
add_subdirectory(event_driven_framework)