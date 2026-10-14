cmake_minimum_required(VERSION 3.16)

project(TaskManagerPlus VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appTaskManagerPlus
    main.cpp
    HardwareInfo.h
    HardwareInfo.cpp
)

qt_add_qml_module(appTaskManagerPlus
    URI TaskManagerPlus
    QML_FILES
        Main.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appTaskManagerPlus PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appTaskManagerPlus
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appTaskManagerPlus
    PRIVATE Qt6::Quick
)

if(WIN32)
    target_link_libraries(appTaskManagerPlus PRIVATE wbemuuid ole32 oleaut32 user32)
endif()

option(BUILD_TESTING "Build focused hardware-info tests" ON)
if(BUILD_TESTING)
    enable_testing()
    qt_add_executable(hardwareInfoTests
        HardwareInfoTests.cpp
        HardwareInfo.h
        HardwareInfo.cpp
    )
    target_link_libraries(hardwareInfoTests PRIVATE Qt6::Core)
    if(WIN32)
        target_link_libraries(hardwareInfoTests PRIVATE wbemuuid ole32 oleaut32 user32)
    endif()
    add_test(NAME hardwareInfoTests COMMAND hardwareInfoTests)
endif()

install(TARGETS appTaskManagerPlus
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
