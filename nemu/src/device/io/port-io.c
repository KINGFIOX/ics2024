/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <device/map.h>

#include "isa.h"

#define PORT_IO_SPACE_MAX 65535

#define NR_MAP 16

static IOMap maps[NR_MAP] = {};
static int nr_map = 0;

/* device interface */
void add_pio_map(const char *name, ioaddr_t addr, void *space, uint32_t len, io_callback_t callback) {
  assert(nr_map < NR_MAP);
  assert(addr + len <= PORT_IO_SPACE_MAX);
  maps[nr_map] = (IOMap){.name = name, .low = addr, .high = addr + len - 1, .space = space, .callback = callback};
  Log("Add port-io map '%s' at [" FMT_PADDR ", " FMT_PADDR "]", maps[nr_map].name, maps[nr_map].low, maps[nr_map].high);

  nr_map++;
}

/* CPU interface */
uint32_t pio_read(ioaddr_t addr, int len) {
  assert(addr + len - 1 < PORT_IO_SPACE_MAX);
  int mapid = find_mapid_by_addr(maps, nr_map, addr);
  assert(mapid != -1);
  IOMap *map = &maps[mapid];
  word_t data = map_read(addr, len, map);
#ifdef CONFIG_DTRACE
  void dtrace_log(vaddr_t pc, word_t data, const char *name, bool is_pio, paddr_t addr, int len, bool is_write);
  dtrace_log(cpu.pc, data, map->name, true, addr, len, false);
#endif
  return data;
}

void pio_write(ioaddr_t addr, int len, uint32_t data) {
  assert(addr + len - 1 < PORT_IO_SPACE_MAX);
  int mapid = find_mapid_by_addr(maps, nr_map, addr);
  assert(mapid != -1);
  IOMap *map = &maps[mapid];
#ifdef CONFIG_DTRACE
  void dtrace_log(vaddr_t pc, word_t data, const char *name, bool is_pio, paddr_t addr, int len, bool is_write);
  dtrace_log(cpu.pc, data, map->name, true, addr, len, true);
#endif
  map_write(addr, len, data, map);
}
