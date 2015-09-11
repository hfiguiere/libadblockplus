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

#include <vector>
#include <AdblockPlus/JsValue.h>
#include <AdblockPlus/JsEngine.h>
#include "JsContext.h"
#include "JsError.h"
#include "Utils.h"
#include "V8JsValuePrivate.h"
#include "V8JsEnginePrivate.h"

using namespace AdblockPlus;

V8JsValuePrivateImpl::V8JsValuePrivateImpl(const JsEnginePtr& jsEngine, v8::Handle<v8::Value> value)
  : jsEngine(jsEngine), value(GetPrivateImpl(jsEngine)->isolate, value)
{
}

V8ValueHolder<v8::Value> V8JsValuePrivateImpl::Clone(const JsEnginePtr& jsEngine, const V8ValueHolder<v8::Value>& value)
{
  JsContext jsContext(jsEngine);
  return V8ValueHolder<v8::Value>(GetPrivateImpl(jsEngine)->isolate, v8::Local<v8::Value>::New(GetPrivateImpl(jsEngine)->isolate, value));
}

V8JsValuePrivateImpl::V8JsValuePrivateImpl(const V8JsValuePrivateImpl& src)
  : jsEngine(src.jsEngine), value(Clone(jsEngine, src.value))
{
}

V8JsValuePrivateImpl::~V8JsValuePrivateImpl()
{
}

bool V8JsValuePrivateImpl::IsUndefined() const
{
  const JsContext context(jsEngine);
  return UnwrapValue()->IsUndefined();
}

bool V8JsValuePrivateImpl::IsNull() const
{
  const JsContext context(jsEngine);
  return UnwrapValue()->IsNull();
}

bool V8JsValuePrivateImpl::IsString() const
{
  const JsContext context(jsEngine);
  v8::Local<v8::Value> value = UnwrapValue();
  return value->IsString() || value->IsStringObject();
}

bool V8JsValuePrivateImpl::IsNumber() const
{
  const JsContext context(jsEngine);
  v8::Local<v8::Value> value = UnwrapValue();
  return value->IsNumber() || value->IsNumberObject();
}

bool V8JsValuePrivateImpl::IsBool() const
{
  const JsContext context(jsEngine);
  v8::Local<v8::Value> value = UnwrapValue();
  return value->IsBoolean() || value->IsBooleanObject();
}

bool V8JsValuePrivateImpl::IsObject() const
{
  const JsContext context(jsEngine);
  return UnwrapValue()->IsObject();
}

bool V8JsValuePrivateImpl::IsArray() const
{
  const JsContext context(jsEngine);
  return UnwrapValue()->IsArray();
}

bool V8JsValuePrivateImpl::IsFunction() const
{
  const JsContext context(jsEngine);
  return UnwrapValue()->IsFunction();
}

std::string V8JsValuePrivateImpl::AsString() const
{
  const JsContext context(jsEngine);
  return Utils::FromV8String(UnwrapValue());
}

int64_t V8JsValuePrivateImpl::AsInt() const
{
  const JsContext context(jsEngine);
  return UnwrapValue()->IntegerValue();
}

bool V8JsValuePrivateImpl::AsBool() const
{
  const JsContext context(jsEngine);
  return UnwrapValue()->BooleanValue();
}

JsValueList V8JsValuePrivateImpl::AsList() const
{
  if (!IsArray())
    throw std::runtime_error("Cannot convert a non-array to list");

  const JsContext context(jsEngine);
  JsValueList result;
  v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(UnwrapValue());
  uint32_t length = array->Length();
  for (uint32_t i = 0; i < length; i++)
  {
    result.push_back(GetPrivateImpl(jsEngine)->CreateJsValuePtr(array->Get(i)));
  }
  return result;
}

std::vector<std::string> V8JsValuePrivateImpl::GetOwnPropertyNames() const
{
  if (!IsObject())
    throw new std::runtime_error("Attempting to get propert list for a non-object");

  const JsContext context(jsEngine);
  v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(UnwrapValue());
  JsValuePtr jsObject = GetPrivateImpl(jsEngine)->CreateJsValuePtr(object->GetOwnPropertyNames());
  JsValueList properties = jsObject->AsList();
  std::vector<std::string> result;
  for (JsValueList::iterator it = properties.begin(); it != properties.end(); ++it)
    result.push_back((*it)->AsString());
  return result;
}


JsValuePtr V8JsValuePrivateImpl::GetProperty(const std::string& name) const
{
  if (!IsObject())
    throw new std::runtime_error("Attempting to get property of a non-object");

  const JsContext context(jsEngine);
  v8::Local<v8::String> property = Utils::ToV8String(GetPrivateImpl(jsEngine)->isolate, name);
  v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(UnwrapValue());
  return GetPrivateImpl(jsEngine)->CreateJsValuePtr(obj->Get(property));
}

void V8JsValuePrivateImpl::SetProperty(const std::string& name, v8::Handle<v8::Value> val)
{
  if (!IsObject())
    throw new std::runtime_error("Attempting to set property on a non-object");

  v8::Local<v8::String> property = Utils::ToV8String(GetPrivateImpl(jsEngine)->isolate, name);
  v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(UnwrapValue());
  obj->Set(property, val);
}

v8::Local<v8::Value> V8JsValuePrivateImpl::UnwrapValue() const
{
  return v8::Local<v8::Value>::New(GetPrivateImpl(jsEngine)->isolate, value);
}

void V8JsValuePrivateImpl::SetProperty(const std::string& name, const std::string& val)
{
  const JsContext context(jsEngine);
  SetProperty(name, Utils::ToV8String(GetPrivateImpl(jsEngine)->isolate, val));
}

void V8JsValuePrivateImpl::SetProperty(const std::string& name, int64_t val)
{
  const JsContext context(jsEngine);
  SetProperty(name, v8::Number::New(GetPrivateImpl(jsEngine)->isolate, val));
}

void V8JsValuePrivateImpl::SetProperty(const std::string& name, const JsValuePtr& val)
{
  const JsContext context(jsEngine);
  SetProperty(name, GetPrivateImpl(val)->UnwrapValue());
}

void V8JsValuePrivateImpl::SetProperty(const std::string& name, bool val)
{
  const JsContext context(jsEngine);
  SetProperty(name, v8::Boolean::New(val));
}

std::string V8JsValuePrivateImpl::GetClass() const
{
  if (!IsObject())
    throw new std::runtime_error("Cannot get constructor of a non-object");

  const JsContext context(jsEngine);
  v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(UnwrapValue());
  return Utils::FromV8String(obj->GetConstructorName());
}

JsValuePtr V8JsValuePrivateImpl::Call(const JsValueList& params, JsValuePtr thisPtr) const
{
  if (!IsFunction())
    throw new std::runtime_error("Attempting to call a non-function");

  const JsContext context(jsEngine);
  if (!thisPtr)
  {
    V8JsEnginePrivateImpl* jsEnginePriv = GetPrivateImpl(jsEngine);
    v8::Local<v8::Context> localContext = v8::Local<v8::Context>::New(jsEnginePriv->isolate, jsEnginePriv->context);
    thisPtr = jsEnginePriv->CreateJsValuePtr(localContext->Global());
  }
  if (!thisPtr->IsObject())
    throw new std::runtime_error("`this` pointer has to be an object");
  v8::Local<v8::Object> thisObj = v8::Local<v8::Object>::Cast(GetPrivateImpl(thisPtr)->UnwrapValue());

  std::vector<v8::Handle<v8::Value>> argv;
  for (JsValueList::const_iterator it = params.begin(); it != params.end(); ++it)
    argv.push_back(GetPrivateImpl(*it)->UnwrapValue());

  const v8::TryCatch tryCatch;
  v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(UnwrapValue());
  v8::Local<v8::Value> result = func->Call(thisObj, argv.size(),
      argv.size() ? &argv.front() : 0);

  if (tryCatch.HasCaught())
    throw JsError(tryCatch.Exception(), tryCatch.Message());

  return GetPrivateImpl(jsEngine)->CreateJsValuePtr(result);
}

JsValuePrivatePtr AdblockPlus::CloneJsValuePrivate(const JsValuePrivate& jsValuePriv)
{
  return JsValuePrivatePtr(new V8JsValuePrivateImpl(static_cast<const V8JsValuePrivateImpl&>(jsValuePriv)));
}

V8JsValuePrivateImpl* AdblockPlus::GetPrivateImpl(JsValue& jsValue)
{
  return static_cast<V8JsValuePrivateImpl*>(jsValue.PrivateImplementation());
}
