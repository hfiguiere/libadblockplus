//
//  ViewController.m
//  ios-app
//
//  Created by Sergei Zabolotskikh on 10/09/15.
//  Copyright © 2015 Eyeo GmbH. All rights reserved.
//

#import "ViewController.h"
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


@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  
  AdblockPlus::AppInfo appInfo;
  appInfo.version = "0.2";
  appInfo.application = "libadblockplus-iOS-test-driver";
  appInfo.applicationVersion = "0.1";
  appInfo.developmentBuild = true;
  
  auto jsEngine = AdblockPlus::JsEngine::New(appInfo);
  
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
  
  // Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning {
  [super didReceiveMemoryWarning];
  // Dispose of any resources that can be recreated.
}

@end
