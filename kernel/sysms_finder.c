// SPDX-License-Identifier: GPL-2.0

#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/kallsyms.h>
#include <linux/printk.h>
#include <linux/types.h>

#include <linux/sysms_finder.h>

#define SYSMS_FINDER_RETRY_INTERVAL	msecs_to_jiffies(5000)

struct symbol_entry {
	const char *name;
	unsigned long addr;
	unsigned long next_retry;
};

static struct symbol_entry symbols_status[NR_SYMBOLS] = {
	[SYMBOL_GAME_PID] = {
		.name = "game_pid",
	},
};

unsigned long lookup_symbol(int symbol_index)
{
	struct symbol_entry *entry;
	unsigned long addr;
	unsigned long next_retry;

	if (symbol_index < 0 || symbol_index >= NR_SYMBOLS)
		return 0;

	entry = &symbols_status[symbol_index];
	addr = READ_ONCE(entry->addr);
	if (addr)
		return addr;

	next_retry = READ_ONCE(entry->next_retry);
	if (next_retry && time_before(jiffies, next_retry))
		return 0;

	addr = kallsyms_lookup_name(entry->name);
	if (addr) {
		WRITE_ONCE(entry->addr, addr);
		pr_info("sysms_finder: %s found\n", entry->name);
		return addr;
	}

	WRITE_ONCE(entry->next_retry, jiffies + SYSMS_FINDER_RETRY_INTERVAL);
	pr_err_ratelimited("sysms_finder: Error looking up %s\n", entry->name);

	return 0;
}

bool check_game_pid(void)
{
	pid_t *var_ptr;
	pid_t game_pid;
	unsigned long addr;

	addr = lookup_symbol(SYMBOL_GAME_PID);
	if (!addr)
		return true;

	var_ptr = (pid_t *)addr;
	game_pid = READ_ONCE(*var_ptr);

	return game_pid == -1;
}
