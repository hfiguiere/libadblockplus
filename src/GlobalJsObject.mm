#import "JSCJsEnginePrivate.h"
#import <AdblockPlus/JsEngine.h>
#import "ObjcHelper.h"
#import "JSFileSystem.h"
#import "JSWebRequest.h"
#import <sstream>

using namespace AdblockPlus;

namespace
{
  void Log(LogSystem::LogLevel logLevel)
  {
    ABPJSContext* jsContext = ObjcCast<ABPJSContext>([JSContext currentContext]);
    AdblockPlus::JsEnginePtr jsEngine;
    if (!jsContext || !(jsEngine = jsContext->m_jsEngine.lock()))
      return;
    
    NSArray* currentArguments = [JSContext currentArguments];
    std::stringstream ss;
    bool notFirstEntry = false;
    for (JSValue* argument in currentArguments)
    {
      if (notFirstEntry)
        ss << " ";
      if (argument == nil || [argument isUndefined])
        ss << "undefined";
      else
        ss << [[argument description] UTF8String];
      
      notFirstEntry = true;
    }
    (*jsEngine->GetLogSystem())(logLevel, ss.str(), "");
  }
}

@protocol ABPJSConsole <JSExport>
-(void)log;
-(void)debug;
-(void)info;
-(void)warn;
-(void)error;
-(void)trace;@end

@interface ABPJSConsoleImpl : NSObject<ABPJSConsole>
-(void)log;
-(void)debug;
-(void)info;
-(void)warn;
-(void)error;
-(void)trace;
@end

@implementation ABPJSConsoleImpl

-(void)log
{
  Log(LogSystem::LogLevel::LOG_LEVEL_LOG);
}

-(void)debug
{
  Log(LogSystem::LogLevel::LOG_LEVEL_LOG);
}

-(void)info
{
  Log(LogSystem::LogLevel::LOG_LEVEL_INFO);
}

-(void)warn
{
  Log(LogSystem::LogLevel::LOG_LEVEL_WARN);
}

-(void)error
{
  Log(LogSystem::LogLevel::LOG_LEVEL_ERROR);
}

-(void)trace
{
  Log(LogSystem::LogLevel::LOG_LEVEL_TRACE);
}
@end

namespace
{
  JsValuePtr createAppInfo(const JsEnginePtr& jsEngine, const AppInfo& appInfo)
  {
    auto jsAppInfo = jsEngine->NewObject();
    jsAppInfo->SetProperty("version", appInfo.version);
    jsAppInfo->SetProperty("name", appInfo.name);
    jsAppInfo->SetProperty("name", appInfo.name);
    jsAppInfo->SetProperty("application", appInfo.application);
    jsAppInfo->SetProperty("applicationVersion", appInfo.applicationVersion);
    jsAppInfo->SetProperty("locale", appInfo.locale);
    jsAppInfo->SetProperty("developmentBuild", appInfo.developmentBuild);
    return jsAppInfo;
  }
}

void GlobalJsObject::SetUp(const JsEnginePtr& jsEngine, const AppInfo& appInfo)
{
  ABPJSContext* jsContext = GetPrivateImpl(jsEngine)->jsContext;
  jsContext[@"console"] = [[ABPJSConsoleImpl alloc] init];
  jsContext[@"setTimeout"] = ^(JSValue* function, JSValue* timeout)
  {
    ABPJSContext* jsContext = ObjcCast<ABPJSContext>([JSContext currentContext]);
    if (!jsContext)
      return;
    [jsContext setTimeout:function withTimeout:timeout];
  };
  jsContext[@"_triggerEvent"] = ^()
  {
    ABPJSContext* jsContext = ObjcCast<ABPJSContext>([JSContext currentContext]);
    if (!jsContext)
      return;
    [jsContext triggerEvent];
  };
  jsContext[@"_fileSystem"] = [[ABPJSFileSystemImpl alloc] init];
  jsContext[@"_webRequest"] = [[ABPJSWebRequestImpl alloc] init];
  jsEngine->SetGlobalProperty("_appInfo", createAppInfo(jsEngine, appInfo));
}
