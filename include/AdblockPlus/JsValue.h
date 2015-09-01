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

#ifndef ADBLOCK_PLUS_JS_VALUE_H
#define ADBLOCK_PLUS_JS_VALUE_H

#include <cstdint>
#include <string>
#include "Declarations.h"

namespace AdblockPlus
{
  class JsValuePrivate;
  
  /**
   * Wrapper for JavaScript values.
   * See `JsEngine` for creating `JsValue` objects.
   */
  class JsValue
  {
    friend class JsValuePrivate;
  public:
    JsValue(const JsEnginePtr& jsEngine, std::unique_ptr<JsValuePrivate>&& priv);
    virtual ~JsValue();

    bool IsUndefined() const;
    bool IsNull() const;
    bool IsString() const;
    bool IsNumber() const;
    bool IsBool() const;
    bool IsObject() const;
    bool IsArray() const;
    bool IsFunction() const;
    std::string AsString() const;
    int64_t AsInt() const;
    bool AsBool() const;
    JsValueList AsList() const;

    /**
     * Returns a list of property names if this is an object (see `IsObject()`).
     * @return List of property names.
     */
    std::vector<std::string> GetOwnPropertyNames() const;

    /**
     * Returns a property value if this is an object (see `IsObject()`).
     * @param name Property name.
     * @return Property value, undefined (see `IsUndefined()`) if the property
     *         does not exist.
     */
    JsValuePtr GetProperty(const std::string& name) const;

    //@{
    /**
     * Sets a property value if this is an object (see `IsObject()`).
     * @param name Property name.
     * @param val Property value.
     */
    void SetProperty(const std::string& name, const std::string& val);
    void SetProperty(const std::string& name, int64_t val);
    void SetProperty(const std::string& name, bool val);
    void SetProperty(const std::string& name, const JsValuePtr& value);
    inline void SetProperty(const std::string& name, const char* val)
    {
      SetProperty(name, std::string(val));
    }
    inline void SetProperty(const std::string& name, int val)
    {
      SetProperty(name, static_cast<int64_t>(val));
    }
    //@}

    /**
     * Returns the value's class name, e.g.\ _Array_ for arrays
     * (see `IsArray()`).
     * Technically, this is the name of the function that was used as a
     * constructor.
     * @return Class name of the value.
     */
    std::string GetClass() const;

    /**
     * Invokes the value as a function (see `IsFunction()`).
     * @param params Optional list of parameters.
     * @param thisPtr Optional `this` value.
     * @return Value returned by the function.
     */
    JsValuePtr Call(const JsValueList& params = JsValueList(), JsValuePtr thisPtr = JsValuePtr()) const;

    /**
     * Returns an opaque pointer to the private implementation.
     * It's required for advanced using of the value by other internal parts.
     */
    JsValuePrivate* PrivateImplementation()
    {
      return privateImpl.get();
    }
  protected:
    JsValue(const JsValuePtr& value);
    JsEnginePtr jsEngine;
  private:
    std::unique_ptr<JsValuePrivate> privateImpl;
  };
}

#endif
