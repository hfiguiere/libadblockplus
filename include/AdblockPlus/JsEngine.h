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

#ifndef ADBLOCK_PLUS_JS_ENGINE_H
#define ADBLOCK_PLUS_JS_ENGINE_H

#include <map>
#include <stdexcept>
#include <cstdint>
#include <string>
#include <functional>
#include "AppInfo.h"
#include "LogSystem.h"
#include "FileSystem.h"
#include "JsValue.h"
#include "WebRequest.h"
#include "Declarations.h"

namespace AdblockPlus
{
  class JsEnginePrivate;
  typedef std::unique_ptr<JsEnginePrivate> JsEnginePrivatePtr;
  JsEnginePrivatePtr NewJsEnginePrivate();

  /**
   * JavaScript engine used by `FilterEngine`, wraps v8.
   */
  class JsEngine : public std::enable_shared_from_this<JsEngine>
  {
  public:
    /**
     * Event callback function.
     */
    typedef std::function<void(const JsValueList& params)> EventCallback;
    /**
     * Maps events to callback functions.
     */
    typedef std::map<std::string, EventCallback> EventMap;
  public:
    ~JsEngine();

    /**
     * Creates a new JavaScript engine instance.
     * @param appInfo Information about the app.
     * @return New `JsEngine` instance.
     */
    static JsEnginePtr New(const AppInfo& appInfo = AppInfo());

    /**
     * Registers the callback function for an event.
     * @param eventName Event name. Note that this can be any string - it's a
     *        general purpose event handling mechanism.
     * @param callback Event callback function.
     */
    void SetEventCallback(const std::string& eventName, const EventCallback& callback);

    /**
     * Removes the callback function for an event.
     * @param eventName Event name.
     */
    void RemoveEventCallback(const std::string& eventName);

    /**
     * Triggers an event.
     * @param eventName Event name.
     * @param params Event parameters.
     */
    void TriggerEvent(const std::string& eventName, const JsValueList& params);

    /**
     * Evaluates a JavaScript expression.
     * @param source JavaScript expression to evaluate.
     * @param filename Optional file name for the expression, used in error
     *        messages.
     * @return Result of the evaluated expression.
     */
    JsValuePtr Evaluate(const std::string& source,
        const std::string& filename = "");

    /**
     * Initiates a garbage collection.
     */
    void Gc();

    //@{
    /**
     * Creates a new JavaScript value.
     * @param val Value to convert.
     * @return New `JsValue` instance.
     */
    JsValuePtr NewValue(const std::string& val);
    JsValuePtr NewValue(int64_t val);
    JsValuePtr NewValue(bool val);
    inline JsValuePtr NewValue(const char* val)
    {
      return NewValue(std::string(val));
    }
    inline JsValuePtr NewValue(int val)
    {
      return NewValue(static_cast<int64_t>(val));
    }
#ifdef __APPLE__
    inline JsValuePtr NewValue(long val)
    {
      return NewValue(static_cast<int64_t>(val));
    }
#endif
    //@}

    /**
     * Creates a new JavaScript object.
     * @return New `JsValue` instance.
     */
    JsValuePtr NewObject();

    /**
     * @see `SetFileSystem()`.
     */
    FileSystemPtr GetFileSystem();

    /**
     * Sets the `FileSystem` implementation used for all file I/O.
     * Setting this is optional, the engine will use a `DefaultFileSystem`
     * instance by default, which might be sufficient.
     * @param The `FileSystem` instance to use.
     */
    void SetFileSystem(FileSystemPtr val);

    /**
     * @see `SetWebRequest()`.
     */
    WebRequestPtr GetWebRequest();

    /**
     * Sets the `WebRequest` implementation used for XMLHttpRequests.
     * Setting this is optional, the engine will use a `DefaultWebRequest`
     * instance by default, which might be sufficient.
     * @param The `WebRequest` instance to use.
     */
    void SetWebRequest(WebRequestPtr val);
    /**
     * @see `SetLogSystem()`.
     */
    LogSystemPtr GetLogSystem();

    /**
     * Sets the `LogSystem` implementation used for logging (e.g. to handle
     * `console.log()` calls from JavaScript).
     * Setting this is optional, the engine will use a `DefaultLogSystem`
     * instance by default, which might be sufficient.
     * @param The `LogSystem` instance to use.
     */
    void SetLogSystem(LogSystemPtr val);

    /**
     * Sets a global property that can be accessed by all the scripts.
     * @param name Name of the property to set.
     * @param value Value of the property to set.
     */
    void SetGlobalProperty(const std::string& name, const JsValuePtr& value);
    JsEnginePrivate* PrivateImplementation()
    {
      return privateImpl.get();
    }
  private:
    JsEngine();
    JsEnginePrivatePtr privateImpl;
    EventMap eventCallbacks;
    FileSystemPtr fileSystem;
    WebRequestPtr webRequest;
    LogSystemPtr logSystem;
  };
}

#endif
