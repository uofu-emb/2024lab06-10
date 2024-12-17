# Test binary
add_executable(mytest
    mytest.c            # Add the main test file
    unity_config.c      # Include UNITY configuration
    ../test/helper.c    # Include the helper implementation
)

# Include directories
target_include_directories(mytest PRIVATE
    ../test             # Include path for helper.h and other headers
    ${UNITY_PATH}       # Ensure UNITY headers are included
)

# Link required libraries
target_link_libraries(mytest PRIVATE
    pico_stdlib         # Standard library for Pico
    pico_async_context_freertos
    FreeRTOS-Kernel-Heap4
    unity               # Link UNITY library for testing
)

# The CYW43 is the Wifi/Bluetooth module. If the board is set to pico_w, this
# variable will be set and the wireless libraries added.
if(PICO_CYW43_SUPPORTED)
    target_link_libraries(mytest PRIVATE
        pico_cyw43_arch_none
    )
endif()
#
# Modify the below lines to enable/disable output over UART/USB.
# These are mutually exclusive, only set one.
pico_enable_stdio_uart(mytest 0)
pico_enable_stdio_usb(mytest 1)

# This will speed up the simulation, since data won't be copied from the flash controller.
# It does make the binary non-persistent on a real microcontroller.
# pico_set_binary_type(mytest no_flash)

find_program(RENODE renode
# This is extra stuff so it can find the mac installation paths.
 NAMES macos_run.command Renode.exe
 PATHS /Applications/Renode.app/Contents/MacOS)

set(RENODE_FLAGS
  --disable-xwt
  --port -2
  --pid-file renode.pid
  --console
  )

add_test(NAME simulate_mytest COMMAND
    ${RENODE}
     ${RENODE_FLAGS}
    -e "$ELF=@$<TARGET_FILE:mytest>; $WORKING=@${CMAKE_SOURCE_DIR}; include @${CMAKE_SOURCE_DIR}/test/simulate.resc; start"
    )

# Flash target for test
add_custom_target(flash_test picotool load -f $<TARGET_FILE:mytest> DEPENDS mytest)
