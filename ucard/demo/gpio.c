/*-
 * Copyright (C) 2010, Audrey Diacre.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * $Id$
 */

#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "gpio.h"

#define GPIO_RED_LED    "/dev/gpio/2"
#define GPIO_GREEN_LED  "/dev/gpio/0"
#define GPIO_BLUE_LED   "/dev/gpio/1"
int fd_red;
int fd_green;
int fd_blue;

void init_led(){

    if( ( fd_red = open( GPIO_RED_LED, O_RDWR ) ) < 0 ) {
        perror( "open: " GPIO_RED_LED );
        exit( EXIT_FAILURE );
    }
    if( ( fd_green = open( GPIO_GREEN_LED, O_RDWR ) ) < 0 ) {
        perror( "open: " GPIO_GREEN_LED );
        exit( EXIT_FAILURE );
    }
    if( ( fd_blue = open( GPIO_BLUE_LED, O_RDWR ) ) < 0 ) {
        perror( "open: " GPIO_BLUE_LED );
        exit( EXIT_FAILURE );
    }
    if( ioctl( fd_red, GPIO_CONFIG_AS_OUT ) < 0 ) {
        close( fd_red );
        perror( "ioctl: " GPIO_RED_LED );
        exit( EXIT_FAILURE );
    }
    if( ioctl( fd_green, GPIO_CONFIG_AS_OUT ) < 0 ) {
        close( fd_green );
        perror( "ioctl: " GPIO_GREEN_LED );
        exit( EXIT_FAILURE );
    }
    if( ioctl( fd_blue, GPIO_CONFIG_AS_OUT ) < 0 ) {
        close( fd_blue );
        perror( "ioctl: " GPIO_BLUE_LED );
        exit( EXIT_FAILURE );
    }
    rgb_led(0, 0, 1);
}

void rgb_led(int red, int green, int blue){
    int ret_val;
    if( ( ret_val = write( fd_red, (char *)&red, sizeof( char ) ) ) != sizeof( char ) ){
        perror( "write: " GPIO_RED_LED );
        close( fd_red );
        exit( EXIT_FAILURE );
    }
    if( ( ret_val = write( fd_green, (char *)&green, sizeof( char ) ) ) != sizeof( char ) ){
        perror( "write: " GPIO_GREEN_LED );
        close( fd_green );
        exit( EXIT_FAILURE );
    }
    if( ( ret_val = write( fd_blue, (char *)&blue, sizeof( char ) ) ) != sizeof( char ) ){
        perror( "write: " GPIO_BLUE_LED );
        close( fd_blue );
        exit( EXIT_FAILURE );
    }
}
