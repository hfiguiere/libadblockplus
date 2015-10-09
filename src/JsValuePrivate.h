/*
 * This file is part of Adblock Plus <https://adblockplus.org/>,
 * Copyright (C) 2006-2015 Eyeo GmbH
 *
 * Adblock Plus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * Adblock Plus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Adblock Plus.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ADBLOCK_PLUS_JS_VALUE_PRIVATE_H
#define ADBLOCK_PLUS_JS_VALUE_PRIVATE_H
#include <AdblockPlus/Declarations.h>

namespace AdblockPlus
{
  struct JsValuePrivate
  {
  public:
    virtual ~JsValuePrivate(){}

    virtual bool IsNull() const = 0;
    virtual bool IsString() const = 0;
    virtual bool IsObject() const = 0;
    virtual bool IsArray() const = 0;
    virtual bool IsFunction() const = 0;
    virtual bool IsUndefined() const = 0;
    virtual bool IsNumber() const = 0;
    virtual bool IsBool() const = 0;

    virtual std::string AsString() const = 0;
    virtual JsValueInt AsInt() const = 0;
    virtual bool AsBool() const = 0;
    virtual JsValueList AsList() const = 0;
    virtual JsValuePtr GetProperty(const std::string& name) const = 0;

    virtual void SetProperty(const std::string& name, const std::string& val) = 0;
    virtual void SetProperty(const std::string& name, JsValueInt val) = 0;
    virtual void SetProperty(const std::string& name, bool val) = 0;

    virtual void SetProperty(const std::string& name, const JsValuePtr& value) = 0;
    virtual std::string GetClass() const = 0;
    virtual JsValuePtr Call(const JsValueList& params, JsValuePtr thisPtr) const = 0;
    virtual std::vector<std::string> GetOwnPropertyNames() const = 0;
  };
}

#endif // ADBLOCK_PLUS_JS_VALUE_PRIVATE_H