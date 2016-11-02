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

#include "JsContext.h"
#include "Utils.h"

AdblockPlus::JsContext::JsContext(const std::weak_ptr<JsEngine>& jsEngine)
  : m_jsEngine(Utils::lockJsEngine(jsEngine))
  , m_locker(m_jsEngine->GetIsolate())
  , m_isolateScope(m_jsEngine->GetIsolate())
  , m_handleScope(m_jsEngine->GetIsolate())
  , m_contextScope(v8::Local<v8::Context>::New(m_jsEngine->GetIsolate(), *m_jsEngine->context))
{
}
