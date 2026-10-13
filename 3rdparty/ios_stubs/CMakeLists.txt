# iOS has no public IOKit HID / IOKit USB, so hidapi and libusb are replaced with
# stub implementations that report "no devices". Real headers are still used so
# the call sites in RPCS3 compile unchanged.
set(HIDAPI_INC  ${CMAKE_CURRENT_SOURCE_DIR}/../hidapi/hidapi/hidapi ${CMAKE_CURRENT_SOURCE_DIR}/../hidapi/hidapi/mac)
set(LIBUSB_INC  ${CMAKE_CURRENT_SOURCE_DIR}/../libusb/libusb/libusb)

add_library(ios_hidapi_stub STATIC hidapi_stub.c)
target_include_directories(ios_hidapi_stub PUBLIC ${HIDAPI_INC})
target_compile_options(ios_hidapi_stub PRIVATE -w)
target_link_libraries(3rdparty_hidapi INTERFACE ios_hidapi_stub)
target_include_directories(3rdparty_hidapi SYSTEM INTERFACE ${HIDAPI_INC})

add_library(usb-1.0-static STATIC libusb_stub.c)
target_include_directories(usb-1.0-static SYSTEM PUBLIC ${LIBUSB_INC})
target_compile_options(usb-1.0-static PRIVATE -w)
