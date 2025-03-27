cmake_minimum_required(VERSION 3.16)

project(TomlObjectViewer
  VERSION 0.2.0
  LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

FetchContent_Declare(
  tomlplusplus
  GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
  GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

set(TS_FILES TomlObjectViewer_ru_RU.ts)

set(PROJECT_SOURCES
  main.cpp
  MainWindow.cpp
  MainWindow.h
  MainWindow.ui
  TreeModel.h
  TreeModel.cpp
  TreeItem.h
  TreeItem.cpp
  TreeItemDelegate.h
  TreeItemDelegate.cpp
  ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)  
  qt_add_executable(TomlObjectViewer
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
  )

  qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_include_directories(TomlObjectViewer PRIVATE
  ${tomlplusplus_SOURCE_DIR}/include/
)

target_link_libraries(TomlObjectViewer PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TomlObjectViewer)
endif()

set_target_properties(TomlObjectViewer PROPERTIES
  ${BUNDLE_ID_OPTION}
  MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
  MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
  MACOSX_BUNDLE TRUE
  WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)

install(TARGETS TomlObjectViewer
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
  qt_finalize_executable(TomlObjectViewer)
endif()
