PROJECT(desknfc)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

# Qt4
FIND_PACKAGE(Qt4 REQUIRED)
SET(QT_USE_QTDBUS true)
INCLUDE(${QT_USE_FILE})

# KDE4
FIND_PACKAGE( KDE4 REQUIRED )
include_directories( ${KDE4_INCLUDES} )

# Sources files
SET(desknfc_SOURCES main.cpp NfcUiManager.cpp NotifyAction.cpp Notification.cpp Content.cpp
			NfcTarget.cpp NfcDevice.cpp NfcDeviceManager.cpp)

# MOCable headers
SET(desknfc_MOCABLE_HEADERS NfcUiManager.h NotifyAction.h Notification.h Content.h
			NfcTarget.h NfcDevice.h NfcDeviceManager.h)

# UI Classes
SET(desknfc_UI ui/MainWindow.ui ui/ContentWidget.ui ui/TargetWidget.ui)
QT4_WRAP_UI(desknfc_SOURCES ${desknfc_UI})

# Additonnal ressources
QT4_ADD_RESOURCES(desknfc_SOURCES systray.qrc)

# Translation
SET(desknfc_TS i18n/fr_FR.ts)
SET(REBUILD_TRANSLATIONS OFF CACHE BOOL "Update source translation i18n/*.ts (WARNING: make clean will delete the source .ts files! Danger!)")
IF (REBUILD_TRANSLATIONS)
  QT4_CREATE_TRANSLATION(desknfc_QM ${desknfc_SOURCES} ${desknfc_UI} ${desknfc_TS})
ELSE (REBUILD_TRANSLATIONS)
  QT4_ADD_TRANSLATION(desknfc_QM ${desknfc_TS})
ENDIF (REBUILD_TRANSLATIONS) 

ADD_CUSTOM_TARGET (translations_target DEPENDS ${desknfc_QM})


# Generate D-Bus interface
  # NfcDeviceManager
  SET_SOURCE_FILES_PROPERTIES(NfcDeviceManager.xml PROPERTIES CLASSNAME NfcDeviceManagerInterface)
  QT4_ADD_DBUS_INTERFACE(desknfc_SOURCES NfcDeviceManager.xml NfcDeviceManagerInterface)

  # NfcDevice
  SET_SOURCE_FILES_PROPERTIES(NfcDevice.xml PROPERTIES CLASSNAME NfcDeviceInterface)
  QT4_ADD_DBUS_INTERFACE(desknfc_SOURCES NfcDevice.xml NfcDeviceInterface)

  # Notify
  SET_SOURCE_FILES_PROPERTIES(Notify.xml PROPERTIES CLASSNAME NotifyInterface)
  QT4_ADD_DBUS_INTERFACE(desknfc_SOURCES Notify.xml NotifyInterface)

  # NfcTarget
  SET_SOURCE_FILES_PROPERTIES(NfcTarget.xml PROPERTIES CLASSNAME NfcTargetInterface)
  QT4_ADD_DBUS_INTERFACE(desknfc_SOURCES NfcTarget.xml NfcTargetInterface)

# Qt Definitions
ADD_DEFINITIONS(${QT_DEFINITIONS})

# Additionnal definitions
IF(CMAKE_COMPILER_IS_GNU_CC)
  ADD_DEFINITIONS(-std=c99)
  # Make sure we will not miss some warnings ;)
  ADD_DEFINITIONS(-Wall -pedantic)
ENDIF(CMAKE_COMPILER_IS_GNU_CC)

# Generate MOC files
QT4_WRAP_CPP( desknfc_MOC ${desknfc_MOCABLE_HEADERS} )

# Current Project Include
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE( desknfc ${desknfc_SOURCES} ${desknfc_MOC} ${desknfc_QM})

TARGET_LINK_LIBRARIES( desknfc ${QT_LIBRARIES} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} )

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/desknfc DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/img/desknfc.png DESTINATION pixmaps)
install(FILES ${desknfc_QM} DESTINATION  i18n)

