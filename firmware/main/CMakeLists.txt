idf_component_register(SRCS "main.cpp" "voice_ui.cpp" INCLUDE_DIRS "." "tusb"
    REQUIRES m5stack__m5unified espressif__usb_device_uac espressif__tinyusb)
idf_component_get_property(UAC_PATH espressif__usb_device_uac COMPONENT_DIR)
idf_component_get_property(tusb_lib espressif__tinyusb COMPONENT_LIB)
target_include_directories(${tusb_lib} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/tusb" "${UAC_PATH}/tusb_uac")
target_sources(${tusb_lib} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/tusb/usb_descriptors.c")
cmake_policy(SET CMP0079 NEW)
target_link_libraries(${tusb_lib} PRIVATE ${COMPONENT_LIB})
