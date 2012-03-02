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

#ifndef GPIO_H_
#define GPIO_H_

typedef enum {
	IRQ_HIGH,
	IRQ_LOW,
	IRQ_RISING,
	IRQ_FALLING
}ext_irq_type_t;


/* ioctl magic numbers */
#define GPIO_IOCTL_BASE		'G'


/* inputs */
#define GPIO_CONFIG_AS_INP	_IO  (GPIO_IOCTL_BASE, 0)	/* config this pin as input */
#define GPIO_READ_PIN_VAL	_IOR (GPIO_IOCTL_BASE, 1, int)	/* read pin value */

/* outputs */
#define GPIO_CONFIG_AS_OUT	_IO  (GPIO_IOCTL_BASE, 2)	/* config this pin as output */
#define GPIO_WRITE_PIN_VAL	_IOW (GPIO_IOCTL_BASE, 3, int)	/* sets the pin value */

/* irqs */
#define GPIO_CONFIG_AS_IRQ	_IOR (GPIO_IOCTL_BASE, 4, ext_irq_type_t)	/* config this pin as interrupt */

#define GPIO_IOCTL_MAXNR	4

void             rgb_led(int red, int green, int blue);
void             init_led();

#endif /* GPIO_H_ */



