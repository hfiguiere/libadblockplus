#import "JSWebRequest.h"
#import "ObjCHelper.h"
#import "JSCJsEnginePrivate.h"
#import <AdblockPlus/JsEngine.h>

#define ABPREQUEST_TIMEOUT 60.0

using AdblockPlus::ObjcCast;
using AdblockPlus::ToInt32;

typedef void(^ABPJSWebRequestDelegateImpl_OnDoneCallback)(int32_t networkStatus, int32_t statusCode, NSData* data, NSDictionary* headers);

@interface ABPJSWebRequestDelegateImpl : NSObject<NSURLConnectionDataDelegate>
-(instancetype)initWithOnDoneCallback:(ABPJSWebRequestDelegateImpl_OnDoneCallback) onDone;
- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response;
- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data;
- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error;
- (void)connectionDidFinishLoading:(NSURLConnection *)connection;

@property(nonatomic, copy) ABPJSWebRequestDelegateImpl_OnDoneCallback onDone;
@property(nonatomic) NSMutableData* data;
@property(nonatomic) int32_t statusCode;
@property(nonatomic) int32_t networkStatus;
@property(nonatomic, copy) NSDictionary* headers;
@end

@implementation ABPJSWebRequestDelegateImpl
-(instancetype)initWithOnDoneCallback:(ABPJSWebRequestDelegateImpl_OnDoneCallback) onDone;
{
  self = [super init];
  if (!self)
    return nil;
  
  self.onDone = onDone;
  return self;
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
  if (!self.data)
  {
    long long expectedContentLength = [response expectedContentLength];
    if (expectedContentLength == NSURLResponseUnknownLength)
    {
      self.data = [[NSMutableData alloc] init];
    }
    else
    {
      self.data = [[NSMutableData alloc] initWithCapacity:expectedContentLength];
    }
  }
  
  NSHTTPURLResponse* httpUrlResponse = ObjcCast<NSHTTPURLResponse>(response);
  if (httpUrlResponse)
  {
    self.statusCode = (int32_t)[httpUrlResponse statusCode];
    self.headers = [httpUrlResponse allHeaderFields];
  }
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
  [self.data appendData:data];
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
  self.onDone(self.networkStatus, self.statusCode, self.data, self.headers);
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
  self.onDone(self.networkStatus, self.statusCode, self.data, self.headers);
}
@end


@implementation ABPJSWebRequestImpl

-(instancetype)init
{
  self = [super init];
  if (!self)
    return nil;
  
  return self;
}

-(void)GET:(NSString*) url withHeaders:(JSValue*) jsHeaders doneCallback:(JSValue*) jsDoneCallback
{
  ABPJSWebRequestDelegateImpl_OnDoneCallback doneCallback = ^(int32_t networkStatus, int32_t statusCode, NSData* data, NSDictionary* headers)
  {
    JSContext* jsContext = [jsDoneCallback context];
    JSValue* jsResult = [JSValue valueWithNewObjectInContext:jsContext];
    [jsResult setValue:[JSValue valueWithInt32:networkStatus inContext:jsContext] forProperty:@"status"];
    [jsResult setValue:[JSValue valueWithInt32:statusCode inContext:jsContext] forProperty:@"responseStatus"];

    [jsResult setValue:AdblockPlus::ToNSString(data) forProperty:@"responseText"];
    
    JSValue* jsHeaders = [JSValue valueWithNewObjectInContext:jsContext];
    for (NSString* headerName in headers)
    {
      [jsHeaders setValue:[headers objectForKey:headerName] forProperty:[headerName lowercaseString]];
    }
    [jsResult setValue:jsHeaders forProperty:@"responseHeaders"];
    [jsDoneCallback callWithArguments:@[jsResult]];
  };
  
  ABPJSContext* jsContext = ObjcCast<ABPJSContext>([JSContext currentContext]);
  AdblockPlus::JsEnginePtr jsEngine;
  if (!jsContext || !(jsEngine = jsContext->m_jsEngine.lock()))
    return;
  
  // if there is overridden web request then use it
  if (auto webRequest = jsEngine->GetWebRequest())
  {
    AdblockPlus::HeaderList headerList;
    NSDictionary* headers = jsHeaders ? [jsHeaders toDictionary] : nil;
    for (NSString* headerName in headers)
    {
      NSString* headerValue = [headers objectForKey:headerName];
      headerList.emplace_back(std::make_pair([headerName UTF8String], [headerValue UTF8String]));
    }
    try
    {
      webRequest->GET([url UTF8String], headerList, [doneCallback](const AdblockPlus::ServerResponse& response)
      {
        NSMutableDictionary* headers = [NSMutableDictionary new];
        for (const auto& header: response.responseHeaders)
        {
          [headers setValue:@(header.second.c_str()) forKey:@(header.first.c_str())];
        }
      
        NSData* data = [[NSData alloc] initWithBytes:response.responseText.c_str() length:response.responseText.length()];
        doneCallback(ToInt32(response.status), response.responseStatus, data, headers);
      });
    }
    catch(...)
    {
      
    }
    return;
  }
  
  NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:url]
                                                         cachePolicy:NSURLRequestUseProtocolCachePolicy
                                                     timeoutInterval:ABPREQUEST_TIMEOUT];
  
  NSDictionary* headers = jsHeaders ? [jsHeaders toDictionary] : nil;
  for (NSString* headerName in headers)
  {
    [request setValue:[headers objectForKey:headerName] forHTTPHeaderField:headerName];
  }
  
  ABPJSWebRequestDelegateImpl* connectionDelegate = [[ABPJSWebRequestDelegateImpl alloc] initWithOnDoneCallback:doneCallback];
  NSURLConnection* connection = [[NSURLConnection alloc] initWithRequest:request delegate:connectionDelegate];
  if (!connection)
  {
    doneCallback(-1, 0, nil, nil);
  }
}

@end