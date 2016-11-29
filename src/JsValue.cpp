/*
 * This file is part of Adblock Plus <https://adblockplus.org/>,
 * Copyright (C) 2006-2016 Eyeo GmbH
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
#include <AdblockPlus.h>

#include "JsContext.h"
#include "JsError.h"
#include "Utils.h"

AdblockPlus::JsValue::JsValue(const std::weak_ptr<AdblockPlus::JsEngine>& jsEngine,
  v8::Handle<v8::Value> value)
  : jsEngine(jsEngine)
  , value(new v8::Persistent<v8::Value>(Utils::lockJsEngine(jsEngine)->GetIsolate(), value))
{
}

AdblockPlus::JsValue::JsValue(AdblockPlus::JsValue&& src)
    : jsEngine(std::move(src.jsEngine)),
      value(std::move(src.value))
{
}

AdblockPlus::JsValue::~JsValue()
{
  try
  {
    JsContext context(jsEngine);
    if (value)
    {
      value->Dispose();
      value.reset();
    }
  }
  catch (const JsEngine::JsEngineNotAvailableException&)
  {
  }
}

bool AdblockPlus::JsValue::IsUndefined() const
{
  JsContext context(jsEngine);
  return UnwrapValue(context.GetJsEngine())->IsUndefined();
}

bool AdblockPlus::JsValue::IsNull() const
{
  JsContext context(jsEngine);
  return UnwrapValue(context.GetJsEngine())->IsNull();
}

bool AdblockPlus::JsValue::IsString() const
{
  JsContext context(jsEngine);
  v8::Local<v8::Value> value = UnwrapValue(context.GetJsEngine());
  return value->IsString() || value->IsStringObject();
}

bool AdblockPlus::JsValue::IsNumber() const
{
  JsContext context(jsEngine);
  v8::Local<v8::Value> value = UnwrapValue(context.GetJsEngine());
  return value->IsNumber() || value->IsNumberObject();
}

bool AdblockPlus::JsValue::IsBool() const
{
  JsContext context(jsEngine);
  v8::Local<v8::Value> value = UnwrapValue(context.GetJsEngine());
  return value->IsBoolean() || value->IsBooleanObject();
}

bool AdblockPlus::JsValue::IsObject() const
{
  JsContext context(jsEngine);
  return UnwrapValue(context.GetJsEngine())->IsObject();
}

bool AdblockPlus::JsValue::IsArray() const
{
  JsContext context(jsEngine);
  return UnwrapValue(context.GetJsEngine())->IsArray();
}

bool AdblockPlus::JsValue::IsFunction() const
{
  JsContext context(jsEngine);
  return UnwrapValue(context.GetJsEngine())->IsFunction();
}

std::string AdblockPlus::JsValue::AsString() const
{
  JsContext context(jsEngine);
  return Utils::FromV8String(UnwrapValue(context.GetJsEngine()));
}

int64_t AdblockPlus::JsValue::AsInt() const
{
  JsContext context(jsEngine);
  return UnwrapValue(context.GetJsEngine())->IntegerValue();
}

bool AdblockPlus::JsValue::AsBool() const
{
  JsContext context(jsEngine);
  return UnwrapValue(context.GetJsEngine())->BooleanValue();
}

AdblockPlus::JsValueList AdblockPlus::JsValue::AsList() const
{
  JsContext context(jsEngine);
  if (!IsArray())
    throw std::runtime_error("Cannot convert a non-array to list");

  JsValueList result;
  v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(UnwrapValue(context.GetJsEngine()));
  uint32_t length = array->Length();
  for (uint32_t i = 0; i < length; i++)
  {
    v8::Local<v8::Value> item = array->Get(i);
    result.push_back(JsValuePtr(new JsValue(jsEngine, item)));
  }
  return result;
}

std::vector<std::string> AdblockPlus::JsValue::GetOwnPropertyNames() const
{
  JsContext context(jsEngine);
  if (!IsObject())
    throw new std::runtime_error("Attempting to get propert list for a non-object");

  v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(UnwrapValue(context.GetJsEngine()));
  JsValueList properties = JsValuePtr(new JsValue(jsEngine, object->GetOwnPropertyNames()))->AsList();
  std::vector<std::string> result;
  for (JsValueList::iterator it = properties.begin(); it != properties.end(); ++it)
    result.push_back((*it)->AsString());
  return result;
}


AdblockPlus::JsValuePtr AdblockPlus::JsValue::GetProperty(const std::string& name) const
{
  JsContext context(jsEngine);
  if (!IsObject())
    throw new std::runtime_error("Attempting to get property of a non-object");

  v8::Local<v8::String> property = Utils::ToV8String(context.GetJsEngine().GetIsolate(), name);
  v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(UnwrapValue(context.GetJsEngine()));
  return JsValuePtr(new JsValue(jsEngine, obj->Get(property)));
}

void AdblockPlus::JsValue::SetProperty(const std::string& name, v8::Handle<v8::Value> val)
{
  JsContext context(jsEngine);
  if (!IsObject())
    throw new std::runtime_error("Attempting to set property on a non-object");

  v8::Local<v8::String> property = Utils::ToV8String(context.GetJsEngine().GetIsolate(), name);
  v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(UnwrapValue(context.GetJsEngine()));
  obj->Set(property, val);
}

v8::Local<v8::Value> AdblockPlus::JsValue::UnwrapValue(JsEngine& jsEngine) const
{
  return v8::Local<v8::Value>::New(jsEngine.GetIsolate(), *value);
}

void AdblockPlus::JsValue::SetProperty(const std::string& name, const std::string& val)
{
  JsContext context(jsEngine);
  SetProperty(name, Utils::ToV8String(context.GetJsEngine().GetIsolate(), val));
}

void AdblockPlus::JsValue::SetProperty(const std::string& name, int64_t val)
{
  JsContext context(jsEngine);
  SetProperty(name, v8::Number::New(context.GetJsEngine().GetIsolate(), val));
}

void AdblockPlus::JsValue::SetProperty(const std::string& name, const JsValuePtr& val)
{
  JsContext context(jsEngine);
  SetProperty(name, val->UnwrapValue(context.GetJsEngine()));
}

void AdblockPlus::JsValue::SetProperty(const std::string& name, bool val)
{
  JsContext context(jsEngine);
  SetProperty(name, v8::Boolean::New(val));
}

std::string AdblockPlus::JsValue::GetClass() const
{
  JsContext context(jsEngine);
  if (!IsObject())
    throw new std::runtime_error("Cannot get constructor of a non-object");

  v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(UnwrapValue(context.GetJsEngine()));
  return Utils::FromV8String(obj->GetConstructorName());
}

AdblockPlus::JsValuePtr AdblockPlus::JsValue::Call(const JsValueList& params, JsValuePtr thisPtr) const
{
  JsContext context(jsEngine);
  if (!IsFunction())
    throw new std::runtime_error("Attempting to call a non-function");

  if (!thisPtr)
  {
    v8::Local<v8::Context> localContext = v8::Local<v8::Context>::New(
      context.GetJsEngine().GetIsolate(), *context.GetJsEngine().context);
    thisPtr = JsValuePtr(new JsValue(jsEngine, localContext->Global()));
  }
  if (!thisPtr->IsObject())
    throw new std::runtime_error("`this` pointer has to be an object");
  v8::Local<v8::Object> thisObj = v8::Local<v8::Object>::Cast(thisPtr->UnwrapValue(context.GetJsEngine()));

  std::vector<v8::Handle<v8::Value>> argv;
  for (JsValueList::const_iterator it = params.begin(); it != params.end(); ++it)
    argv.push_back((*it)->UnwrapValue(context.GetJsEngine()));

  const v8::TryCatch tryCatch;
  v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(UnwrapValue(context.GetJsEngine()));
  v8::Local<v8::Value> result = func->Call(thisObj, argv.size(),
      argv.size() ? &argv.front() : 0);

  if (tryCatch.HasCaught())
    throw JsError(tryCatch.Exception(), tryCatch.Message());

  return JsValuePtr(new JsValue(jsEngine, result));
}

AdblockPlus::JsValuePtr AdblockPlus::JsValue::Call(const JsValue& arg) const
{
  JsContext context(jsEngine);
  JsValueList params;
  params.push_back(JsValuePtr(new JsValue(arg.jsEngine, arg.UnwrapValue(context.GetJsEngine()))));
  return Call(params);
}
