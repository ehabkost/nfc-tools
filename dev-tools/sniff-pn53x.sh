#!/bin/bash

#######################################################################
# Linux USB sniffer with PN53x basic parsing
# Copyright (C) 2010, Yobibe
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#######################################################################

# Warning! usbmon is truncating long data packets,
# e.g. with PN533 we get only the first 24 bytes after D4/D5

# Usages:

# (default):  decode data, e.g.:
#   d4 02                                   -> GetFirmwareVersion
#   d5 03 32 01 04 07

# --nodecode: don't try to interpret data, e.g.:
#   d402
#   d50332010407

# --internal: reader-dependent frames, e.g.:
# with ACR122U: (output colorized to highlight Tama content)
#   -> ff00480000
#   <- 41435231323255313032
#   -> ff00000002d402
#   <- 6108
#   -> ffc0000008
#   <- d503320104079000
# with PN533: (output colorized to highlight Tama content)
#   -> 0000ff00ff00
#   -> 0000ff02fed4022a00
#   <- 0000ff00ff00
#   <- 0000ff06fad50333020707e500

# --rawusb:   as from usbmon
# with PN533:
#   ffff88000ca45480 3985505915 S Ci:7:045:0 s 80 06 0300 0000 00ff 255 <
#   ffff88000ca45480 3985508248 C Ci:7:045:0 0 4 = 04030904
#   ffff88000ca45480 3985508282 S Ci:7:045:0 s 80 06 0301 0409 00ff 255 <
#   ffff88000ca45480 3985511235 C Ci:7:045:0 0 20 = 14035300 43004d00 20004d00 69006300 72006f00
#   ffff88000ca45480 3985511295 S Ci:7:045:0 s 80 06 0300 0000 00ff 255 <
#   ffff88000ca45480 3985513250 C Ci:7:045:0 0 4 = 04030904
#   ffff88000ca45480 3985513301 S Ci:7:045:0 s 80 06 0302 0409 00ff 255 <
#   ffff88000ca45480 3985516428 C Ci:7:045:0 0 30 = 1e035300 43004c00 33003700 31003100 2d004e00 46004300 26005200 5700
#   ffff88000ca45cc0 3985516625 S Co:7:045:0 s 00 09 0001 0000 0000 0
#   ffff88000ca45cc0 3985518230 C Co:7:045:0 0 0
#   ffff88000ca45cc0 3985518297 S Bo:7:045:4 -115 6 = 0000ff00 ff00
#   ffff88000ca45cc0 3985519229 C Bo:7:045:4 0 6 >
#   ffff88000ca45cc0 3985519263 S Bo:7:045:4 -115 9 = 0000ff02 fed4022a 00
#   ffff88000ca45cc0 3985520253 C Bo:7:045:4 0 9 >
#   ffff880010d31240 3985520300 S Bi:7:045:4 -115 256 <
#   ffff880010d31240 3985521251 C Bi:7:045:4 0 6 = 0000ff00 ff00
#   ffff880010d31240 3985521284 S Bi:7:045:4 -115 256 <
#   ffff880010d31240 3985522430 C Bi:7:045:4 0 13 = 0000ff06 fad50333 020707e5 00

# I try to detect devices in the following order:
DEVICES="pn53x_usb touchatag"

# Find right bus
for DEVICE in $DEVICES
do
    if [ "$DEVICE" == "touchatag" ]
    then
        # USB identifier string
        DEVID="(072f:90cc|072f:2200)"
    elif [ "$DEVICE" == "pn53x_usb" ]
    then
        # USB identifier string PN531/PN533
        DEVID="(054c:0193|04cc:0531|04cc:2533|04e6:5591)"
    fi
    USBBUS=$(lsusb|egrep -m 1 "$DEVID"|cut -d ' ' -f2|sed 's/^0\+//')
    if [ "$USBBUS" != "" ]
    then
        echo "Found $DEVICE on bus $USBBUS" >&2
        break
    fi
done
if [ "$USBBUS" == "" ]
then
    echo "Could not find any reader, sorry"
    exit 1
fi

# Prepare kernel interface for USBMON
ls /sys/kernel/debug| grep -q . || sudo mount -t debugfs none_debugs /sys/kernel/debug
lsmod | grep -q usbmon || sudo modprobe usbmon

FILTER1=true
DECODE=true
OUTPUT=--only-matching

case $1 in
    "--nodecode")
        DECODE=false
    ;;
    "--internal")
        DECODE=false
        OUTPUT=--color
    ;;
    "--rawusb")
        FILTER1=false
        DECODE=false
        OUTPUT=--color
    ;;
esac

if [ "$DEVICE" == "touchatag" ]
then
    function filter1 {
        # APDUs in bulk transfers
        grep --line-buffered "B[io]:.*= ........ ........ ........" | sed -u 's/ //g;s/.*Bi.*=..................../<- /;s/.*Bo.*=..................../-> /'|\
        grep -C999 --line-buffered $OUTPUT -P '((?<=-> ff000000..).*|(?<=<- ).*(?=9000))'
    }
elif [ "$DEVICE" == "pn53x_usb" ]
then
    function filter1 {
        # PN53x USB frames
        grep --line-buffered "B[io]:.*= ........" | sed -u 's/ //g;s/.*Bi.*=/<- /;s/.*Bo.*=/-> /' |\
        grep -C999 --line-buffered $OUTPUT -P '(?<=.. 0000ff(?!ffff)....)(..).*(?=....)|(?<=.. 0000ffffff......)(..).*(?=....)'
    }
fi

function get_usb_data {
    if [ ! -e "/sys/kernel/debug/usb/usbmon/${USBBUS}u" ]
    then
        echo "Error device $DEVICE not found" >&2
        exit 1
    fi
    sudo cat /sys/kernel/debug/usb/usbmon/${USBBUS}u
}

function decode_pn53x {
    sed -u 's/^[<>-][<>-] //;s/\([0-9a-f]\{2\}\)/\1 /g'|awk '
    function showline(col2) {
        if (done) return
        col1=$0
        col1width=40
        printf col1;
        for (i=length(); i<col1width; i++){printf " "};
        printf col2 "\n"
        fflush()
        done=1
    }
    // {done=0}
    /^d4 00/ { showline("-> Diagnose") }
    /^d4 02/ { showline("-> GetFirmwareVersion") }
    /^d4 04/ { showline("-> GetGeneralStatus") }
    /^d4 06/ { showline("-> ReadRegister 0x" $3 $4 ) }
    /^d4 08/ { showline("-> WriteRegister 0x" $3 $4 ) }
    /^d4 0c/ { showline("-> ReadGPIO") }
    /^d4 0e/ { showline("-> WriteGPIO") }
    /^d4 10/ { showline("-> SetSerialBaudRate") }           # PN531 PN532
    /^d4 12/ { showline("-> SetParameters: 0x" $3) }
    /^d4 14/ { showline("-> SAMConfiguration") }            # PN531 PN532
    /^d4 16/ { showline("-> PowerDown") }                   # PN531 PN532
    /^d4 18/ { showline("-> AlparCommandForTDA") }          # PN533
    /^d4 32 01/ { showline("-> RFConfiguration RFfield: " $4) }
    /^d4 32 02/ { showline("-> RFConfiguration Timings: " $4 " " $5 " " $6) }
    /^d4 32 04/ { showline("-> RFConfiguration MaxRtyCOM: " 44) }
    /^d4 32 05/ { showline("-> RFConfiguration MaxRetries: " $4 " " $5 " " $6) }
    /^d4 32/ { showline("-> RFConfiguration unknown") }
    /^d4 32/ { showline("-> RFConfiguration") }
    /^d4 38/ { showline("-> InQuartetByteExchange") }       # PN533
    /^d4 40/ { showline("-> InDataExchange") }
    /^d4 42/ { showline("-> InCommunicateThru") }
    /^d4 44/ { showline("-> InDeselect") }
    /^d4 46/ { showline("-> InJumpForPSL") }
    /^d4 48/ { showline("-> InActivateDeactivatePaypass") } # PN533
    /^d4 4a/ { showline("-> InListPassiveTarget") }
    /^d4 4e/ { showline("-> InPSL") }
    /^d4 50/ { showline("-> InATR") }
    /^d4 52/ { showline("-> InRelease") }
    /^d4 54/ { showline("-> InSelect") }
    /^d4 56/ { showline("-> InJumpForDEP") }
    /^d4 58/ { showline("-> RFRegulationTest") }
    /^d4 60/ { showline("-> InAutoPoll") }                  # PN532
    /^d4 86/ { showline("-> TgGetData") }
    /^d4 88/ { showline("-> TgGetInitiatorCommand") }
    /^d4 8a/ { showline("-> TgGetTargetStatus") }
    /^d4 8c/ { showline("-> TgInitAsTarget") }
    /^d4 8e/ { showline("-> TgSetData") }
    /^d4 90/ { showline("-> TgResponseToInitiator") }
    /^d4 92/ { showline("-> TgSetGeneralBytes") }
    /^d4 94/ { showline("-> TgSetMetaData") }               # PN531 PN532
    /^d4 96/ { showline("-> TgSetDataSecure") }             # PN533
    /^d4 98/ { showline("-> TgSetMetaData") }               # PN533
    /^7f/    { showline("<- resp error frame") }
    !done
'
}

if   $DECODE
then
    get_usb_data | filter1 | decode_pn53x
elif $FILTER1
then
    get_usb_data | filter1
else
    get_usb_data
fi
