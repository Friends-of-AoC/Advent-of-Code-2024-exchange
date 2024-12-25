// SPDX-License-Identifier: AGPL-3.0-or-later

/*
 * color.h
 *
 *  Created on: Dec 14, 2024
 *      Author: pat
 */

#ifndef SRC_COLOR_H_
#define SRC_COLOR_H_

#include "control.h"

/* basic stuff like coloring */

#define CSI ESC "["
#define C_SEP ";"
#define C_END "m"

#define C_RESET             "0"
#define C_BOLD              "1"
#define C_LESS_INTENSE      "2"
#define C_ITALIC            "3"
#define C_UNDERLINE         "4"
#define C_REVERSE_FB        "7"
#define C_STRIKE_THROUGH    "9"
#define C_DEF_INTENSE       "22"
#define C_NO_ITALIC         "23"
#define C_NO_UNDERLINE      "24"
#define C_NO_REVERSE_FB     "27"
#define C_NO_STRIKE_THROUGH "29"
#define C_FC_DEF            "39"
#define C_BC_DEF            "49"

#define C_FC_PREFIX "3"
#define C_BC_PREFIX "4"

#define C_BLACK   "0"
#define C_RED     "1"
#define C_GREEN   "2"
#define C_YELLOW  "3"
#define C_BLUE    "4"
#define C_MAGENTA "5"
#define C_CYAN    "6"
#define C_WHITE   "7"

#define C_RGB "8" C_SEP "2" C_SEP

#define C_RGB_GRAY       C_RGB "128" C_SEP "128" C_SEP "128"
#define C_RGB_LIGHT_GRAY C_RGB "192" C_SEP "192" C_SEP "192"
#define C_RGB_DARK_GRAY  C_RGB  "64" C_SEP  "64" C_SEP  "64"

#define RESET             CSI C_RESET C_END
#define BOLD              CSI C_BOLD C_END
#define LESS_INTENSE      CSI C_LESS_INTENSE C_END
#define ITALIC            CSI C_ITALIC C_END
#define UNDERLINE         CSI C_UNDERLINE C_END
#define REVERSE_FB        CSI C_REVERSE_FB C_END
#define STRIKE_THROUGH    CSI C_STRIKE_THROUGH C_END
#define DEF_INTENSE       CSI C_DEF_INTENSE C_END
#define NO_ITALIC         CSI C_NO_ITALIC C_END
#define NO_UNDERLINE      CSI C_NO_UNDERLINE C_END
#define NO_REVERSE_FB     CSI C_NO_REVERSE_FB C_END
#define NO_STRIKE_THROUGH CSI C_NO_STRIKE_THROUGH C_END
#define FC_DEF            CSI C_FC_DEF C_END
#define BC_DEF            CSI C_BC_DEF C_END
#define FC_BLACK          CSI C_FC_PREFIX C_BLACK C_END
#define FC_RED            CSI C_FC_PREFIX C_RED C_END
#define FC_GREEN          CSI C_FC_PREFIX C_GREEN C_END
#define FC_YELLOW         CSI C_FC_PREFIX C_YELLOW C_END
#define FC_BLUE           CSI C_FC_PREFIX C_BLUE C_END
#define FC_MAGENTA        CSI C_FC_PREFIX C_MAGENTA C_END
#define FC_CYAN           CSI C_FC_PREFIX C_CYAN C_END
#define FC_WHITE          CSI C_FC_PREFIX C_WHITE C_END
#define FC_GRAY           CSI C_FC_PREFIX C_RGB_GRAY C_END
#define FC_LIGHT_GRAY     CSI C_FC_PREFIX C_RGB_LIGHT_GRAY C_END
#define FC_DARK_GRAY      CSI C_FC_PREFIX C_RGB_DARK_GRAY C_END
#define BC_BLACK          CSI C_BC_PREFIX C_BLACK C_END
#define BC_RED            CSI C_BC_PREFIX C_RED C_END
#define BC_GREEN          CSI C_BC_PREFIX C_GREEN C_END
#define BC_YELLOW         CSI C_BC_PREFIX C_YELLOW C_END
#define BC_BLUE           CSI C_BC_PREFIX C_BLUE C_END
#define BC_MAGENTA        CSI C_BC_PREFIX C_MAGENTA C_END
#define BC_CYAN           CSI C_BC_PREFIX C_CYAN C_END
#define BC_WHITE          CSI C_BC_PREFIX C_WHITE C_END
#define BC_GRAY           CSI C_BC_PREFIX C_RGB_GRAY C_END
#define BC_LIGHT_GRAY     CSI C_BC_PREFIX C_RGB_LIGHT_GRAY C_END
#define BC_DARK_GRAY      CSI C_BC_PREFIX C_RGB_DARK_GRAY C_END

#define BC_RGB(r,g,b) CSI C_BC_PREFIX C_RGB #r C_SEP #g C_SEP #b C_END
#define FC_RGB(r,g,b) CSI C_FC_PREFIX C_RGB #r C_SEP #g C_SEP #b C_END

#define FRMT_BC_RGB BC_RGB(%u, %u, %u)
#define FRMT_FC_RGB FC_RGB(%u, %u, %u)

#endif /* SRC_COLOR_H_ */
