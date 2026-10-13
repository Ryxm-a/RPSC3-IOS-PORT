# RPCS3 iOS app shell (Qt-free). Built when CMAKE_SYSTEM_NAME=iOS.
enable_language(OBJCXX)
set(CMAKE_OBJCXX_STANDARD 23)

# ---------------------------------------------------------------------------------------------
# Sources the emulator core needs but upstream only compiles inside the Qt frontend
# (rpcs3/rpcs3qt/CMakeLists.txt -> rpcs3_ui), which is skipped when RPCS3_NO_QT is set.
# Without these, rpcs3_ios fails to link (pad_thread, pad handlers, version info, PS Move
# config, Wiimote/GunCon3 support, ...). Only Qt-free files are listed here:
#   not built on iOS: keyboard/mouse handlers, gui_pad_thread, mouse_gyro_handler.cpp (Qt),
#                     evdev/xinput/mm_joystick/raw_mouse (other OSes), sdl_* (USE_SDL=OFF)
#   ps_move_tracker.cpp compiles without OpenCV (its OpenCV code is behind HAVE_OPENCV).
# ---------------------------------------------------------------------------------------------
set(RPCS3_SRC_DIR ${CMAKE_SOURCE_DIR}/rpcs3)
target_sources(rpcs3_emu PRIVATE
    ${RPCS3_SRC_DIR}/rpcs3_version.cpp
    ${RPCS3_SRC_DIR}/Input/pad_thread.cpp
    ${RPCS3_SRC_DIR}/Input/product_info.cpp
    ${RPCS3_SRC_DIR}/Input/hid_instance.cpp
    ${RPCS3_SRC_DIR}/Input/hid_pad_handler.cpp
    ${RPCS3_SRC_DIR}/Input/ds3_pad_handler.cpp
    ${RPCS3_SRC_DIR}/Input/ds4_pad_handler.cpp
    ${RPCS3_SRC_DIR}/Input/dualsense_pad_handler.cpp
    ${RPCS3_SRC_DIR}/Input/skateboard_pad_handler.cpp
    ${RPCS3_SRC_DIR}/Input/ps_move_handler.cpp
    ${RPCS3_SRC_DIR}/Input/ps_move_config.cpp
    ${RPCS3_SRC_DIR}/Input/ps_move_calibration.cpp
    ${RPCS3_SRC_DIR}/Input/ps_move_tracker.cpp
    ${RPCS3_SRC_DIR}/Input/wiimote_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ios_input_support.cpp   # Qt-free mouse_gyro_handler + g_input_config_override
)

add_executable(rpcs3_ios MACOSX_BUNDLE
    main.mm
    ios_app.mm
    ios_gs_frame.mm
    ios_glue.cpp
)

target_link_libraries(rpcs3_ios PRIVATE rpcs3_emu
    "-framework UIKit" "-framework Foundation" "-framework QuartzCore"
    "-framework Metal" "-framework AudioToolbox" "-framework AVFoundation"
    "-framework CoreAudio" "-framework CoreMotion" "-framework GameController"
    "-framework UniformTypeIdentifiers" "-framework CoreGraphics" iconv)

target_compile_options(rpcs3_ios PRIVATE -fobjc-arc)

set_target_properties(rpcs3_ios PROPERTIES
    OUTPUT_NAME "RPCS3"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    MACOSX_BUNDLE_GUI_IDENTIFIER "net.rpcs3.ios"
    MACOSX_BUNDLE_BUNDLE_NAME "RPCS3"
    MACOSX_BUNDLE_BUNDLE_VERSION "1"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.0.1"
    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "16.0"
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "net.rpcs3.ios"
    # Unsigned build: sign later with your sideloading tool (AltStore/SideStore/etc.)
    XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
    XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/rpcs3.entitlements"
    XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
)

# Bundle GUI configs / icons the emulator expects next to the executable
add_custom_command(TARGET rpcs3_ios POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/bin/Icons $<TARGET_BUNDLE_DIR:rpcs3_ios>/Icons
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/bin/GuiConfigs $<TARGET_BUNDLE_DIR:rpcs3_ios>/GuiConfigs
)
