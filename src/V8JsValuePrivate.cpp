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

JsValuePrivate::JsValuePrivate(const JsEnginePtr& jsEngine, v8::Handle<v8::Value> value)
  : jsEngine(jsEngine), value(jsEngine->PrivateImplementation()->isolate, value)
{
}

JsValuePrivate::~JsValuePrivate()
{
}

bool JsValuePrivate::IsUndefined() const
{
  const JsContext context(jsEngine);
  return UnwrapValue()->IsUndefined();
}

bool JsValuePrivate::IsNull() const
{
  const JsContext context(jsEngine);
  return UnwrapValue()->IsNull();
}

bool JsValuePrivate::IsString() const
{
  const JsContext context(jsEngine);
  v8::Local<v8::Value> value = UnwrapValue();
  return value->IsString() || value->IsStringObject();
}

bool JsValuePrivate::IsNumber() const
{
  const JsContext context(jsEngine);
  v8::Local<v8::Value> value = UnwrapValue();
  return value->IsNumber() || value->IsNumberObject();
}

bool JsValuePrivate::IsBool() const
{
  const JsContext context(jsEngine);
  v8::Local<v8::Value> value = UnwrapValue();
  return value->IsBoolean() || value->IsBooleanObject();
}

bool JsValuePrivate::IsObject() const
{
  const JsContext context(jsEngine);
  return UnwrapValue()->IsObject();
}

bool JsValuePrivate::IsArray() const
{
  const JsContext context(jsEngine);
  return UnwrapValue()->IsArray();
}

bool JsValuePrivate::IsFunction() const
{
  const JsContext context(jsEngine);
  return UnwrapValue()->IsFunction();
}

std::string JsValuePrivate::AsString() const
{
  const JsContext context(jsEngine);
  return Utils::FromV8String(UnwrapValue());
}

int64_t JsValuePrivate::AsInt() const
{
  const JsContext context(jsEngine);
  return UnwrapValue()->IntegerValue();
}

bool JsValuePrivate::AsBool() const
{
  const JsContext context(jsEngine);
  return UnwrapValue()->BooleanValue();
}

JsValueList JsValuePrivate::AsList() const
{
  if (!IsArray())
    throw std::runtime_error("Cannot convert a non-array to list");

  const JsContext context(jsEngine);
  JsValueList result;
  v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(UnwrapValue());
  uint32_t length = array->Length();
  for (uint32_t i = 0; i < length; i++)
  {
    result.push_back(jsEngine->PrivateImplementation()->CreateJsValuePtr(array->Get(i)));
  }
  return result;
}

std::vector<std::string> JsValuePrivate::GetOwnPropertyNames() const
{
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
}


JsValuePtr JsValuePrivate::GetProperty(const std::string& name) const
{
  if (!IsObject())
    throw new std::runtime_error("Attempting to get property of a non-object");

  const JsContext context(jsEngine);
  v8::Local<v8::String> property = Utils::ToV8String(jsEngine->PrivateImplementation()->isolate, name);
  v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(UnwrapValue());
  return jsEngine->PrivateImplementation()->CreateJsValuePtr(obj->Get(property));
}

void JsValuePrivate::SetProperty(const std::string& name, v8::Handle<v8::Value> val)
{
  if (!IsObject())
    throw new std::runtime_error("Attempting to set property on a non-object");

  v8::Local<v8::String> property = Utils::ToV8String(jsEngine->PrivateImplementation()->isolate, name);
  v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(UnwrapValue());
  obj->Set(property, val);
}

v8::Local<v8::Value> JsValuePrivate::UnwrapValue() const
{
  return v8::Local<v8::Value>::New(jsEngine->PrivateImplementation()->isolate, value);
}

void JsValuePrivate::SetProperty(const std::string& name, const std::string& val)
{
  const JsContext context(jsEngine);
  SetProperty(name, Utils::ToV8String(jsEngine->PrivateImplementation()->isolate, val));
}

void JsValuePrivate::SetProperty(const std::string& name, int64_t val)
{
  const JsContext context(jsEngine);
  SetProperty(name, v8::Number::New(jsEngine->PrivateImplementation()->isolate, val));
}

void JsValuePrivate::SetProperty(const std::string& name, const JsValuePtr& val)
{
  const JsContext context(jsEngine);
  SetProperty(name, val->PrivateImplementation()->UnwrapValue());
}

void JsValuePrivate::SetProperty(const std::string& name, bool val)
{
  const JsContext context(jsEngine);
  SetProperty(name, v8::Boolean::New(val));
}

std::string JsValuePrivate::GetClass() const
{
  if (!IsObject())
    throw new std::runtime_error("Cannot get constructor of a non-object");

  const JsContext context(jsEngine);
  v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(UnwrapValue());
  return Utils::FromV8String(obj->GetConstructorName());
}

JsValuePtr JsValuePrivate::Call(const JsValueList& params, JsValuePtr thisPtr) const
{
  if (!IsFunction())
    throw new std::runtime_error("Attempting to call a non-function");

  const JsContext context(jsEngine);
  if (!thisPtr)
  {
    JsEnginePrivate* jsEnginePriv = jsEngine->PrivateImplementation();
    v8::Local<v8::Context> localContext = v8::Local<v8::Context>::New(jsEnginePriv->isolate, jsEnginePriv->context);
    thisPtr = jsEnginePriv->CreateJsValuePtr(localContext->Global());
  }
  if (!thisPtr->IsObject())
    throw new std::runtime_error("`this` pointer has to be an object");
  v8::Local<v8::Object> thisObj = v8::Local<v8::Object>::Cast(thisPtr->PrivateImplementation()->UnwrapValue());

  std::vector<v8::Handle<v8::Value>> argv;
  for (JsValueList::const_iterator it = params.begin(); it != params.end(); ++it)
    argv.push_back((*it)->PrivateImplementation()->UnwrapValue());

  const v8::TryCatch tryCatch;
  v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(UnwrapValue());
  v8::Local<v8::Value> result = func->Call(thisObj, argv.size(),
      argv.size() ? &argv.front() : 0);

  if (tryCatch.HasCaught())
    throw JsError(tryCatch.Exception(), tryCatch.Message());

  return jsEngine->PrivateImplementation()->CreateJsValuePtr(result);
}
