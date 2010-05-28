project(desknfc)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

set(PACKAGE_NAME "desknfc")
set(PACKAGE_VERSION "0.0.0")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")

# Qt4
find_package(Qt4 REQUIRED)
set(QT_USE_QTDBUS true)
include(${QT_USE_FILE})

# KDE4
find_package( KDE4 REQUIRED )
include_directories( ${KDE4_INCLUDES} )
include(KDE4Defaults)

# If definitions like -D_GNU_SOURCE are needed for these checks they
# should be added to _KDE4_PLATFORM_DEFINITIONS when it is originally
# defined outside this file.  Here we include these definitions in
# CMAKE_REQUIRED_DEFINITIONS so they will be included in the build of
# checks below.
set( CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS} )

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )
remove_definitions(-DQT_NO_STL)
include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${LIBKNOTIFICATIONITEM-1_INCLUDE_DIR})

set(PACKAGE_DATA_INSTALL_DIR ${DATA_INSTALL_DIR}/${PACKAGE_NAME})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

# Icons
add_subdirectory( icons )

# Documentation
#set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
include(UseDoxygen)

# Sources files
set(desknfc_SRCS
#  main.cpp
#  NfcUiManager.cpp
  Content.cpp
  NfcTarget.cpp
  NfcDevice.cpp
  NfcDeviceManager.cpp
  Notification.cpp
  ui/DesknfcPlasmoid.cpp
#  ui/DeviceItem.cpp
  ui/NotifierDialog.cpp
  ui/NotifierView.cpp
)

# UI Classes
#kde4_add_ui_files(desknfc_SRCS
#  ui/MainWindow.ui
#  ui/ContentWidget.ui
#  ui/TargetWidget.ui
#)

# Translation
set(desknfc_TS i18n/fr_FR.ts)
set(rebuild_translations off cache bool "update source translation i18n/*.ts (WARNING: make clean will delete the source .ts files! Danger!)")
if (REBUILD_TRANSLATIONS)
  qt4_create_translation(desknfc_QM ${desknfc_SRCS} ${desknfc_UI} ${desknfc_TS})
else (REBUILD_TRANSLATIONS)
  qt4_add_translation(desknfc_QM ${desknfc_TS})
endif (REBUILD_TRANSLATIONS) 

add_custom_target (translations_target DEPENDS ${desknfc_QM})

# Generate D-Bus interface
  # NfcDeviceManager
  set_source_files_properties(NfcDeviceManager.xml PROPERTIES CLASSNAME NfcDeviceManagerInterface)
  qt4_add_dbus_interface(desknfc_SRCS NfcDeviceManager.xml NfcDeviceManagerInterface)

  # NfcDevice
  set_source_files_properties(NfcDevice.xml PROPERTIES CLASSNAME NfcDeviceInterface)
  qt4_add_dbus_interface(desknfc_SRCS NfcDevice.xml NfcDeviceInterface)

  # NfcTarget
  set_source_files_properties(NfcTarget.xml PROPERTIES CLASSNAME NfcTargetInterface)
  qt4_add_dbus_interface(desknfc_SRCS NfcTarget.xml NfcTargetInterface)


# Additionnal definitions
if(CMAKE_COMPILER_IS_GNU_CC)
  # Set C99 standard flag
  add_definitions(-std=c99)
  # Make sure we will not miss some warnings ;)
  add_definitions(-Wall -pedantic)
endif(CMAKE_COMPILER_IS_GNU_CC)

# Current Project Include
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

kde4_add_app_icon(desknfc_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/desknfc.png")

kde4_add_plugin( plasma_applet_desknfc ${desknfc_SRCS} ${desknfc_QM})

target_link_libraries( plasma_applet_desknfc
  ${QT_LIBRARIES}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KPARTS_LIBS}
  ${KDE4_PLASMA_LIBS}
  ${KDE4_SOLID_LIBS}
)

install(TARGETS plasma_applet_desknfc DESTINATION ${PLUGIN_INSTALL_DIR} )


#install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/desknfc DESTINATION ${BIN_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/plasma-applet-desknfc.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES ${desknfc_QM} DESTINATION  ${PACKAGE_DATA_INSTALL_DIR}/i18n)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/desknfc.png DESTINATION ${PACKAGE_DATA_INSTALL_DIR}/icons)
install(FILES desknfc.notifyrc DESTINATION ${PACKAGE_DATA_INSTALL_DIR})
