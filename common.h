/* Copyright (c) 2011, 2013, Oracle and/or its affiliates. All rights reserved.
   Copyright (c) 2014 MariaDB Foundation
   Copyright (c) 2019 MariaDB Corporation
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

#ifndef PROFILER_COMMON_H
#define PROFILER_COMMON_H

#include "item.h"

#include "sql_class.h" // THD, Security_context
#include "sql_acl.h"

#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <cstdio>
#include <dlfcn.h>

enum Feature : unsigned
{
  FEAT_TCMALLOC_HEAP_PROF= 1u << 0, // HeapProfiler{Start,Dump,Stop}
  FEAT_JEMALLOC_MALLCTL= 1u << 1,   // je_mallctl available
  FEAT_TCMALLOC_CPU_PROF= 1u << 2,  // Profiler{Start,Stop,Flush}
};

inline std::atomic<unsigned> g_features{0};
inline std::once_flag g_feat_once;

#define HAS(F) ((g_features.load(std::memory_order_relaxed) & (F)) != 0)

// ---------- Cached function pointers (optional but handy) ----------
using HeapProfilerDumpFn= void (*)(const char *);
using HeapProfilerStartFn= void (*)(const char *);
using HeapProfilerStopFn= void (*)();

inline HeapProfilerDumpFn fp_HeapProfilerDump= nullptr;
inline HeapProfilerStartFn fp_HeapProfilerStart= nullptr;
inline HeapProfilerStopFn fp_HeapProfilerStop= nullptr;

using CpuProfilerStartFn= int (*)(const char *);
using CpuProfilerStopFn= void (*)();
using CpuProfilerFlushFn= void (*)();

inline CpuProfilerStartFn fp_ProfilerStart= nullptr;
inline CpuProfilerStopFn fp_ProfilerStop= nullptr;
inline CpuProfilerFlushFn fp_ProfilerFlush= nullptr;

using JeMallctlFn= int (*)(const char *, void *, size_t *, void *, size_t);
inline JeMallctlFn fp_je_mallctl= nullptr;

inline void detect_allocator_features_once()
{
  std::call_once(g_feat_once, [] {
    auto sym= [](const char *s) { return dlsym(RTLD_DEFAULT, s); };

    // tcmalloc heap-profiler exists only in full libtcmalloc (not *_minimal)
    fp_HeapProfilerDump=
        reinterpret_cast<HeapProfilerDumpFn>(sym("HeapProfilerDump"));
    fp_HeapProfilerStart=
        reinterpret_cast<HeapProfilerStartFn>(sym("HeapProfilerStart"));
    fp_HeapProfilerStop=
        reinterpret_cast<HeapProfilerStopFn>(sym("HeapProfilerStop"));
    if (fp_HeapProfilerDump && fp_HeapProfilerStart && fp_HeapProfilerStop)
      g_features.fetch_or(FEAT_TCMALLOC_HEAP_PROF, std::memory_order_relaxed);

    fp_ProfilerStart=
        reinterpret_cast<CpuProfilerStartFn>(sym("ProfilerStart"));
    fp_ProfilerStop= reinterpret_cast<CpuProfilerStopFn>(sym("ProfilerStop"));
    fp_ProfilerFlush=
        reinterpret_cast<CpuProfilerFlushFn>(sym("ProfilerFlush"));
    if (fp_ProfilerStart && fp_ProfilerStop && fp_ProfilerFlush)
      g_features.fetch_or(FEAT_TCMALLOC_CPU_PROF, std::memory_order_relaxed);

    // jemalloc control interface
    fp_je_mallctl= reinterpret_cast<JeMallctlFn>(sym("je_mallctl"));
    if (fp_je_mallctl)
      g_features.fetch_or(FEAT_JEMALLOC_MALLCTL, std::memory_order_relaxed);
  });
}

inline std::string get_provider(void *fn)
{
  if (HAS(FEAT_TCMALLOC_HEAP_PROF))
  {
    return "tcmalloc";
  }
  else if (HAS(FEAT_JEMALLOC_MALLCTL))
  {
    return "jemalloc (no profiling support with this plugin)";
  }
  else
  {
    return "glibc memory allocator (no profiling support)";
  }
}

extern bool has_dump_with_prefix(const std::string &prefix_path);
extern int remove_dump_files_with_prefix(const std::string &prefix_path,
                                         std::string *failed_path,
                                         int *error_code);
extern int remove_file_if_exists(const std::string &path,
                                 std::string *failed_path, int *error_code);
extern std::string get_mariadb_server_binary();
extern std::string exec_pprof(const std::vector<std::string> &argv);
extern std::string limit_lines(const std::string &input, size_t max_lines);
extern std::string item_to_string(Item *item, String *str);
extern std::string normalized_report_type(const std::string &report_type);
extern std::string pprof_output_flag(const std::string &normalized);
extern bool is_report_type(const std::string &value);
extern void append_profile_args(std::vector<std::string> *argv,
                                const std::string &profile_arg);
extern std::string strip_pprof_text_preamble(const std::string &report);
extern std::string strip_pprof_dot_preamble(const std::string &report);

#endif // PROFILER_COMMON_H
