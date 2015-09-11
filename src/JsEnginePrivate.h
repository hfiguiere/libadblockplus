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

#ifndef ADBLOCK_PLUS_JS_ENGINE_PRIVATE_H
#define ADBLOCK_PLUS_JS_ENGINE_PRIVATE_H
#include <AdblockPlus/Declarations.h>
#include <string>
#include <cstdint>

namespace AdblockPlus
{
  struct JsEnginePrivate
  {
  public:
    virtual ~JsEnginePrivate(){}
    virtual void Gc() = 0;
    virtual void Init(const JsEnginePtr& jsEngine, const AppInfo& appInfo) = 0;

    virtual JsValuePtr Evaluate(const std::string& source, const std::string& filename = "") = 0;
    virtual JsValuePtr NewObject() = 0;
    virtual JsValuePtr NewValue(const std::string& val) = 0;
    virtual JsValuePtr NewValue(int64_t val) = 0;
    virtual JsValuePtr NewValue(bool val) = 0;
    virtual void SetGlobalProperty(const std::string& name, const JsValuePtr& value) = 0;
  };
}

#endif // ADBLOCK_PLUS_JS_ENGINE_PRIVATE_H