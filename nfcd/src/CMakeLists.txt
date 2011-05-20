# Sources files
set(nfcd_SOURCES
  NFCd.cpp
  NfcDevice.cpp
  NfcDeviceManager.cpp
#  NfcForumManager.cpp
#  NfcForumTag.cpp
  NfcTarget.cpp
#  iso14443b.c
#  nfcforum_type4_tag.c
)

# Generate D-Bus adaptor
qt4_add_dbus_adaptor(nfcd_SOURCES NfcDeviceManager.xml NfcDeviceManager.h NfcDeviceManager )
qt4_add_dbus_adaptor(nfcd_SOURCES NfcDevice.xml NfcDevice.h NfcDevice )
qt4_add_dbus_adaptor(nfcd_SOURCES NfcTarget.xml NfcTarget.h NfcTarget )
#qt4_add_dbus_adaptor(nfcd_SOURCES NfcForumManager.xml NfcForumManager.h NfcForumManager )
#qt4_add_dbus_adaptor(nfcd_SOURCES NfcForumTag.xml NfcForumTag.h NfcForumTag )

set(nfcd_MOCABLE_HEADERS
  NfcDevice.h
  NfcDeviceManager.h
  NfcTarget.h
#  NfcForumManager.h
#  NfcForumTag.h
)

# Qt Definitions
add_definitions(${QT_DEFINITIONS})

# Additionnal definitions
qt4_wrap_cpp( nfcd_MOC_SOURCES ${nfcd_MOCABLE_HEADERS} )

add_executable( nfcd ${nfcd_SOURCES} ${nfcd_MOC_SOURCES} )

target_link_libraries( nfcd
  ${QT_LIBRARIES}
  ${LIBNFC_LIBRARIES}
  ${LIBFREEFARE_LIBRARIES}
)

install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/nfcd DESTINATION bin )
#install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/nfcd.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )
