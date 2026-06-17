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
#include "mysql/plugin.h"
#include "variables.h"

// SYSVARs for the profiler plugin

char *g_dump_path= nullptr;
char *g_pprof_binary= nullptr;

static MYSQL_SYSVAR_STR(dump_path, g_dump_path,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
                        "Path for tcmalloc profiler dumps",
                        NULL,               // check
                        NULL,               // update
                        "/tmp/memprof_dump" // default
);

static MYSQL_SYSVAR_STR(pprof_binary, g_pprof_binary,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
                        "Path to the pprof binary",
                        NULL,            // check
                        NULL,            // update
                        "/usr/bin/pprof" // default
);

// Array of your plugin’s sysvars
struct st_mysql_sys_var *profiler_sysvars[]= {
    MYSQL_SYSVAR(dump_path), MYSQL_SYSVAR(pprof_binary), NULL};

// STATUSVARs for the profiler plugin

// Definition of the shared flag
std::atomic<const char *> g_profiler_memory_status{"OFF"};

// SHOW_FUNC getter so the server asks us at SHOW time
static int show_profiler_memory_status(MYSQL_THD,
                                       struct st_mysql_show_var *var, char *)
{
  var->type= SHOW_CHAR;
  var->value=
      (char *) get_profiler_memory_status(); // safe: points to string literal
  return 0;
}

struct st_mysql_show_var profiler_statusvars[]= {
    {"tcmalloc_profiler_memory_status", (char *) show_profiler_memory_status,
     SHOW_FUNC},
    {nullptr, nullptr, SHOW_UNDEF}};
