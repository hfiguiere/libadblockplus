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
#include <AdblockPlus/JsEngine.h>
#include <AdblockPlus/DefaultWebRequest.h>
#include <AdblockPlus/DefaultFileSystem.h>
#include <AdblockPlus/DefaultLogSystem.h>
#include "JsEnginePrivate.h"

using namespace AdblockPlus;

JsEngine::JsEngine()
  : privateImpl(NewJsEnginePrivate())
{
}

JsEngine::~JsEngine()
{
}

JsEnginePtr JsEngine::New(const AppInfo& appInfo)
{
  JsEnginePtr result(new JsEngine());
  result->PrivateImplementation()->Init(result, appInfo);
  return result;
}

JsValuePtr JsEngine::Evaluate(const std::string& source, const std::string& filename)
{
  return privateImpl->Evaluate(source, filename);
}

void JsEngine::Gc()
{
  privateImpl->Gc();
}

void JsEngine::SetEventCallback(const std::string& eventName, const EventCallback& callback)
{
  eventCallbacks[eventName] = callback;
}

void JsEngine::RemoveEventCallback(const std::string& eventName)
{
  eventCallbacks.erase(eventName);
}

void JsEngine::TriggerEvent(const std::string& eventName, const JsValueList& params)
{
  EventMap::iterator it = eventCallbacks.find(eventName);
  if (it != eventCallbacks.end())
    it->second(params);
}

JsValuePtr JsEngine::NewValue(const std::string& val)
{
  return privateImpl->NewValue(val);
}

JsValuePtr JsEngine::NewValue(int64_t val)
{
  return privateImpl->NewValue(val);
}

JsValuePtr JsEngine::NewValue(bool val)
{
  return privateImpl->NewValue(val);
}

JsValuePtr JsEngine::NewObject()
{
  return privateImpl->NewObject();
}

FileSystemPtr JsEngine::GetFileSystem()
{
  if (!fileSystem)
    fileSystem.reset(new DefaultFileSystem());
  return fileSystem;
}

void JsEngine::SetFileSystem(FileSystemPtr val)
{
  if (!val)
    throw std::runtime_error("FileSystem cannot be null");

  fileSystem = val;
}

WebRequestPtr JsEngine::GetWebRequest()
{
  if (!webRequest)
    webRequest.reset(new DefaultWebRequest());
  return webRequest;
}

void JsEngine::SetWebRequest(WebRequestPtr val)
{
  if (!val)
    throw std::runtime_error("WebRequest cannot be null");

  webRequest = val;
}

LogSystemPtr JsEngine::GetLogSystem()
{
  if (!logSystem)
    logSystem.reset(new DefaultLogSystem());
  return logSystem;
}

void JsEngine::SetLogSystem(LogSystemPtr val)
{
  if (!val)
    throw std::runtime_error("LogSystem cannot be null");

  logSystem = val;
}

void JsEngine::SetGlobalProperty(const std::string& name, const JsValuePtr& value)
{
  privateImpl->SetGlobalProperty(name, value);
}
