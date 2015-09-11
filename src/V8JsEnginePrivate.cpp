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

#include "GlobalJsObject.h"
#include "JsError.h"
#include "Utils.h"
#include "V8JsEnginePrivate.h"
#include "V8JsValuePrivate.h"

namespace
{
  v8::Handle<v8::Script> CompileScript(v8::Isolate* isolate,
    const std::string& source, const std::string& filename)
  {
    using AdblockPlus::Utils::ToV8String;
    const v8::Handle<v8::String> v8Source = ToV8String(isolate, source);
    if (filename.length())
    {
      const v8::Handle<v8::String> v8Filename = ToV8String(isolate, filename);
      return v8::Script::Compile(v8Source, v8Filename);
    }
    else
      return v8::Script::Compile(v8Source);
  }

  void CheckTryCatch(const v8::TryCatch& tryCatch)
  {
    if (tryCatch.HasCaught())
      throw AdblockPlus::JsError(tryCatch.Exception(), tryCatch.Message());
  }

  class V8Initializer
  {
    V8Initializer()
    {
      v8::V8::Initialize();
    }

    ~V8Initializer()
    {
      v8::V8::Dispose();
    }
  public:
    static void Init()
    {
      // it's threadsafe since C++11 and it will be instantiated only once and
      // destroyed at the application exit
      static V8Initializer initializer;
    }
  };
}

using namespace AdblockPlus;

V8JsEnginePrivateImpl::V8JsEnginePrivateImpl()
  : isolate(v8::Isolate::GetCurrent())
{
}

JsValuePtr V8JsEnginePrivateImpl::CreateJsValuePtr(v8::Local<v8::Value> v8value)
{
  std::unique_ptr<JsValuePrivate> jsValuePriv(new V8JsValuePrivateImpl(parent.lock(), v8value));
  return JsValuePtr(new JsValue(parent.lock(), move(jsValuePriv)));
}

std::unique_ptr<JsEnginePrivate> AdblockPlus::NewJsEnginePrivate()
{
  V8Initializer::Init();
  std::unique_ptr<JsEnginePrivate> result(new V8JsEnginePrivateImpl());
  return result;
}

void V8JsEnginePrivateImpl::Gc()
{
  while (!v8::V8::IdleNotification());
}

void V8JsEnginePrivateImpl::Init(const JsEnginePtr& jsEngine, const AppInfo& appInfo)
{
  parent = jsEngine;
  const v8::Locker locker(isolate);
  const v8::HandleScope handleScope;
  context.reset(isolate, v8::Context::New(isolate));
  v8::Local<v8::Object> globalContext = v8::Local<v8::Context>::New(
    isolate, context)->Global();
  globalJsObject = CreateJsValuePtr(globalContext);
  GlobalJsObject::Setup(parent.lock(), appInfo, globalJsObject);
}

JsValuePtr V8JsEnginePrivateImpl::Evaluate(const std::string& source,
    const std::string& filename)
{
  const JsContext context(parent.lock());
  const v8::TryCatch tryCatch;
  const v8::Handle<v8::Script> script = CompileScript(isolate, source,
    filename);
  CheckTryCatch(tryCatch);
  v8::Local<v8::Value> result = script->Run();
  CheckTryCatch(tryCatch);
  return CreateJsValuePtr(result);
}

JsValuePtr V8JsEnginePrivateImpl::NewValue(const std::string& val)
{
  const JsContext context(parent.lock());
  return CreateJsValuePtr(Utils::ToV8String(isolate, val));
}

JsValuePtr V8JsEnginePrivateImpl::NewValue(int64_t val)
{
  const JsContext context(parent.lock());
  return CreateJsValuePtr(v8::Number::New(isolate, val));
}

JsValuePtr V8JsEnginePrivateImpl::NewValue(bool val)
{
  const JsContext context(parent.lock());
  return CreateJsValuePtr(v8::Boolean::New(val));
}

JsValuePtr V8JsEnginePrivateImpl::NewObject()
{
  const JsContext context(parent.lock());
  return CreateJsValuePtr(v8::Object::New());
}

JsValuePtr V8JsEnginePrivateImpl::NewCallback(v8::InvocationCallback callback)
{
  const JsContext context(parent.lock());

  // Note: we are leaking this weak pointer, no obvious way to destroy it when
  // it's no longer used
  std::weak_ptr<JsEngine>* data =
    new std::weak_ptr<JsEngine>(parent.lock());
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(callback,
      v8::External::New(data));
  return CreateJsValuePtr(templ->GetFunction());
}

JsEnginePtr V8JsEnginePrivateImpl::FromArguments(const v8::Arguments& arguments)
{
  const v8::Local<const v8::External> external =
      v8::Local<const v8::External>::Cast(arguments.Data());
  std::weak_ptr<JsEngine>* data =
      static_cast<std::weak_ptr<JsEngine>*>(external->Value());
  JsEnginePtr result = data->lock();
  if (!result)
    throw std::runtime_error("Oops, our JsEngine is gone, how did that happen?");
  return result;
}

JsValueList V8JsEnginePrivateImpl::ConvertArguments(const v8::Arguments& arguments)
{
  const JsContext context(parent.lock());
  JsValueList list;
  for (int i = 0; i < arguments.Length(); i++)
    list.push_back(CreateJsValuePtr(arguments[i]));
  return list;
}

void V8JsEnginePrivateImpl::SetGlobalProperty(const std::string& name,  const JsValuePtr& value)
{
  if (!globalJsObject)
    throw std::runtime_error("Global object cannot be null");

  globalJsObject->SetProperty(name, value);
}

V8JsEnginePrivateImpl* AdblockPlus::GetPrivateImpl(JsEngine& jsEngine)
{
  return static_cast<V8JsEnginePrivateImpl*>(jsEngine.PrivateImplementation());
}
