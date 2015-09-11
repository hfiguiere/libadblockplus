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

#include "JSCJsValuePrivate.h"
#include <AdblockPlus/JsValue.h>

using namespace AdblockPlus;


JSCJsValuePrivateImpl::JSCJsValuePrivateImpl(const JsEnginePtr& jsEngine, JSValue* jsValue)
: jsEngine(jsEngine), jsValue([jsValue retain])
{
  
}

JSCJsValuePrivateImpl::JSCJsValuePrivateImpl(const JSCJsValuePrivateImpl& src)
: jsEngine(src.jsEngine), jsValue([src.jsValue retain])
{
  
}

JSCJsValuePrivateImpl::~JSCJsValuePrivateImpl()
{
  [jsValue release];
  jsValue = nil;
}


bool JSCJsValuePrivateImpl::IsUndefined() const
{
  return [jsValue isUndefined] != NO;
}

bool JSCJsValuePrivateImpl::IsNull() const
{
  return !jsValue || [jsValue isNull] != NO;
}

bool JSCJsValuePrivateImpl::IsString() const
{
  return [jsValue isString] != NO;
}

bool JSCJsValuePrivateImpl::IsNumber() const
{
  return [jsValue isNumber] != NO;
}

bool JSCJsValuePrivateImpl::IsBool() const
{
  return [jsValue isBoolean] != NO;
}

bool JSCJsValuePrivateImpl::IsObject() const
{
  return [jsValue isObject] != NO;
}

bool JSCJsValuePrivateImpl::IsArray() const
{
  return [jsValue isArray] != NO;
}

bool JSCJsValuePrivateImpl::IsFunction() const
{
  JSContext* jsContext = [jsValue context];
  return [jsValue isInstanceOf:[jsContext evaluateScript:@"Function"]] != NO;
}

std::string JSCJsValuePrivateImpl::AsString() const
{
  return [[jsValue toString] UTF8String];
}

int64_t JSCJsValuePrivateImpl::AsInt() const
{
  return [jsValue toInt32];
}

bool JSCJsValuePrivateImpl::AsBool() const
{
  return [jsValue toBool] != NO;
}

JsValueList JSCJsValuePrivateImpl::AsList() const
{
  if (!IsArray())
    throw std::runtime_error("Cannot convert a non-array to list");
  
  JsValueList result;
  JSValue* arrayLengthJSValue = [jsValue valueForProperty:@"length"];
  if (!arrayLengthJSValue)
    return result;
  NSUInteger length = [arrayLengthJSValue toUInt32];
  for (NSUInteger i = 0; i < length; ++i)
  {
    JsValuePrivatePtr jsValuePrivate(new JSCJsValuePrivateImpl(jsEngine, [jsValue valueAtIndex:i]));
    result.push_back(JsValuePtr(new JsValue(jsEngine, move(jsValuePrivate))));
  }
  return result;
}

std::vector<std::string> JSCJsValuePrivateImpl::GetOwnPropertyNames() const
{
/*
 if (!IsObject())
    throw new std::runtime_error("Attempting to get propert list for a non-object");
    
    const JsContext context(jsEngine);
    v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(UnwrapValue());
    JsValuePtr jsObject = jsEngine->PrivateImplementation()->CreateJsValuePtr(object->GetOwnPropertyNames());
    JsValueList properties = jsObject->AsList();
    std::vector<std::string> result;
  for (JsValueList::iterator it = properties.begin(); it != properties.end(); ++it)
    result.push_back((*it)->AsString());
    return result;

 */
  throw std::runtime_error("GetOwnPropertyNames is not implemented");
}

JsValuePtr JSCJsValuePrivateImpl::GetProperty(const std::string& name) const
{
  if (!IsObject())
    throw std::runtime_error("Attempting to get property of a non-object");
  
  JSValue* jsProperty = [jsValue valueForProperty:@(name.c_str())];
  JsValuePrivatePtr jsValuePrivate(new JSCJsValuePrivateImpl(jsEngine, jsProperty));
  return JsValuePtr(new JsValue(jsEngine, move(jsValuePrivate)));
}

void JSCJsValuePrivateImpl::SetProperty(const std::string& name, const std::string& val)
{
  [jsValue setValue:@(val.c_str()) forProperty:@(name.c_str())];
}

void JSCJsValuePrivateImpl::SetProperty(const std::string& name, int64_t val)
{
  [jsValue setValue:[JSValue valueWithInt32:static_cast<int32_t>(val) inContext:[jsValue context]] forProperty:@(name.c_str())];
 }

void JSCJsValuePrivateImpl::SetProperty(const std::string& name, const JsValuePtr& val)
{
  [jsValue setValue:GetPrivateImpl(val)->jsValue forProperty:@(name.c_str())];
}

void JSCJsValuePrivateImpl::SetProperty(const std::string& name, bool val)
{
  [jsValue setValue:[JSValue valueWithBool:val inContext:[jsValue context]] forProperty:@(name.c_str())];
}

std::string JSCJsValuePrivateImpl::GetClass() const
{
  if (!IsObject())
    throw std::runtime_error("Cannot get constructor of a non-object");
  
  JSValue* jsConstructor = [jsValue valueForProperty:@"constructor"];
  if (!jsConstructor)
  {
    return "";
  }
  JSValue* jsConstructorName = [jsConstructor valueForProperty:@"name"];
  if (!jsConstructorName)
  {
    return "";
  }
  return [[jsConstructorName toString] UTF8String];
}

JsValuePtr JSCJsValuePrivateImpl::Call(const JsValueList& params, JsValuePtr thisPtr) const
{
  @autoreleasepool {
    if (!IsFunction())
      throw std::runtime_error("Attempting to call a non-function");
    /*
    if (!thisPtr)
    {
      JsEnginePrivate* jsEnginePriv = jsEngine->PrivateImplementation();
      v8::Local<v8::Context> localContext = v8::Local<v8::Context>::New(jsEnginePriv->isolate, jsEnginePriv->context);
      thisPtr = jsEnginePriv->CreateJsValuePtr(localContext->Global());
    }
    if (!thisPtr->IsObject())
      throw new std::runtime_error("`this` pointer has to be an object");
    v8::Local<v8::Object> thisObj = v8::Local<v8::Object>::Cast(thisPtr->PrivateImplementation()->UnwrapValue());
    */
    NSMutableArray* jsParams = [[NSMutableArray alloc] init];
    for (const auto& param : params)
    {
      [jsParams addObject:GetPrivateImpl(param)->jsValue];
    }
    //const v8::TryCatch tryCatch;
    JSValue* retValue = [jsValue callWithArguments:jsParams];
    JsValuePrivatePtr jsValuePrivate(new JSCJsValuePrivateImpl(jsEngine, retValue));
    [jsParams release];
    jsParams = nil;
    /*
    if (tryCatch.HasCaught())
      throw JsError(tryCatch.Exception(), tryCatch.Message());
    */
    return JsValuePtr(new JsValue(jsEngine, move(jsValuePrivate)));
  }
}

JSCJsValuePrivateImpl* AdblockPlus::GetPrivateImpl(JsValue& jsValue)
{
  return static_cast<JSCJsValuePrivateImpl*>(jsValue.PrivateImplementation());
}

JsValuePrivatePtr AdblockPlus::CloneJsValuePrivate(const JsValuePrivate& src)
{
  return JsValuePrivatePtr(new JSCJsValuePrivateImpl(static_cast<const JSCJsValuePrivateImpl&>(src)));
}