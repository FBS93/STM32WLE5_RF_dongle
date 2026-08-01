if(EBF_CORE STREQUAL "EBF_CORE_BAREMETAL")
    add_subdirectory(baremetal)
elseif(EBF_CORE STREQUAL "EBF_CORE_RTOS")
    # Currently no specific ebf implementation for rtos.
elseif(EBF_CORE STREQUAL "EBF_CORE_OS")
    add_subdirectory(os)
else()
    message(FATAL_ERROR "Invalid EBF_CORE value: ${EBF_CORE}")
endif()