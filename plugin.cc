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

#define MYSQL_SERVER

#include <mariadb.h>
#include "profiler.h"
#include "profiler_cpu.h"
#include "profiler_memory.h"
#include <sql_class.h>
#include <mysql/plugin_function.h>
#include "variables.h"

static int profiler_init(void *)
{
  detect_allocator_features_once();
  return 0;
}

static int profiler_deinit(void *) { return 0; }

class Create_func_profiler_info : public Create_func_arg0
{
public:
  Item *create_builder(THD *thd) override
  {
    return new (thd->mem_root) Item_func_profiler_info(thd);
  }
  static Create_func_profiler_info s_singleton;

protected:
  Create_func_profiler_info() {}
  ~Create_func_profiler_info() override {}
};

Create_func_profiler_info Create_func_profiler_info::s_singleton;

class Create_func_cleanup : public Create_func_arg0
{
public:
  Item *create_builder(THD *thd) override
  {
    return new (thd->mem_root) Item_func_cleanup(thd);
  }
  static Create_func_cleanup s_singleton;

protected:
  Create_func_cleanup() {}
  ~Create_func_cleanup() override {}
};

Create_func_cleanup Create_func_cleanup::s_singleton;

class Create_func_memprof_start : public Create_native_func
{
public:
  Item *create_native(THD *thd, const LEX_CSTRING *name,
                      List<Item> *item_list) override
  {
    const uint n= item_list ? item_list->elements : 0;
    if (n == 0)
      return new (thd->mem_root) Item_func_memprof_start(thd);
    if (n == 1)
      return new (thd->mem_root) Item_func_memprof_start(thd, *item_list);

    my_error(ER_WRONG_PARAMCOUNT_TO_NATIVE_FCT, MYF(0), name->str);
    return nullptr;
  }
  static Create_func_memprof_start s_singleton;

protected:
  Create_func_memprof_start() {}
  ~Create_func_memprof_start() override {}
};

Create_func_memprof_start Create_func_memprof_start::s_singleton;

class Create_func_memprof_stop : public Create_func_arg0
{
public:
  Item *create_builder(THD *thd) override
  {
    return new (thd->mem_root) Item_func_memprof_stop(thd);
  }
  static Create_func_memprof_stop s_singleton;

protected:
  Create_func_memprof_stop() {}
  ~Create_func_memprof_stop() override {}
};

Create_func_memprof_stop Create_func_memprof_stop::s_singleton;

class Create_func_memprof_dump : public Create_func_arg0
{
public:
  Item *create_builder(THD *thd) override
  {
    return new (thd->mem_root) Item_func_memprof_dump(thd);
  }
  static Create_func_memprof_dump s_singleton;

protected:
  Create_func_memprof_dump() {}
  ~Create_func_memprof_dump() override {}
};

Create_func_memprof_dump Create_func_memprof_dump::s_singleton;

class Create_func_memprof_report : public Create_native_func
{
public:
  Item *create_native(THD *thd, const LEX_CSTRING *name,
                      List<Item> *item_list) override
  {
    const uint n= item_list ? item_list->elements : 0;
    if (n == 0)
      return new (thd->mem_root) Item_func_memprof_report(thd);
    if (n < 4)
      return new (thd->mem_root) Item_func_memprof_report(thd, *item_list);
    my_error(ER_WRONG_PARAMCOUNT_TO_NATIVE_FCT, MYF(0), name->str);
    return nullptr;
  }
  static Create_func_memprof_report s_singleton;

protected:
  Create_func_memprof_report() {}
  ~Create_func_memprof_report() override {}
};

Create_func_memprof_report Create_func_memprof_report::s_singleton;

class Create_func_memprof_diff : public Create_native_func
{
public:
  Item *create_native(THD *thd, const LEX_CSTRING *name,
                      List<Item> *item_list) override
  {
    const uint n= item_list ? item_list->elements : 0;
    if (n == 0)
      return new (thd->mem_root) Item_func_memprof_diff(thd);
    if (n < 4)
      return new (thd->mem_root) Item_func_memprof_diff(thd, *item_list);
    my_error(ER_WRONG_PARAMCOUNT_TO_NATIVE_FCT, MYF(0), name->str);
    return nullptr;
  }
  static Create_func_memprof_diff s_singleton;

protected:
  Create_func_memprof_diff() {}
  ~Create_func_memprof_diff() override {}
};

Create_func_memprof_diff Create_func_memprof_diff::s_singleton;

class Create_func_cpuprof_start : public Create_native_func
{
public:
  Item *create_native(THD *thd, const LEX_CSTRING *name,
                      List<Item> *item_list) override
  {
    const uint n= item_list ? item_list->elements : 0;
    if (n == 0)
      return new (thd->mem_root) Item_func_cpuprof_start(thd);
    if (n == 1)
      return new (thd->mem_root) Item_func_cpuprof_start(thd, *item_list);

    my_error(ER_WRONG_PARAMCOUNT_TO_NATIVE_FCT, MYF(0), name->str);
    return nullptr;
  }
  static Create_func_cpuprof_start s_singleton;

protected:
  Create_func_cpuprof_start() {}
  ~Create_func_cpuprof_start() override {}
};

Create_func_cpuprof_start Create_func_cpuprof_start::s_singleton;

class Create_func_cpuprof_stop : public Create_func_arg0
{
public:
  Item *create_builder(THD *thd) override
  {
    return new (thd->mem_root) Item_func_cpuprof_stop(thd);
  }
  static Create_func_cpuprof_stop s_singleton;

protected:
  Create_func_cpuprof_stop() {}
  ~Create_func_cpuprof_stop() override {}
};

Create_func_cpuprof_stop Create_func_cpuprof_stop::s_singleton;

class Create_func_cpuprof_flush : public Create_func_arg0
{
public:
  Item *create_builder(THD *thd) override
  {
    return new (thd->mem_root) Item_func_cpuprof_flush(thd);
  }
  static Create_func_cpuprof_flush s_singleton;

protected:
  Create_func_cpuprof_flush() {}
  ~Create_func_cpuprof_flush() override {}
};

Create_func_cpuprof_flush Create_func_cpuprof_flush::s_singleton;

class Create_func_cpuprof_report : public Create_native_func
{
public:
  Item *create_native(THD *thd, const LEX_CSTRING *name,
                      List<Item> *item_list) override
  {
    const uint n= item_list ? item_list->elements : 0;
    if (n == 0)
      return new (thd->mem_root) Item_func_cpuprof_report(thd);
    if (n < 3)
      return new (thd->mem_root) Item_func_cpuprof_report(thd, *item_list);

    my_error(ER_WRONG_PARAMCOUNT_TO_NATIVE_FCT, MYF(0), name->str);
    return nullptr;
  }
  static Create_func_cpuprof_report s_singleton;

protected:
  Create_func_cpuprof_report() {}
  ~Create_func_cpuprof_report() override {}
};

Create_func_cpuprof_report Create_func_cpuprof_report::s_singleton;

#define BUILDER(F) &F::s_singleton

static Plugin_function plugin_descriptor_function_profiler_info(
    BUILDER(Create_func_profiler_info)),
    plugin_descriptor_function_cleanup(BUILDER(Create_func_cleanup)),
    plugin_descriptor_function_memprof_start(
        BUILDER(Create_func_memprof_start)),
    plugin_descriptor_function_memprof_stop(BUILDER(Create_func_memprof_stop)),
    plugin_descriptor_function_memprof_report(
        BUILDER(Create_func_memprof_report)),
    plugin_descriptor_function_memprof_diff(BUILDER(Create_func_memprof_diff)),
    plugin_descriptor_function_memprof_dump(BUILDER(Create_func_memprof_dump)),
    plugin_descriptor_function_cpuprof_start(
        BUILDER(Create_func_cpuprof_start)),
    plugin_descriptor_function_cpuprof_stop(BUILDER(Create_func_cpuprof_stop)),
    plugin_descriptor_function_cpuprof_flush(
        BUILDER(Create_func_cpuprof_flush)),
    plugin_descriptor_function_cpuprof_report(
        BUILDER(Create_func_cpuprof_report));

/*************************************************************************/

maria_declare_plugin(type_test){
    MariaDB_FUNCTION_PLUGIN, // the plugin type (see include/mysql/plugin.h)
    &plugin_descriptor_function_profiler_info, // pointer to type-specific
                                               // plugin descriptor
    "tcmalloc_profiler",                       // plugin name
    "lefred",                                  // plugin author
    "TCMalloc Profiler plugin for MariaDB (system variables)", // the plugin
                                                               // description
    PLUGIN_LICENSE_GPL,  // the plugin license (see include/mysql/plugin.h)
    profiler_init,       // Pointer to plugin initialization function
    profiler_deinit,     // Pointer to plugin deinitialization function
    0x0100,              // Numeric version 0xAABB means AA.BB version
    profiler_statusvars, // Status variables
    profiler_sysvars,    // System variables
    "1.0",               // String version representation
    MariaDB_PLUGIN_MATURITY_EXPERIMENTAL // Maturity(see
                                         // include/mysql/plugin.h)*/
},
    {
        MariaDB_FUNCTION_PLUGIN,                   // the plugin type (see
                                                   // include/mysql/plugin.h)
        &plugin_descriptor_function_memprof_start, // pointer to type-specific
                                                   // plugin descriptor
        "tcmalloc_memprof_start",                  // plugin name
        "lefred",                                  // plugin author
        "Function TCMALLOC_MEMPROF_START()",       // the plugin description
        PLUGIN_LICENSE_GPL, // the plugin license (see include/mysql/plugin.h)
        0,                  // Pointer to plugin initialization function
        0,                  // Pointer to plugin deinitialization function
        0x0100,             // Numeric version 0xAABB means AA.BB version
        NULL,               // Status variables
        NULL,               // System variables
        "1.0",              // String version representation
        MariaDB_PLUGIN_MATURITY_EXPERIMENTAL // Maturity(see
                                             // include/mysql/plugin.h)*/
    },
    {
        MariaDB_FUNCTION_PLUGIN,                  // the plugin type (see
                                                  // include/mysql/plugin.h)
        &plugin_descriptor_function_memprof_stop, // pointer to type-specific
                                                  // plugin descriptor
        "tcmalloc_memprof_stop",                  // plugin name
        "lefred",                                 // plugin author
        "Function TCMALLOC_MEMPROF_STOP()",       // the plugin description
        PLUGIN_LICENSE_GPL, // the plugin license (see include/mysql/plugin.h)
        0,                  // Pointer to plugin initialization function
        0,                  // Pointer to plugin deinitialization function
        0x0100,             // Numeric version 0xAABB means AA.BB version
        NULL,               // Status variables
        NULL,               // System variables
        "1.0",              // String version representation
        MariaDB_PLUGIN_MATURITY_EXPERIMENTAL // Maturity(see
                                             // include/mysql/plugin.h)*/
    },
    {
        MariaDB_FUNCTION_PLUGIN,                  // the plugin type (see
                                                  // include/mysql/plugin.h)
        &plugin_descriptor_function_memprof_dump, // pointer to type-specific
                                                  // plugin descriptor
        "tcmalloc_memprof_dump",                  // plugin name
        "lefred",                                 // plugin author
        "Function TCMALLOC_MEMPROF_DUMP()",       // the plugin description
        PLUGIN_LICENSE_GPL, // the plugin license (see include/mysql/plugin.h)
        0,                  // Pointer to plugin initialization function
        0,                  // Pointer to plugin deinitialization function
        0x0100,             // Numeric version 0xAABB means AA.BB version
        NULL,               // Status variables
        NULL,               // System variables
        "1.0",              // String version representation
        MariaDB_PLUGIN_MATURITY_EXPERIMENTAL // Maturity(see
                                             // include/mysql/plugin.h)*/
    },
    {
        MariaDB_FUNCTION_PLUGIN,                    // the plugin type (see
                                                    // include/mysql/plugin.h)
        &plugin_descriptor_function_memprof_report, // pointer to type-specific
                                                    // plugin descriptor
        "tcmalloc_memprof_report",                  // plugin name
        "lefred",                                   // plugin author
        "Function TCMALLOC_MEMPROF_REPORT()",       // the plugin description
        PLUGIN_LICENSE_GPL, // the plugin license (see include/mysql/plugin.h)
        0,                  // Pointer to plugin initialization function
        0,                  // Pointer to plugin deinitialization function
        0x0100,             // Numeric version 0xAABB means AA.BB version
        NULL,               // Status variables
        NULL,               // System variables
        "1.0",              // String version representation
        MariaDB_PLUGIN_MATURITY_EXPERIMENTAL // Maturity(see
                                             // include/mysql/plugin.h)*/
    },
    {
        MariaDB_FUNCTION_PLUGIN,                  // the plugin type (see
                                                  // include/mysql/plugin.h)
        &plugin_descriptor_function_memprof_diff, // pointer to type-specific
                                                  // plugin descriptor
        "tcmalloc_memprof_diff",                  // plugin name
        "lefred",                                 // plugin author
        "Function TCMALLOC_MEMPROF_DIFF()",       // the plugin description
        PLUGIN_LICENSE_GPL, // the plugin license (see include/mysql/plugin.h)
        0,                  // Pointer to plugin initialization function
        0,                  // Pointer to plugin deinitialization function
        0x0100,             // Numeric version 0xAABB means AA.BB version
        NULL,               // Status variables
        NULL,               // System variables
        "1.0",              // String version representation
        MariaDB_PLUGIN_MATURITY_EXPERIMENTAL // Maturity(see
                                             // include/mysql/plugin.h)*/
    },
    {
        MariaDB_FUNCTION_PLUGIN,                   // the plugin type (see
                                                   // include/mysql/plugin.h)
        &plugin_descriptor_function_cpuprof_start, // pointer to type-specific
                                                   // plugin descriptor
        "tcmalloc_cpuprof_start",                  // plugin name
        "lefred",                                  // plugin author
        "Function TCMALLOC_CPUPROF_START()",       // the plugin description
        PLUGIN_LICENSE_GPL, // the plugin license (see include/mysql/plugin.h)
        0,                  // Pointer to plugin initialization function
        0,                  // Pointer to plugin deinitialization function
        0x0100,             // Numeric version 0xAABB means AA.BB version
        NULL,               // Status variables
        NULL,               // System variables
        "1.0",              // String version representation
        MariaDB_PLUGIN_MATURITY_EXPERIMENTAL // Maturity(see
                                             // include/mysql/plugin.h)*/
    },
    {
        MariaDB_FUNCTION_PLUGIN,                  // the plugin type (see
                                                  // include/mysql/plugin.h)
        &plugin_descriptor_function_cpuprof_stop, // pointer to type-specific
                                                  // plugin descriptor
        "tcmalloc_cpuprof_stop",                  // plugin name
        "lefred",                                 // plugin author
        "Function TCMALLOC_CPUPROF_STOP()",       // the plugin description
        PLUGIN_LICENSE_GPL, // the plugin license (see include/mysql/plugin.h)
        0,                  // Pointer to plugin initialization function
        0,                  // Pointer to plugin deinitialization function
        0x0100,             // Numeric version 0xAABB means AA.BB version
        NULL,               // Status variables
        NULL,               // System variables
        "1.0",              // String version representation
        MariaDB_PLUGIN_MATURITY_EXPERIMENTAL // Maturity(see
                                             // include/mysql/plugin.h)*/
    },
    {
        MariaDB_FUNCTION_PLUGIN,                   // the plugin type (see
                                                   // include/mysql/plugin.h)
        &plugin_descriptor_function_cpuprof_flush, // pointer to type-specific
                                                   // plugin descriptor
        "tcmalloc_cpuprof_flush",                  // plugin name
        "lefred",                                  // plugin author
        "Function TCMALLOC_CPUPROF_FLUSH()",       // the plugin description
        PLUGIN_LICENSE_GPL, // the plugin license (see include/mysql/plugin.h)
        0,                  // Pointer to plugin initialization function
        0,                  // Pointer to plugin deinitialization function
        0x0100,             // Numeric version 0xAABB means AA.BB version
        NULL,               // Status variables
        NULL,               // System variables
        "1.0",              // String version representation
        MariaDB_PLUGIN_MATURITY_EXPERIMENTAL // Maturity(see
                                             // include/mysql/plugin.h)*/
    },
    {
        MariaDB_FUNCTION_PLUGIN,                    // the plugin type (see
                                                    // include/mysql/plugin.h)
        &plugin_descriptor_function_cpuprof_report, // pointer to type-specific
                                                    // plugin descriptor
        "tcmalloc_cpuprof_report",                  // plugin name
        "lefred",                                   // plugin author
        "Function TCMALLOC_CPUPROF_REPORT()",       // the plugin description
        PLUGIN_LICENSE_GPL, // the plugin license (see include/mysql/plugin.h)
        0,                  // Pointer to plugin initialization function
        0,                  // Pointer to plugin deinitialization function
        0x0100,             // Numeric version 0xAABB means AA.BB version
        NULL,               // Status variables
        NULL,               // System variables
        "1.0",              // String version representation
        MariaDB_PLUGIN_MATURITY_EXPERIMENTAL // Maturity(see
                                             // include/mysql/plugin.h)*/
    },
    {
        MariaDB_FUNCTION_PLUGIN,             // the plugin type (see
                                             // include/mysql/plugin.h)
        &plugin_descriptor_function_cleanup, // pointer to type-specific plugin
                                             // descriptor
        "tcmalloc_profiler_cleanup",         // plugin name
        "lefred",                            // plugin author
        "Function TCMALLOC_PROFILER_CLEANUP()", // the plugin description
        PLUGIN_LICENSE_GPL, // the plugin license (see include/mysql/plugin.h)
        0,                  // Pointer to plugin initialization function
        0,                  // Pointer to plugin deinitialization function
        0x0100,             // Numeric version 0xAABB means AA.BB version
        NULL,               // Status variables
        NULL,               // System variables
        "0.2.0",              // String version representation
        MariaDB_PLUGIN_MATURITY_EXPERIMENTAL // Maturity(see
                                             // include/mysql/plugin.h)*/
    }

maria_declare_plugin_end;
