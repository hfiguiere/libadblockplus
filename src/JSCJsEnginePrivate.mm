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

#import <AdblockPlus/JsEngine.h>
#import "JSCJsEnginePrivate.h"
#import "JSCJsValuePrivate.h"

using namespace AdblockPlus;

@implementation ABPJSContext
-(instancetype)initWithVirtualMachine:(JSVirtualMachine *)virtualMachine withParentJsEngine: (AdblockPlus::JsEnginePtr) parentJsEngine
{
  if (!(self = [super initWithVirtualMachine:virtualMachine]))
    return nil;
  
  m_jsEngine = parentJsEngine;
  return self;
}

-(void)dealloc
{
  JSVirtualMachine* virtualMachine = super.virtualMachine;
  [virtualMachine retain];
  [super dealloc];
  [virtualMachine release];
}

-(void)setTimeout:(JSValue *)callback withTimeout:(JSValue *)timeout
{
  if (!m_jsEngine.lock())
    return;
  
  [self performSelector:@selector(onTimeout:) withObject: @{@"callback":callback} afterDelay:[timeout toDouble] / 1000.];
}

-(void)onTimeout:(NSDictionary*)data
{
  if (!m_jsEngine.lock())
    return;
  
  JSValue* callback = data[@"callback"];
  [callback callWithArguments:nil];
}

-(void)triggerEvent
{
  AdblockPlus::JsEnginePtr jsEngine = m_jsEngine.lock();
  if (!jsEngine)
    return;
  
  NSArray* currentArguments = [JSContext currentArguments];
  NSUInteger argc = [currentArguments count];
  if (argc == 0)
    return;
  NSString* nsStrventName = [[currentArguments objectAtIndex:0] toString];
  std::string eventName([nsStrventName UTF8String], [nsStrventName length]);
  JsValueList jsValues;
  for (NSUInteger argIndex = 1; argIndex < argc; ++argIndex)
  {
    JSValue* argument =[currentArguments objectAtIndex:argIndex];
    std::unique_ptr<JsValuePrivate> jsValuePrivate(new JSCJsValuePrivateImpl(jsEngine->shared_from_this(), argument));
    jsValues.emplace_back(new JsValue(jsEngine->shared_from_this(), move(jsValuePrivate)));
  }
  jsEngine->TriggerEvent(eventName, jsValues);
}
@end


JsEnginePrivatePtr AdblockPlus::NewJsEnginePrivate()
{
  return std::unique_ptr<JsEnginePrivate>(new JSCJsEnginePrivateImpl());
}

JSCJsEnginePrivateImpl::JSCJsEnginePrivateImpl()
{

}

JSCJsEnginePrivateImpl::~JSCJsEnginePrivateImpl()
{
  [jsContext release];
  jsContext = nil;
}

void JSCJsEnginePrivateImpl::Init(const JsEnginePtr& jsEngine, const AppInfo& appInfo)
{
  JSVirtualMachine* jsVirtualMachine = [[JSVirtualMachine alloc] init];
  jsContext = [[ABPJSContext alloc] initWithVirtualMachine: jsVirtualMachine withParentJsEngine:jsEngine];
  [jsVirtualMachine release];
  parent = jsEngine;
  GlobalJsObject::SetUp(jsEngine, appInfo);
}

JsValuePtr JSCJsEnginePrivateImpl::Evaluate(const std::string& source, const std::string& filename)
{
  JSValue* jsValue = [jsContext evaluateScript:@(source.c_str()) withSourceURL:[NSURL URLWithString:@(filename.c_str())]];
  std::unique_ptr<JsValuePrivate> jsValuePrivate(new JSCJsValuePrivateImpl(parent.lock(), jsValue));
  return JsValuePtr(new JsValue(parent.lock(), move(jsValuePrivate)));
}

JsValuePtr JSCJsEnginePrivateImpl::NewObject()
{
  JSValue* jsValue = [JSValue valueWithNewObjectInContext:jsContext];
  JsValuePrivatePtr jsValuePrivate(new JSCJsValuePrivateImpl(parent.lock(), jsValue));
  return std::shared_ptr<JsValue>(new JsValue(parent.lock(), move(jsValuePrivate)));
}

JsValuePtr JSCJsEnginePrivateImpl::NewValue(const std::string& val)
{
  JSValue* jsValue = [JSValue valueWithObject:@(val.c_str()) inContext:jsContext];
  JsValuePrivatePtr jsValuePrivate(new JSCJsValuePrivateImpl(parent.lock(), jsValue));
  return std::shared_ptr<JsValue>(new JsValue(parent.lock(), move(jsValuePrivate)));
}

JsValuePtr JSCJsEnginePrivateImpl::NewValue(int64_t val)
{
  JSValue* jsValue = [JSValue valueWithInt32:static_cast<int32_t>(val) inContext:jsContext];
  JsValuePrivatePtr jsValuePrivate(new JSCJsValuePrivateImpl(parent.lock(), jsValue));
  return std::shared_ptr<JsValue>(new JsValue(parent.lock(), move(jsValuePrivate)));
}

JsValuePtr JSCJsEnginePrivateImpl::NewValue(bool val)
{
  JSValue* jsValue = [JSValue valueWithBool:val ? YES : NO inContext:jsContext];
  JsValuePrivatePtr jsValuePrivate(new JSCJsValuePrivateImpl(parent.lock(), jsValue));
  return std::shared_ptr<JsValue>(new JsValue(parent.lock(), move(jsValuePrivate)));
}

void JSCJsEnginePrivateImpl::SetGlobalProperty(const std::string& name, const JsValuePtr& value)
{
  jsContext[@(name.c_str())] = GetPrivateImpl(value)->jsValue;
}

JSCJsEnginePrivateImpl* AdblockPlus::GetPrivateImpl(JsEngine& jsEngine)
{
  return static_cast<JSCJsEnginePrivateImpl*>(jsEngine.PrivateImplementation());
}

