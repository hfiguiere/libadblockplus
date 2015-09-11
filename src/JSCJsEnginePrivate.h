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

#ifndef ADBLOCK_PLUS_JSC_JS_ENGINE_PRIVATE_H
#define ADBLOCK_PLUS_JSC_JS_ENGINE_PRIVATE_H
#import "JsEnginePrivate.h"
#import <JavaScriptCore/JavaScriptCore.h>

namespace AdblockPlus
{
  class JSCJsEnginePrivateImpl;
}

// It seems there is a bug in JSContext::dealloc. I had not an oppotunity to debug
// JavaScriptCore internals to find out the exact reason and file a patch.
// Anyway it seems logical to destroy the virtual machine after destroying of the context.
@interface ABPJSContext : JSContext
{
  @public std::weak_ptr<AdblockPlus::JsEngine> m_jsEngine;
}
-(instancetype)initWithVirtualMachine:(JSVirtualMachine *)virtualMachine withParentJsEngine: (AdblockPlus::JsEnginePtr) parentJsEngine;
-(void)dealloc;
-(void)setTimeout:(JSValue*) callback withTimeout:(JSValue*) timeout;

-(void)onTimeout:(NSDictionary*)data;

-(void)triggerEvent;
@end

namespace AdblockPlus
{
  class JSCJsEnginePrivateImpl : public JsEnginePrivate
  {
    friend class JsValuePrivate;
    friend class GlobalJsObject;
  public:
    JSCJsEnginePrivateImpl();
    ~JSCJsEnginePrivateImpl();
    void Gc() override{};
    void Init(const JsEnginePtr& jsEngine, const AppInfo& appInfo) override;
    
    JsValuePtr Evaluate(const std::string& source, const std::string& filename = "") override;
    JsValuePtr NewObject() override;
    JsValuePtr NewValue(const std::string& val) override;
    JsValuePtr NewValue(int64_t val) override;
    JsValuePtr NewValue(bool val) override;
    void SetGlobalProperty(const std::string& name, const JsValuePtr& value) override;
  private:
    std::weak_ptr<JsEngine> parent;
    ABPJSContext* jsContext;
  };
  JSCJsEnginePrivateImpl* GetPrivateImpl(JsEngine& jsEngine);
  inline JSCJsEnginePrivateImpl* GetPrivateImpl(const JsEnginePtr& jsEngine)
  {
    return GetPrivateImpl(*jsEngine);
  }
  
  class GlobalJsObject
  {
  public:
    static void SetUp(const JsEnginePtr& jsEngine, const AppInfo& appInfo);
  };
}

#endif /* ADBLOCK_PLUS_JSC_JS_ENGINE_PRIVATE_H */
