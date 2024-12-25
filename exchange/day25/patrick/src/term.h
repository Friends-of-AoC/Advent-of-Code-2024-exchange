// SPDX-License-Identifier: AGPL-3.0-or-later

/*
 * term.h
 *
 *  Created on: Dec 14, 2024
 *      Author: pat
 */

#ifndef SRC_TERM_H_
#define SRC_TERM_H_

#include "color.h"

/* more advanced stuff */

#define C_CURSOR_UP        "A"
#define C_CURSOR_DOWN      "B"
#define C_CURSOR_FORWARD   "C"
#define C_CURSOR_BACK      "D"
#define C_CURSOR_NEXT_LINE "E"
#define C_CURSOR_PREV_LINE "F"
#define C_CURSOR_SET_COLUM "G"
#define C_CURSOR_SET       "H"

#define C_ERASE_IN_DISPLAY "J"
#define C_ERASE_IN_LINE    "K"

#define CURSOR_UP_ONE           CSI C_CURSOR_UP
#define CURSOR_DOWN_ONE         CSI C_CURSOR_DOWN
#define CURSOR_FORWARD_ONE      CSI C_CURSOR_FORWARD
#define CURSOR_BACK_ONE         CSI C_CURSOR_BACK
#define CURSOR_NEXT_LINE        CSI C_CURSOR_NEXT_LINE
#define CURSOR_PREV_LINE        CSI C_CURSOR_PREV_LINE
#define CURSOR_START_OF_LINE    CSI C_CURSOR_SET_COLUM
#define CURSOR_START_OF_DISPLAY CSI C_CURSOR_SET

/* moves n steps in the given direction */
#define CURSOR_FORWARD(count) CSI #count C_CURSOR_FORWARD
#define CURSOR_BACK(count)    CSI #count C_CURSOR_BACK
#define CURSOR_UP(count)      CSI #count C_CURSOR_UP
#define CURSOR_DOWN(count)    CSI #count C_CURSOR_DOWN

#define FRMT_CURSOR_FORWARD CURSOR_FORWARD(%u)
#define FRMT_CURSOR_BACK    CURSOR_BACK(%u)
#define FRMT_CURSOR_UP      CURSOR_UP(%u)
#define FRMT_CURSOR_DOWN    CURSOR_DOWN(%u)

/* line cursor up/down, but also set the column to 1 */
#define CURSOR_UP_START(count)    CSI #count C_CURSOR_PREV_LINE
#define CURSOR_DOWN_START(count)  CSI #count C_CURSOR_NEXT_LINE

#define FRMT_CURSOR_DOWN_START  CURSOR_DOWN_START(%u)
#define FRMT_CURSOR_UP_START    CURSOR_UP_START(%u)

/* set the cursors position */
#define CURSOR_SET(lin, col)   CSI #lin C_SEP #col C_CURSOR_SET
#define FRMT_CURSOR_SET        CURSOR_SET(%u,%u)

/* set the cursor to the start of the given line */
#define CURSOR_SET_LINE(lin)   CSI #lin C_CURSOR_SET
#define FRMT_CURSOR_SET_LINE   CURSOR_SET_LINE(%u)

/* set the cursor to the given column */
#define CURSOR_SET_COLUMN(count) CSI #count C_CURSOR_SET_COLUM
#define FRMT_CURSOR_SET_COLUMN CURSOR_SET_COLUMN(%u)

#define CURSOR_GET CSI "6n"

#define SCROLL_PAGE_UP CSI "S"
#define SCROLL_PAGE_DOWN CSI "S"

/* set the window title */
#define SCROLL_PAGES_UP(count) CSI #count C_SCROLL_PAGE_UP
#define SCROLL_PAGES_DOWN(count) CSI #count C_SCROLL_PAGE_DOWN

#define C_SCROLL_PAGE_UP "S"
#define C_SCROLL_PAGE_DOWN "S"

#define FRMT_SCROLL_PAGE_UP SCROLL_PAGE_UP(%u)
#define FRMT_SCROLL_PAGE_DOWN SCROLL_PAGE_DOWN(%u)

#define C_END_OF_STRING ESC "\\"

#define TITLE_START ESC "]0"
#define TITLE_END   C_END_OF_STRING

/* set the window title */
#define TITLE(title) TITLE_START #title TITLE_END
#define FRMT_TITLE TITLE(%s)

#define SHOW_CURSOR CSI "?25h"
#define HIDE_CURSOR CSI "?25l"

#define ERASE_END_OF_DISPLAY   CSI C_ERASE_IN_DISPLAY
#define ERASE_START_OF_DISPLAY CSI "1" C_ERASE_IN_DISPLAY
#define ERASE_COMPLETE_DISPLAY CSI "2" C_ERASE_IN_DISPLAY

#define ERASE_END_OF_LINE   CSI C_ERASE_IN_LINE
#define ERASE_START_OF_LINE CSI "1" C_ERASE_IN_LINE
#define ERASE_COMPLETE_LINE CSI "2" C_ERASE_IN_LINE

#endif /* SRC_TERM_H_ */
