/*
   Copyright (c) 2019, MariaDB Corporation
   Copyright (c) 2025, lefred (Frédéric Descamps)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <atomic>

// STATUS for the profiler plugin

extern std::atomic<const char *> g_profiler_memory_status;

extern char *g_dump_path;
extern char *g_pprof_binary;

// Status/sysvar arrays are only declared here
struct st_mysql_show_var;
struct st_mysql_sys_var;

extern struct st_mysql_show_var profiler_statusvars[];
extern struct st_mysql_sys_var *profiler_sysvars[];

// Helpers
inline void set_profiler_memory_status(bool on)
{
  g_profiler_memory_status.store(on ? "ON" : "OFF", std::memory_order_release);
}
inline const char *get_profiler_memory_status()
{
  return g_profiler_memory_status.load(std::memory_order_acquire);
}

inline const char *get_dump_path()
{
  return g_dump_path ? g_dump_path : "/tmp/memprof_dump";
}
inline const char *get_pprof_binary()
{
  return g_pprof_binary ? g_pprof_binary : "/usr/bin/pprof";
}
