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

#ifndef ADBLOCK_PLUS_JSC_JS_VALUE_PRIVATE_H
#define ADBLOCK_PLUS_JSC_JS_VALUE_PRIVATE_H
#import <AdblockPlus/Declarations.h>
#import "JsValuePrivate.h"
#import <JavaScriptCore/JavaScriptCore.h>

namespace AdblockPlus
{
  class JSCJsValuePrivateImpl : public JsValuePrivate
  {
    friend class JSCJsEnginePrivateImpl;
  public:
    JSCJsValuePrivateImpl(const JsEnginePtr& jsEngine, JSValue* jsValue);
    JSCJsValuePrivateImpl(const JSCJsValuePrivateImpl& src);
    ~JSCJsValuePrivateImpl();
    
    bool IsNull() const;
    bool IsString() const;
    bool IsObject() const;
    bool IsArray() const;
    bool IsFunction() const;
    bool IsUndefined() const;
    bool IsNumber() const;
    bool IsBool() const;
    
    std::string AsString() const;
    JsValueInt AsInt() const;
    bool AsBool() const;
    JsValueList AsList() const;
    JsValuePtr GetProperty(const std::string& name) const;
    
    void SetProperty(const std::string& name, const std::string& val);
    void SetProperty(const std::string& name, JsValueInt val);
    void SetProperty(const std::string& name, bool val);
    
    void SetProperty(const std::string& name, const JsValuePtr& value);
    std::string GetClass() const;
    JsValuePtr Call(const JsValueList& params, JsValuePtr thisPtr) const;
    std::vector<std::string> GetOwnPropertyNames() const;
  private:
    JsEnginePtr jsEngine;
    // Pay attention that JSValue is not our type, it comes from JavaScriptCore
    // and has a diffirent case of the second letter ('S').
    JSValue* jsValue;
  };
  JSCJsValuePrivateImpl* GetPrivateImpl(JsValue& jsValue);
  inline JSCJsValuePrivateImpl* GetPrivateImpl(const JsValuePtr& jsValue)
  {
    return GetPrivateImpl(*jsValue);
  }
}


#endif /* ADBLOCK_PLUS_JSC_JS_VALUE_PRIVATE_H */
