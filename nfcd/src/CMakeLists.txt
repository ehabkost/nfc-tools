# Sources files
    SET(nfcd_SOURCES main.cpp NfcDevice.cpp NfcDeviceManager.cpp NfcTarget.cpp
      Entry.cpp MimeContentEntry.cpp Content.cpp UriEntry.cpp TitleEntry.cpp)

# Generate D-Bus adaptor
    QT4_ADD_DBUS_ADAPTOR(nfcd_SOURCES NfcDeviceManager.xml NfcDeviceManager.h NfcDeviceManager )
    QT4_ADD_DBUS_ADAPTOR(nfcd_SOURCES NfcDevice.xml NfcDevice.h NfcDevice )
    QT4_ADD_DBUS_ADAPTOR(nfcd_SOURCES NfcTarget.xml NfcTarget.h NfcTarget ) 

    SET(nfcd_MOCABLE_HEADERS NfcDevice.h NfcDeviceManager.h NfcTarget.h
     Entry.h MimeContentEntry.h Content.h UriEntry.h TitleEntry.h)

# Qt Definitions
    ADD_DEFINITIONS(${QT_DEFINITIONS})

# Additionnal definitions
    ADD_DEFINITIONS( -Wall )
    ADD_DEFINITIONS( -ggdb )
    ADD_DEFINITIONS( -O0 )
QT4_WRAP_CPP( nfcd_MOC_SOURCES ${nfcd_MOCABLE_HEADERS} )

ADD_EXECUTABLE( nfcd ${nfcd_SOURCES} ${nfcd_MOC_SOURCES} )

TARGET_LINK_LIBRARIES( nfcd ${QT_LIBRARIES} ${LIBNFC_LIBRARIES} ${LIBFREEFARE_LIBRARIES} -lndef)

if(CMAKE_SYSTEM_NAME MATCHES Linux)
  set(PREFIX /usr) 
endif(CMAKE_SYSTEM_NAME MATCHES Linux)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/nfcd DESTINATION ${PREFIX}/bin)
