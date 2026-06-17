#ifndef CPUPROF_INCLUDED
#define CPUPROF_INCLUDED

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

#include "common.h"

class Item_func_cpuprof_start : public Item_str_func
{
public:
  explicit Item_func_cpuprof_start(THD *thd) : Item_str_func(thd) {}

  Item_func_cpuprof_start(THD *thd, List<Item> &list)
      : Item_str_func(thd, list)
  {
  }

  LEX_CSTRING func_name_cstring() const override
  {
    static LEX_CSTRING name= {C_STRING_WITH_LEN("cpuprof_start")};
    return name;
  }

  bool fix_fields(THD *thd, Item **ref) override
  {
    if (Item_str_func::fix_fields(thd, ref))
      return true;
    if (check_global_access(thd, SUPER_ACL))
      return true;
    return false;
  }

  bool fix_length_and_dec(THD * /*thd*/) override
  {
    collation.set(system_charset_info);
    fix_char_length(32);
    return false;
  }

  String *val_str(String *str) override;

  Item *shallow_copy(THD *thd) const override
  {
    return get_item_copy<Item_func_cpuprof_start>(thd, this);
  }
};

class Item_func_cpuprof_stop : public Item_str_func
{
public:
  explicit Item_func_cpuprof_stop(THD *thd) : Item_str_func(thd) {}

  LEX_CSTRING func_name_cstring() const override
  {
    static LEX_CSTRING name= {STRING_WITH_LEN("cpuprof_stop")};
    return name;
  }

  bool fix_fields(THD *thd, Item **ref) override
  {
    if (Item_str_func::fix_fields(thd, ref))
      return true;
    if (check_global_access(thd, SUPER_ACL))
      return true;
    return false;
  }

  bool fix_length_and_dec(THD * /*thd*/) override
  {
    collation.set(system_charset_info);
    fix_char_length(32);
    return false;
  }

  String *val_str(String *str) override;

  Item *shallow_copy(THD *thd) const override
  {
    return get_item_copy<Item_func_cpuprof_stop>(thd, this);
  }
};

class Item_func_cpuprof_flush : public Item_str_func
{
public:
  explicit Item_func_cpuprof_flush(THD *thd) : Item_str_func(thd) {}

  LEX_CSTRING func_name_cstring() const override
  {
    static LEX_CSTRING name= {STRING_WITH_LEN("cpuprof_flush")};
    return name;
  }

  bool fix_fields(THD *thd, Item **ref) override
  {
    if (Item_str_func::fix_fields(thd, ref))
      return true;
    if (check_global_access(thd, SUPER_ACL))
      return true;
    return false;
  }

  bool fix_length_and_dec(THD * /*thd*/) override
  {
    collation.set(system_charset_info);
    fix_char_length(32);
    return false;
  }

  String *val_str(String *str) override;

  Item *shallow_copy(THD *thd) const override
  {
    return get_item_copy<Item_func_cpuprof_flush>(thd, this);
  }
};

class Item_func_cpuprof_report : public Item_str_func
{
public:
  explicit Item_func_cpuprof_report(THD *thd) : Item_str_func(thd) {}

  Item_func_cpuprof_report(THD *thd, List<Item> &list)
      : Item_str_func(thd, list)
  {
  }

  LEX_CSTRING func_name_cstring() const override
  {
    static LEX_CSTRING name= {C_STRING_WITH_LEN("cpuprof_report")};
    return name;
  }

  bool fix_fields(THD *thd, Item **ref) override
  {
    if (Item_str_func::fix_fields(thd, ref))
      return true;
    if (check_global_access(thd, SUPER_ACL))
      return true;
    return false;
  }

  bool fix_length_and_dec(THD * /*thd*/) override
  {
    collation.set(system_charset_info);
    fix_char_length(32);
    return false;
  }

  String *val_str(String *str) override;

  Item *shallow_copy(THD *thd) const override
  {
    return get_item_copy<Item_func_cpuprof_report>(thd, this);
  }
};

#endif
