// SPDX-License-Identifier: AGPL-3.0-or-later

/*
 * control.h
 *
 *  Created on: Dec 15, 2024
 *      Author: pat
 */

#ifndef SRC_CONTROL_H_
#define SRC_CONTROL_H_

/* Null */
#define NUL_C '\x00'
#define NUL   "\x00"

/* Start of Heading */
#define SOH_C '\x01'
#define SOH   "\x01"

/* Start of Text */
#define STX_C '\x02'
#define STX   "\x02"

/* End of Text */
#define ETX_C '\x03'
#define ETX   "\x03"

/* End of Transmission */
#define EOT_C '\x04'
#define EOT   "\x04"

/* Enquiry */
#define ENQ_C '\x05'
#define ENQ   "\x05"
#define WRU_C ENQ_C
#define WRU   ENQ

/* Acknowledge */
#define ACK_C '\x06'
#define ACK   "\x06"

/* Bell, Alert */
#define BEL_C '\x07'
#define BEL   "\x07"

/* Backspace */
#define BS_C  '\x08'
#define BS    "\x08"

/* Character Tabulation, Horizontal Tabulation */
#define HT_C  '\x09'
#define HT    "\x09"

/* Line Feed */
#define LF_C  '\x0A'
#define LF    "\x0A"

/* Line Tabulation, Vertical Tabulation */
#define VT_C  '\x0B'
#define VT    "\x0B"

/* Form Feed */
#define FF_C  '\x0C'
#define FF    "\x0C"

/* Carriage Return */
#define CR_C  '\x0D'
#define CR    "\x0D"

/* Shift Out */
#define SO_C  '\x0E'
#define SO    "\x0E"

/* Shift In */
#define SI_C  '\x0F'
#define SI    "\x0F"

/* Data Link Escape */
#define DLE_C '\x10'
#define DLE   "\x10"

/* Device Control One */
#define XON_C '\x11'
#define XON   "\x11"

/* Device Control Two */
#define TAPEON_C '\x12'
#define TAPEON   "\x12"

/* Device Control Three */
#define XOFF_C '\x13'
#define XOFF   "\x13"

/* Device Control Four */
#define TAPEOFF_C '\x14'
#define TAPEOFF   "\x14"

/* Negative Acknowledge */
#define NAK_C '\x15'
#define NAK   "\x15"

/* Synchronous Idle */
#define SYN_C '\x16'
#define SYN   "\x16"

/* End of Transmission Block */
#define ETB_C '\x17'
#define ETB   "\x17"

/* Cancel */
#define CAN_C '\x18'
#define CAN   "\x18"

/* End of medium */
#define EM_C  '\x19'
#define EM    "\x19"

/* Substitute */
#define SUB_C '\x1A'
#define SUB   "\x1A"

/* Escape */
#define ESC_C '\x1B'
#define ESC   "\x1B"

/* File Separator */
#define FS_C  '\x1C'
#define FS    "\x1C"

/* Group Separator */
#define GS_C  '\x1D'
#define GS    "\x1D"

/* Record Separator */
#define RS_C  '\x1E'
#define RS    "\x1E"

/* Unit Separator */
#define US_C  '\x1F'
#define US    "\x1F"

/* Space */
#define SP_C  '\x20'
#define SP    "\x20"

/* Delete */
#define DEL_C '\x7F'
#define DEL   "\x7F"

#endif /* SRC_CONTROL_H_ */
