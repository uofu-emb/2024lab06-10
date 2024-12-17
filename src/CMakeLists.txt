# Main binary
add_executable(main
    main.c
    ../test/helper.c   # Include the helper implementation
)

# Add include directory for helper.h
target_include_directories(main PRIVATE
    ../test   # Add test directory to include paths
)

pico_set_program_name(main "test")
pico_set_program_version(main "0.1")

# Enable output over USB
pico_enable_stdio_uart(main 0)
pico_enable_stdio_usb(main 1)

# Link required libraries
target_link_libraries(main PRIVATE
  pico_stdlib
  pico_multicore
  pico_async_context_freertos
  FreeRTOS-Kernel-Heap4
)

# Add CYW43 libraries if supported
if(PICO_CYW43_SUPPORTED)
    target_link_libraries(main PRIVATE
        pico_cyw43_arch_none
    )
endif()

pico_add_extra_outputs(main)

# Flash target
add_custom_target(flash picotool load -f $<TARGET_FILE:main> DEPENDS main)
