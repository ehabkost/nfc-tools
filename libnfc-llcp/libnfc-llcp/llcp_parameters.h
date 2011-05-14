/*-
 * Copyright (C) 2011, Romain Tartière
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
 */

/*
 * $Id$
 */

#ifndef _LLCP_PARAMETERS_H
#define _LLCP_PARAMETERS_H

#include <sys/types.h>

#include "llcp.h"

#define LLCP_PARAMETER_VERSION 0x01
#define LLCP_PARAMETER_MIUX    0x02
#define LLCP_PARAMETER_WKS     0x03
#define LLCP_PARAMETER_LTO     0x04
#define LLCP_PARAMETER_RW      0x05
#define LLCP_PARAMETER_SN      0x06
#define LLCP_PARAMETER_OPT     0x07

int	 parameter_encode_version (uint8_t buffer[], size_t buffer_len, struct llcp_version version);
int	 parameter_decode_version (const uint8_t buffer[], size_t buffer_len, struct llcp_version *version);
int	 parameter_encode_miux (uint8_t buffer[], size_t buffer_len, uint16_t miux);
int	 parameter_decode_miux (const uint8_t buffer[], size_t buffer_len, uint16_t *miux);
int	 parameter_encode_wks (uint8_t buffer[], size_t buffer_len, uint16_t wks);
int	 parameter_decode_wks (const uint8_t buffer[], size_t buffer_len, uint16_t *wks);
int	 parameter_encode_lto (uint8_t buffer[], size_t buffer_len, uint8_t lto);
int	 parameter_decode_lto (const uint8_t buffer[], size_t buffer_len, uint8_t *lto);
int	 parameter_encode_rw (uint8_t buffer[], size_t buffer_len, uint8_t rw);
int	 parameter_decode_rw (const uint8_t buffer[], size_t buffer_len, uint8_t *rw);
int	 parameter_encode_sn (uint8_t buffer[], size_t buffer_len, const char *sn);
int	 parameter_decode_sn (const uint8_t buffer[], size_t buffer_len, char *sn, size_t sn_max_len);
int	 parameter_encode_opt (uint8_t buffer[], size_t buffer_len, uint8_t opt);
int	 parameter_decode_opt (const uint8_t buffer[], size_t buffer_len, uint8_t *opt);

#endif /* !_LLCP_PARAMETERS_H */
