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
#include "profiler_memory.h"

#include "m_ctype.h"

#include <cstdio>
#include "variables.h"

#include <thread>
#include <chrono>
#include <iomanip>

int dump_count= 1;

void startHeapProfilerWithTimeout(const std::string &dumpPath,
                                  int timeoutSeconds)
{
  // Start the heap profiler
  fp_HeapProfilerStart(dumpPath.c_str());
  fp_HeapProfilerDump("starting");
  std::ostringstream filename;
  filename << get_dump_path() << "." << std::setw(4) << std::setfill('0')
           << dump_count << ".heap";
  std::string filePath= filename.str();
  ++dump_count;

  // Launch a separate thread to stop the profiler after the timeout
  std::thread([timeoutSeconds]() {
    std::this_thread::sleep_for(std::chrono::seconds(timeoutSeconds));
    fp_HeapProfilerDump("timeout");
    fp_HeapProfilerStop();
    set_profiler_memory_status(false);
  }).detach(); // Detach the thread to allow it to run independently
}

String *Item_func_memprof_report::val_str(String *str)
{
  std::string out_str= "Not implemented yet";

  int limit= 0;
  std::string report_type= "TEXT";

  const uint n= arg_count;

  std::string end_dump_file= std::string(get_dump_path()) + ".*.heap";

  if (n >= 1)
  {
    std::string first_arg= item_to_string(args[0], str);
    if (is_report_type(first_arg))
      report_type= first_arg;
    else
      end_dump_file= first_arg;
  }

  // limit (2nd arg, index 1)
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

  // report_type (3rd arg, index 2)
  if (n >= 3 && args[2]->val_str(str))
  {
    report_type.assign(args[2]->val_str(str)->c_ptr_safe(),
                       args[2]->val_str(str)->length());
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

  std::string pprof_binary= std::string(get_pprof_binary());
  std::string mariadb_binary= get_mariadb_server_binary();

  std::vector<std::string> pprof_args{
      pprof_binary, "--" + pprof_output_flag(normalized), mariadb_binary};
  append_profile_args(&pprof_args, end_dump_file);
  out_str= exec_pprof(pprof_args);

  if (normalized == "TEXT")
    out_str= strip_pprof_text_preamble(out_str);
  else if (normalized == "DOT")
    out_str= strip_pprof_dot_preamble(out_str);

  if (limit > 0 && normalized == "TEXT")
  {
    out_str= limit_lines(out_str, limit);
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

String *Item_func_memprof_diff::val_str(String *str)
{
  std::string out_str= "Not implemented yet";

  int limit= 0;
  std::string report_type= "TEXT";

  const uint n= arg_count;

  // base_dump_file
  std::string base_dump_file=
      (n >= 1 && args[0]->val_str(str))
          ? std::string(args[0]->val_str(str)->c_ptr_safe(),
                        args[0]->val_str(str)->length())
          : (std::string(get_dump_path()) + ".0001.heap");

  // end_dump_file
  std::string end_dump_file=
      (n >= 2 && args[1]->val_str(str))
          ? std::string(args[1]->val_str(str)->c_ptr_safe(),
                        args[1]->val_str(str)->length())
          : (std::string(get_dump_path()) + ".*.heap");

  // limit (3rd arg, index 2)
  if (n >= 3)
  {
    longlong v= args[2]->val_int();
    if (v > 0)
      limit= static_cast<int>(v);
  }

  // report_type (4th arg, index 3)
  if (n >= 4 && args[3]->val_str(str))
  {
    report_type.assign(args[3]->val_str(str)->c_ptr_safe(),
                       args[3]->val_str(str)->length());
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

  std::string pprof_binary= std::string(get_pprof_binary());
  std::string mariadb_binary= get_mariadb_server_binary();

  std::vector<std::string> pprof_args{
      pprof_binary, "--" + pprof_output_flag(normalized),
      "--base=" + base_dump_file, mariadb_binary};
  append_profile_args(&pprof_args, end_dump_file);
  out_str= exec_pprof(pprof_args);

  if (normalized == "TEXT")
    out_str= strip_pprof_text_preamble(out_str);
  else if (normalized == "DOT")
    out_str= strip_pprof_dot_preamble(out_str);

  if (limit > 0 && normalized == "TEXT")
  {
    out_str= limit_lines(out_str, limit);
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

String *Item_func_memprof_start::val_str(String *str)
{
  std::string out_str;

  if (arg_count > 1)
  {
    my_error(ER_WRONG_PARAMCOUNT_TO_NATIVE_FCT, MYF(0),
             func_name_cstring().str);
    null_value= 1;
    return nullptr;
  }
  if (HAS(FEAT_TCMALLOC_HEAP_PROF))
  {

    // check if we already have a profiler running
    if (strcmp(get_profiler_memory_status(), "ON") == 0)
    {
      out_str= "HeapProfiler already running";
    }
    else if (has_dump_with_prefix(get_dump_path()))
    {
      std::string msg=
          std::string("file with prefix (" + std::string(get_dump_path()) +
                      ".NNNN.heap) already exists");
      my_error(ER_CANT_CREATE_FILE, MYF(0), msg.c_str(), EEXIST);
      null_value= true;
      return nullptr;
    }
    else if (arg_count == 1)
    {
      startHeapProfilerWithTimeout(get_dump_path(), args[0]->val_int());
      out_str= "HeapProfiler started with timeout " +
               std::to_string(args[0]->val_int()) + " seconds to " +
               get_dump_path() + ".NNNN.heap";
      set_profiler_memory_status(true);
    }
    else
    {
      fp_HeapProfilerStart(get_dump_path());
      fp_HeapProfilerDump("starting");
      out_str= std::string("HeapProfiler started, use MEMPROF_STOP() to stop "
                           "it, dumps to ") +
               get_dump_path() + ".NNNN.heap";
      set_profiler_memory_status(true);
    }
  }
  else
  {
    out_str= "HeapProfiler not available (tcmalloc not in use)";
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

String *Item_func_memprof_stop::val_str(String *str)
{

  std::string out_str;

  if (strcmp(get_profiler_memory_status(), "OFF") == 0)
  {
    out_str= "HeapProfiler not running";
  }
  else if (HAS(FEAT_TCMALLOC_HEAP_PROF))
  {
    fp_HeapProfilerDump("stopping");
    fp_HeapProfilerStop();
    set_profiler_memory_status(false);
    out_str= "HeapProfiler stopped";
  }
  else
  {
    out_str= "HeapProfiler not available (tcmalloc not in use)";
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

String *Item_func_memprof_dump::val_str(String *str)
{

  std::string out_str;

  if (strcmp(get_profiler_memory_status(), "OFF") == 0)
  {
    out_str= "HeapProfiler not running";
  }
  else if (HAS(FEAT_TCMALLOC_HEAP_PROF))
  {
    fp_HeapProfilerDump("user request");
    out_str= "HeapProfiler dump requested";
  }
  else
  {
    out_str= "HeapProfiler not available (tcmalloc not in use)";
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
