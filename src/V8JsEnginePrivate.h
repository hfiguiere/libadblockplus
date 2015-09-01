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

#ifndef ADBLOCK_PLUS_V8_JS_ENGINE_PRIVATE_H
#define ADBLOCK_PLUS_V8_JS_ENGINE_PRIVATE_H

#include <AdblockPlus/V8ValueHolder.h>
#include <AdblockPlus/Declarations.h>
#include "JsContext.h"

namespace AdblockPlus
{
  class JsEnginePrivate
  {
    friend class JsValuePrivate;
    friend class JsContext;
  public:
    static std::unique_ptr<JsEnginePrivate> New();
    void Gc();
    void Init(const JsEnginePtr& jsEngine, const AppInfo& appInfo);
    /**
     * Creates a JavaScript function that invokes a C++ callback.
     * @param callback C++ callback to invoke. The callback receives a
     *        `v8::Arguments` object and can use `FromArguments()` to retrieve
     *        the current `JsEngine`.
     * @return New `JsValue` instance.
     */
    JsValuePtr NewCallback(v8::InvocationCallback callback);

    JsValuePtr Evaluate(const std::string& source, const std::string& filename = "");
    JsValuePtr NewObject();
    JsValuePtr NewValue(const std::string& val);
    JsValuePtr NewValue(int64_t val);
    JsValuePtr NewValue(bool val);
    void SetGlobalProperty(const std::string& name, const JsValuePtr& value);

    /**
     * Returns a `JsEngine` instance contained in a `v8::Arguments` object.
     * Use this in callbacks created via `NewCallback()` to retrieve the current
     * `JsEngine`.
     * @param arguments `v8::Arguments` object containing the `JsEngine`
     *        instance.
     * @return `JsEngine` instance from `v8::Arguments`.
     */
    static JsEnginePtr FromArguments(const v8::Arguments& arguments);

    /**
     * Converts v8 arguments to `JsValue` objects.
     * @param arguments `v8::Arguments` object containing the arguments to
     *        convert.
     * @return List of arguments converted to `JsValue` objects.
     */
    JsValueList ConvertArguments(const v8::Arguments& arguments);

    JsValuePtr CreateJsValuePtr(v8::Local<v8::Value> v8value);
  private:
    JsEnginePrivate();
  private:
    std::weak_ptr<JsEngine> parent;
    V8ValueHolder<v8::Context> context;
    JsValuePtr globalJsObject;
    v8::Isolate* isolate;
  };
}
#endif // ADBLOCK_PLUS_V8_JS_ENGINE_PRIVATE_H
