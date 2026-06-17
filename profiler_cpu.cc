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
#include "profiler_cpu.h"

#include "m_ctype.h"
#include "variables.h"

#include <cerrno>
#include <chrono>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>

static std::mutex cpu_profiler_lock;

static bool cpu_profile_file_exists()
{
  return access(get_cpu_profile_path(), F_OK) == 0;
}

static void stop_cpu_profiler()
{
  fp_ProfilerFlush();
  fp_ProfilerStop();
  set_profiler_cpu_status(false);
}

static void start_cpu_profiler_with_timeout(int timeout_seconds)
{
  set_profiler_cpu_status(true);
  std::thread([timeout_seconds]() {
    std::this_thread::sleep_for(std::chrono::seconds(timeout_seconds));
    std::lock_guard<std::mutex> guard(cpu_profiler_lock);
    if (strcmp(get_profiler_cpu_status(), "ON") == 0)
      stop_cpu_profiler();
  }).detach();
}

String *Item_func_cpuprof_start::val_str(String *str)
{
  std::string out_str;

  if (arg_count > 1)
  {
    my_error(ER_WRONG_PARAMCOUNT_TO_NATIVE_FCT, MYF(0),
             func_name_cstring().str);
    null_value= true;
    return nullptr;
  }

  std::lock_guard<std::mutex> guard(cpu_profiler_lock);

  if (!HAS(FEAT_TCMALLOC_CPU_PROF))
  {
    out_str= "CpuProfiler not available (tcmalloc not in use)";
  }
  else if (strcmp(get_profiler_cpu_status(), "ON") == 0)
  {
    out_str= "CpuProfiler already running";
  }
  else if (cpu_profile_file_exists())
  {
    std::string msg= std::string("file (") + get_cpu_profile_path() +
                     ") already exists";
    my_error(ER_CANT_CREATE_FILE, MYF(0), msg.c_str(), EEXIST);
    null_value= true;
    return nullptr;
  }
  else if (!fp_ProfilerStart(get_cpu_profile_path()))
  {
    out_str= "CpuProfiler could not be started";
  }
  else if (arg_count == 1)
  {
    longlong timeout= args[0]->val_int();
    start_cpu_profiler_with_timeout(static_cast<int>(timeout));
    out_str= "CpuProfiler started with timeout " + std::to_string(timeout) +
             " seconds to " + get_cpu_profile_path();
  }
  else
  {
    set_profiler_cpu_status(true);
    out_str= std::string("CpuProfiler started, use CPUPROF_STOP() to stop it, "
                         "profile to ") +
             get_cpu_profile_path();
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

String *Item_func_cpuprof_stop::val_str(String *str)
{
  std::string out_str;

  std::lock_guard<std::mutex> guard(cpu_profiler_lock);

  if (strcmp(get_profiler_cpu_status(), "OFF") == 0)
  {
    out_str= "CpuProfiler not running";
  }
  else if (HAS(FEAT_TCMALLOC_CPU_PROF))
  {
    stop_cpu_profiler();
    out_str= "CpuProfiler stopped";
  }
  else
  {
    out_str= "CpuProfiler not available (tcmalloc not in use)";
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

String *Item_func_cpuprof_flush::val_str(String *str)
{
  std::string out_str;

  std::lock_guard<std::mutex> guard(cpu_profiler_lock);

  if (strcmp(get_profiler_cpu_status(), "OFF") == 0)
  {
    out_str= "CpuProfiler not running";
  }
  else if (HAS(FEAT_TCMALLOC_CPU_PROF))
  {
    fp_ProfilerFlush();
    out_str= "CpuProfiler flush requested";
  }
  else
  {
    out_str= "CpuProfiler not available (tcmalloc not in use)";
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

String *Item_func_cpuprof_report::val_str(String *str)
{
  std::string out_str;
  int limit= 0;
  std::string report_type= "TEXT";
  std::string profile_file= get_cpu_profile_path();
  const uint n= arg_count;

  if (n >= 1)
  {
    std::string first_arg= item_to_string(args[0], str);
    if (is_report_type(first_arg))
      report_type= first_arg;
    else
      profile_file= first_arg;
  }

  if (n >= 2)
  {
    std::string second_arg= item_to_string(args[1], str);
    if (is_report_type(second_arg))
      report_type= second_arg;
    else
    {
      longlong v= args[1]->val_int();
      if (v > 0)
        limit= static_cast<int>(v);
    }
  }

  std::string normalized= normalized_report_type(report_type);
  if (normalized != "TEXT" && normalized != "DOT")
  {
    std::string msg= report_type + "', must be 'TEXT' or 'DOT";
    my_error(ER_WRONG_VALUE_FOR_VAR, MYF(0), func_name_cstring().str,
             msg.c_str());
    null_value= true;
    return nullptr;
  }

  {
    std::lock_guard<std::mutex> guard(cpu_profiler_lock);
    if (strcmp(get_profiler_cpu_status(), "ON") == 0 &&
        HAS(FEAT_TCMALLOC_CPU_PROF))
      fp_ProfilerFlush();
  }

  std::vector<std::string> pprof_args{get_pprof_binary(),
                                      "--" + pprof_output_flag(normalized),
                                      get_mariadb_server_binary()};
  append_profile_args(&pprof_args, profile_file);
  out_str= exec_pprof(pprof_args);

  if (normalized == "TEXT")
    out_str= strip_pprof_text_preamble(out_str);
  else if (normalized == "DOT")
    out_str= strip_pprof_dot_preamble(out_str);

  if (limit > 0 && normalized == "TEXT")
    out_str= limit_lines(out_str, limit);

  if (str->copy(out_str.c_str(), static_cast<uint>(out_str.size()),
                collation.collation))
  {
    null_value= true;
    return nullptr;
  }

  null_value= false;
  return str;
}
