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
    SystemMetricsProvider.h
    SystemMetricsProvider.cpp
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
    target_link_libraries(appTaskManagerPlus PRIVATE wbemuuid ole32 oleaut32 user32 pdh dxgi)
endif()

option(TASKMANAGER_BUILD_SENSOR_BRIDGE "Build the optional LibreHardwareMonitor temperature helper (.NET 8)" ON)
if(WIN32 AND TASKMANAGER_BUILD_SENSOR_BRIDGE)
    find_program(DOTNET_EXECUTABLE dotnet)
    if(DOTNET_EXECUTABLE)
        execute_process(COMMAND "${DOTNET_EXECUTABLE}" --list-sdks OUTPUT_VARIABLE DOTNET_SDKS)
    endif()
    if(DOTNET_EXECUTABLE AND DOTNET_SDKS)
        set(SENSOR_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/sensors")
        file(MAKE_DIRECTORY "${SENSOR_OUTPUT}/licenses")
        foreach(LICENSE_FILE LICENSE THIRD-PARTY-NOTICES.txt dotnet-LICENSE.txt mono-LICENSE.txt)
            set(LICENSE_URL "https://raw.githubusercontent.com/LibreHardwareMonitor/LibreHardwareMonitor/v0.9.6/${LICENSE_FILE}")
            if(LICENSE_FILE STREQUAL "dotnet-LICENSE.txt")
                set(LICENSE_URL "https://raw.githubusercontent.com/dotnet/runtime/v8.0.0/LICENSE.TXT")
            elseif(LICENSE_FILE STREQUAL "mono-LICENSE.txt")
                set(LICENSE_URL "https://raw.githubusercontent.com/mono/mono/mono-5.4.1.6/LICENSE")
            endif()
            if(NOT EXISTS "${SENSOR_OUTPUT}/licenses/${LICENSE_FILE}")
                file(DOWNLOAD
                    "${LICENSE_URL}"
                    "${SENSOR_OUTPUT}/licenses/${LICENSE_FILE}"
                    STATUS LICENSE_DOWNLOAD_STATUS TLS_VERIFY ON TIMEOUT 15 INACTIVITY_TIMEOUT 5)
                list(GET LICENSE_DOWNLOAD_STATUS 0 LICENSE_DOWNLOAD_CODE)
                if(NOT LICENSE_DOWNLOAD_CODE EQUAL 0)
                    file(REMOVE "${SENSOR_OUTPUT}/licenses/${LICENSE_FILE}")
                    message(FATAL_ERROR "Cannot obtain sensor dependency license: ${LICENSE_DOWNLOAD_STATUS}")
                endif()
            endif()
        endforeach()
        configure_file(SensorBridge/THIRD_PARTY_NOTICES.md "${SENSOR_OUTPUT}/licenses/THIRD_PARTY_NOTICES.md" COPYONLY)
        add_custom_target(sensorBridge ALL
            COMMAND "${CMAKE_COMMAND}" -E env DOTNET_CLI_TELEMETRY_OPTOUT=1
                "${DOTNET_EXECUTABLE}" publish
                "${CMAKE_CURRENT_SOURCE_DIR}/SensorBridge/TaskManager.SensorBridge.csproj"
                --configuration Release --self-contained false --output "${SENSOR_OUTPUT}"
                --artifacts-path "${CMAKE_CURRENT_BINARY_DIR}/sensor-artifacts"
            COMMENT "Building isolated LibreHardwareMonitor temperature helper"
            VERBATIM)
        add_dependencies(appTaskManagerPlus sensorBridge)
        install(DIRECTORY "${SENSOR_OUTPUT}/" DESTINATION "${CMAKE_INSTALL_BINDIR}/sensors")
    else()
        message(WARNING "No .NET SDK found: temperatures will be unavailable; native Overview metrics still build")
    endif()
endif()

option(BUILD_TESTING "Build focused hardware and metrics tests" ON)
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
    qt_add_executable(systemMetricsTests
        SystemMetricsTests.cpp
        SystemMetricsProvider.h
        SystemMetricsProvider.cpp
    )
    target_link_libraries(systemMetricsTests PRIVATE Qt6::Core)
    if(WIN32)
        target_link_libraries(systemMetricsTests PRIVATE pdh dxgi user32)
    endif()
    add_test(NAME systemMetricsTests COMMAND systemMetricsTests)
    if(TARGET sensorBridge)
        add_test(NAME temperatureSelectionTests COMMAND "${SENSOR_OUTPUT}/TaskManager.SensorBridge.exe" --self-test)
    endif()
endif()

install(TARGETS appTaskManagerPlus
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
