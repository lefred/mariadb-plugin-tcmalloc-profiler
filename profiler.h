#ifndef PROFILER_INCLUDED
#define PROFILER_INCLUDED

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

class Item_func_profiler_info : public Item_str_func
{
public:
  Item_func_profiler_info(THD *thd) : Item_str_func(thd) {}

  LEX_CSTRING func_name_cstring() const override
  {
    static LEX_CSTRING name= {STRING_WITH_LEN("profiler_info")};
    return name;
  }

  String *val_str(String *str) override;
  bool fix_length_and_dec(THD *thd) override
  {
    if (check_global_access(thd, SUPER_ACL))
    {
      // check_global_access() has already pushed a proper error
      return true; // signal error to the resolver
    }
    collation.set(DTCollation_numeric());
    fix_char_length(32);
    return FALSE;
  }

  Item *shallow_copy(THD *thd) const override
  {
    return get_item_copy<Item_func_profiler_info>(thd, this);
  }
};

class Item_func_cleanup : public Item_str_func
{
public:
  Item_func_cleanup(THD *thd) : Item_str_func(thd) {}

  LEX_CSTRING func_name_cstring() const override
  {
    static LEX_CSTRING name= {STRING_WITH_LEN("profiler_cleanup")};
    return name;
  }

  String *val_str(String *str) override;
  bool fix_length_and_dec(THD *thd) override
  {
    if (check_global_access(thd, SUPER_ACL))
    {
      // check_global_access() has already pushed a proper error
      return true; // signal error to the resolver
    }
    collation.set(DTCollation_numeric());
    fix_char_length(32);
    return FALSE;
  }

  Item *shallow_copy(THD *thd) const override
  {
    return get_item_copy<Item_func_cleanup>(thd, this);
  }
};

#endif
