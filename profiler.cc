/* Copyright (c) 2019,2024, MariaDB Corporation
   Copyright (c) 2025 lefred (Frédéric Descamps)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1335  USA */

#define MYSQL_SERVER
#include "mariadb.h"
#include "profiler.h"

#include "m_ctype.h"

#include <cstdio>
#include "common.h"
#include "variables.h"

String *Item_func_profiler_info::val_str(String *str)
{

  void *cur_malloc= dlsym(RTLD_DEFAULT, "malloc");
  std::string out_str=
      "TCMalloc Profiler plugin by lefred, current allocator: " +
      get_provider(cur_malloc);

  if (str->copy(out_str.c_str(), static_cast<uint>(out_str.size()),
                collation.collation))
  {
    null_value= true;
    return nullptr;
  }

  null_value= false;
  return str;
}

String *Item_func_cleanup::val_str(String *str)
{

  std::string out_str;

  if (strcmp(get_profiler_memory_status(), "ON") == 0)
  {
    out_str= "HeapProfiler running";
  }
  else if (strcmp(get_profiler_cpu_status(), "ON") == 0)
  {
    out_str= "CpuProfiler running";
  }
  else
  {
    std::string failed_path;
    int error_code= 0;
    if (remove_dump_files_with_prefix(get_dump_path(), &failed_path,
                                      &error_code) < 0)
    {
      my_error(ER_CANT_DELETE_FILE, MYF(0),
               failed_path.empty() ? get_dump_path() : failed_path.c_str(),
               error_code);
      null_value= true;
      return nullptr;
    }
    if (remove_file_if_exists(get_cpu_profile_path(), &failed_path,
                              &error_code) < 0)
    {
      my_error(ER_CANT_DELETE_FILE, MYF(0),
               failed_path.empty() ? get_cpu_profile_path()
                                   : failed_path.c_str(),
               error_code);
      null_value= true;
      return nullptr;
    }
    out_str= "Profiling data has been cleaned up";
  }

  if (str->copy(out_str.c_str(), static_cast<uint>(out_str.size()),
                collation.collation))
  {
    null_value= true;
    return nullptr;
  }

  null_value= false;
  return str;
}
