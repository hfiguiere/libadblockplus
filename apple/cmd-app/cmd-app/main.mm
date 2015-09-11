#import <AdblockPlus/JsEngine.h>
#import <AdblockPlus/FilterEngine.h>
#import <Foundation/Foundation.h>
#import <iostream>

@class Kernel;
typedef void(^Kernel_TimeoutBlock)(Kernel* kern);

@interface Kernel : NSObject
{
@public
  AdblockPlus::FilterEnginePtr filterEngine;
}
-(void)onTimeout:(NSDictionary*)data;
@end

@implementation Kernel

-(void)onTimeout:(NSDictionary*)data
{
  Kernel_TimeoutBlock block = data[@"callback"];
  block(self);
}

@end

int main(int argc, const char * argv[])
{
  @autoreleasepool
  {
    NSRunLoop* currentRunLoop = [NSRunLoop currentRunLoop];
    AdblockPlus::AppInfo appInfo;
    appInfo.version = "0.2";
    appInfo.application = "libadblockplus-iOS-test-driver";
    appInfo.applicationVersion = "0.1";
    appInfo.developmentBuild = true;
    
    auto jsEngine = AdblockPlus::JsEngine::New(appInfo);
    //*
    jsEngine->Evaluate("setTimeout(function(){console.log('timeout-1');}, 2000);");
    //*/
    
    /*
    jsEngine->Evaluate("_fileSystem.read('/Users/sergz/Documents/libadblockplus/src/FileSystemJsObject.cpp', function(result){_fileSystem.write('/Users/sergz/Documents/libadblockplus/src/FileSystemJsObject1.cpp', result.content, function(err){console.log(err);});});");
    //*/

    /*
     jsEngine->Evaluate("_webRequest.GET('http://whatheaders.com', {'x-x-xxAAA-xxx':'zzz-zzZZZZ'}, function(result){"
     "console.log(JSON.stringify(result));"
     "});");
     //*/
    
    /*
     {
     jsEngine->SetEventCallback("myInit", [](const AdblockPlus::JsValueList& params){
     std::cout << (params.size() ? params[0]->AsString() : "no args") << std::endl;
     });
     jsEngine->Evaluate("_triggerEvent('myInit', 'myArg');");
     }
     //*/
    
    /*
     {
     jsEngine->Evaluate("console.log(_appInfo.application);");
     }
     //*/
    
    /*
     {
     jsEngine->Evaluate("function MyC(){};");
     std::cout << jsEngine->Evaluate("new MyC()")->GetClass() << std::endl;
     }
     //*/

    AdblockPlus::FilterEngine::CreateAsync(jsEngine, [](const AdblockPlus::FilterEnginePtr& filterEngine){
      Kernel* kern = [Kernel new];
      kern->filterEngine = filterEngine;
      [kern performSelector:@selector(onTimeout:) withObject: @{@"callback":^(Kernel* kern){
        auto filter = kern->filterEngine->Matches("/ad/banner_", AdblockPlus::FilterEngine::CONTENT_TYPE_IMAGE, "");
        std::cout << filter.get() << std::endl;
        if (filter)
        {
          std::cout << filter->GetProperty("text")->AsString() << std::endl;
        }
      }} afterDelay:3];
    });
    
    //*
    jsEngine->Evaluate("setTimeout(function(){console.log('timeout-2');}, 2000);");
    //*/
    
    //[currentRunLoop run];
    [currentRunLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:10]];
  }
    return 0;
}
