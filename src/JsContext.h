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

#ifndef ADBLOCK_PLUS_JS_CONTEXT_H
#define ADBLOCK_PLUS_JS_CONTEXT_H

#include <v8.h>
#include <AdblockPlus/JsEngine.h>

namespace AdblockPlus
{
  class JsContext
  {
  public:
    explicit JsContext(const std::weak_ptr<JsEngine>& jsEngine);
    JsEngine& jsEngine() {
      return *m_jsEngine;
    }
  private:
    JsEnginePtr m_jsEngine;
    const v8::Locker m_locker;
    const v8::Isolate::Scope m_isolateScope;
    const v8::HandleScope m_handleScope;
    const v8::Context::Scope m_contextScope;
  };
}

#endif
