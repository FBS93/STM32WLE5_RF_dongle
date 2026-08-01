if(EBF_CORE STREQUAL "EBF_CORE_BAREMETAL")
    # Currently no specific epf implementation for baremetal.
elseif(EBF_CORE STREQUAL "EBF_CORE_RTOS")
    # Currently no specific epf implementation for rtos.
elseif(EBF_CORE STREQUAL "EBF_CORE_OS")
    add_subdirectory(os)
else()
    message(FATAL_ERROR "Invalid EBF_CORE value: ${EBF_CORE}")
endif()
