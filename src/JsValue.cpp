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
#include <AdblockPlus/JsValue.h>
#include "JsValuePrivate.h"

using namespace AdblockPlus;

JsValue::JsValue(const JsEnginePtr& jsEngine, std::unique_ptr<JsValuePrivate>&& priv)
  : jsEngine(jsEngine), privateImpl(move(priv))
{
}

JsValue::~JsValue()
{
}

bool JsValue::IsUndefined() const
{
  return privateImpl->IsUndefined();
}

bool JsValue::IsNull() const
{
  return privateImpl->IsNull();
}

bool JsValue::IsString() const
{
  return privateImpl->IsString();
}

bool JsValue::IsNumber() const
{
  return privateImpl->IsNumber();
}

bool JsValue::IsBool() const
{
  return privateImpl->IsBool();
}

bool JsValue::IsObject() const
{
  return privateImpl->IsObject();
}

bool JsValue::IsArray() const
{
  return privateImpl->IsArray();
}

bool JsValue::IsFunction() const
{
  return privateImpl->IsFunction();
}

std::string JsValue::AsString() const
{
  return privateImpl->AsString();
}

int64_t JsValue::AsInt() const
{
  return privateImpl->AsInt();
}

bool JsValue::AsBool() const
{
  return privateImpl->AsBool();
}

JsValueList JsValue::AsList() const
{
  return privateImpl->AsList();
}

std::vector<std::string> JsValue::GetOwnPropertyNames() const
{
  return privateImpl->GetOwnPropertyNames();
}

JsValuePtr JsValue::GetProperty(const std::string& name) const
{
  return privateImpl->GetProperty(name);
}

void JsValue::SetProperty(const std::string& name, const std::string& value)
{
  privateImpl->SetProperty(name, value);
}

void JsValue::SetProperty(const std::string& name, const int64_t value)
{
  privateImpl->SetProperty(name, value);
}

void JsValue::SetProperty(const std::string& name, const bool value)
{
  privateImpl->SetProperty(name, value);
}

void JsValue::SetProperty(const std::string& name, const JsValuePtr& value)
{
  privateImpl->SetProperty(name, value);
}

std::string JsValue::GetClass() const
{
  return privateImpl->GetClass();
}

JsValuePtr JsValue::Call(const JsValueList& params, JsValuePtr thisPtr) const
{
  return privateImpl->Call(params, thisPtr);
}

JsValue::JsValue(const JsValuePtr& value)
  : jsEngine(value->jsEngine), privateImpl(CloneJsValuePrivate(*value->privateImpl))
{
}
