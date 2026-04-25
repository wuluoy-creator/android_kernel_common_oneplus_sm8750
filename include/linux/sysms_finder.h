/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_GAME_PID_H
#define _LINUX_GAME_PID_H
#include <linux/types.h>


enum {
	SYMBOL_GAME_PID,
	NR_SYMBOLS,
};

unsigned long lookup_symbol(int symbol_index);
bool check_game_pid(void);

#endif /* _LINUX_GAME_PID_H */
