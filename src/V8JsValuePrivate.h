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

#ifndef ADBLOCK_PLUS_V8_JS_VALUE_PRIVATE_H
#define ADBLOCK_PLUS_V8_JS_VALUE_PRIVATE_H

#include <string>
#include <vector>
#include <v8.h>
#include <AdblockPlus/V8ValueHolder.h>

namespace AdblockPlus
{
  class JsValuePrivate
  {
  public:
    JsValuePrivate(const JsEnginePtr& jsEngine, v8::Handle<v8::Value> value);
    //JsValuePrivate(const JsValuePrivate& src);
    ~JsValuePrivate();

    bool IsNull() const;
    bool IsString() const;
    bool IsObject() const;
    bool IsArray() const;
    bool IsFunction() const;
    bool IsUndefined() const;
    bool IsNumber() const;
    bool IsBool() const;

    std::string AsString() const;
    int64_t AsInt() const;
    bool AsBool() const;
    JsValueList AsList() const;
    JsValuePtr GetProperty(const std::string& name) const;

    void SetProperty(const std::string& name, const std::string& val);
    void SetProperty(const std::string& name, int64_t val);
    void SetProperty(const std::string& name, bool val);

    void SetProperty(const std::string& name, const JsValuePtr& value);
    std::string GetClass() const;
    JsValuePtr Call(const JsValueList& params, JsValuePtr thisPtr) const;
    std::vector<std::string> GetOwnPropertyNames() const;
  private:
    v8::Local<v8::Value> UnwrapValue() const;
    void SetProperty(const std::string& name, v8::Handle<v8::Value> val);
    JsEnginePtr jsEngine;
    V8ValueHolder<v8::Value> value;
  };
}
#endif // ADBLOCK_PLUS_V8_JS_VALUE_PRIVATE_H
