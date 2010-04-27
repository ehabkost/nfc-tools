project(desknfc)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

# Qt4
find_package(Qt4 REQUIRED)
set(QT_USE_QTDBUS true)
include(${QT_USE_FILE})

# KDE4
find_package( KDE4 REQUIRED )
include_directories( ${KDE4_INCLUDES} )

# Icons
add_subdirectory( icons )

# Sources files
set(desknfc_SOURCES main.cpp NfcUiManager.cpp NotifyAction.cpp Notification.cpp Content.cpp
			NfcTarget.cpp NfcDevice.cpp NfcDeviceManager.cpp)

# MOCable headers
set(desknfc_MOCABLE_HEADERS NfcUiManager.h NotifyAction.h Notification.h Content.h
			NfcTarget.h NfcDevice.h NfcDeviceManager.h)

# UI Classes
set(desknfc_UI ui/MainWindow.ui ui/ContentWidget.ui ui/TargetWidget.ui)
qt4_wrap_ui(desknfc_SOURCES ${desknfc_UI})

# Additonnal ressources
#qt4_add_resources(desknfc_SOURCES systray.qrc)

# Translation
set(desknfc_TS i18n/fr_FR.ts)
set(rebuild_translations off cache bool "update source translation i18n/*.ts (WARNING: make clean will delete the source .ts files! Danger!)")
if (REBUILD_TRANSLATIONS)
  qt4_create_translation(desknfc_QM ${desknfc_SOURCES} ${desknfc_UI} ${desknfc_TS})
else (REBUILD_TRANSLATIONS)
  qt4_add_translation(desknfc_QM ${desknfc_TS})
endif (REBUILD_TRANSLATIONS) 

add_custom_target (translations_target DEPENDS ${desknfc_QM})


# Generate D-Bus interface
  # NfcDeviceManager
  set_source_files_properties(NfcDeviceManager.xml PROPERTIES CLASSNAME NfcDeviceManagerInterface)
  qt4_add_dbus_interface(desknfc_SOURCES NfcDeviceManager.xml NfcDeviceManagerInterface)

  # NfcDevice
  set_source_files_properties(NfcDevice.xml PROPERTIES CLASSNAME NfcDeviceInterface)
  qt4_add_dbus_interface(desknfc_SOURCES NfcDevice.xml NfcDeviceInterface)

  # Notify
  set_source_files_properties(Notify.xml PROPERTIES CLASSNAME NotifyInterface)
  qt4_add_dbus_interface(desknfc_SOURCES Notify.xml NotifyInterface)

  # NfcTarget
  set_source_files_properties(NfcTarget.xml PROPERTIES CLASSNAME NfcTargetInterface)
  qt4_add_dbus_interface(desknfc_SOURCES NfcTarget.xml NfcTargetInterface)

# Qt Definitions
add_definitions(${QT_DEFINITIONS})

# Additionnal definitions
if(CMAKE_COMPILER_IS_GNU_CC)
  add_definitions(-std=c99)
  # Make sure we will not miss some warnings ;)
  add_definitions(-Wall -pedantic)
endif(CMAKE_COMPILER_IS_GNU_CC)

# Generate MOC files
qt4_wrap_cpp( desknfc_MOC ${desknfc_MOCABLE_HEADERS} )

# Current Project Include
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable( desknfc ${desknfc_SOURCES} ${desknfc_MOC} ${desknfc_QM})

kde4_add_app_icon(desknfc_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/icons/desknfc.png")

target_link_libraries( desknfc ${QT_LIBRARIES} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} )

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/desknfc DESTINATION ${BIN_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/desknfc.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES ${desknfc_QM} DESTINATION  ${DATA_INSTALL_DIR}/desknfc/i18n)

