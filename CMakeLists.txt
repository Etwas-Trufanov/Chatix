cmake_minimum_required(VERSION 3.16)
project(Chatix VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ищем только Qt6 и CURL
find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets LinguistTools Quick QuickWidgets)

set(TS_FILES Chatix_ru_RU.ts)

set(PROJECT_SOURCES
    main.cpp
    chatixmainwindow.cpp
    chatixmainwindow.h
    chatixmainwindow.ui
    ${TS_FILES}
)

# Создаём исполняемый файл (Qt6)
qt_add_executable(Chatix
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    llmconnector.hpp
    ollama.hpp
    lmmanager.hpp
    errordialog.h errordialog.cpp errordialog.ui


)

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_link_libraries(Chatix PRIVATE
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickWidgets
)

if(WIN32)
    target_link_libraries(Chatix PRIVATE ws2_32 wsock32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(Chatix PRIVATE pthread)
endif()

set_target_properties(Chatix PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Chatix
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(Chatix)