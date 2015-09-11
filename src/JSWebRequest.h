#pragma once
#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>

@protocol ABPJSWebRequest <JSExport>
JSExportAs(GET, -(void)GET:(NSString*) url withHeaders:(JSValue*) headers doneCallback:(JSValue*) doneCallback);
@end

@interface ABPJSWebRequestImpl : NSObject<ABPJSWebRequest>
-(instancetype)init;
-(void)GET:(NSString*) url withHeaders:(JSValue*) headers doneCallback:(JSValue*) doneCallback;
@end