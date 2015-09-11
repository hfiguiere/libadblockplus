#import "JSWebRequest.h"
#import "ObjCHelper.h"

#define ABPREQUEST_TIMEOUT 60.0

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
  
  NSHTTPURLResponse* httpUrlResponse = AdblockPlus::ObjcCast<NSHTTPURLResponse>(response);
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
  NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:url]
                                            cachePolicy:NSURLRequestUseProtocolCachePolicy
                                        timeoutInterval:ABPREQUEST_TIMEOUT];
  
  NSDictionary* headers = jsHeaders ? [jsHeaders toDictionary] : nil;
  for (NSString* headerName in headers)
  {
    [request setValue:[headers objectForKey:headerName] forHTTPHeaderField:headerName];
  }
  
  ABPJSWebRequestDelegateImpl_OnDoneCallback doneCallback = ^(int32_t networkStatus, int32_t statusCode, NSData* data, NSDictionary* headers)
  {
    JSContext* jsContext = [jsDoneCallback context];
    JSValue* jsResult = [JSValue valueWithNewObjectInContext:jsContext];
    [jsResult setValue:[JSValue valueWithInt32:networkStatus inContext:jsContext] forProperty:@"status"];
    [jsResult setValue:[JSValue valueWithInt32:statusCode inContext:jsContext] forProperty:@"responseStatus"];
    NSString* stringData = nil;
    if (data)
      if (const void* bytesData = [data bytes])
        stringData =[NSString stringWithUTF8String: static_cast<const char*>(bytesData)];
    [jsResult setValue:stringData forProperty:@"responseText"];
    
    JSValue* jsHeaders = [JSValue valueWithNewObjectInContext:jsContext];
    for (NSString* headerName in headers)
    {
      [jsHeaders setValue:[headers objectForKey:headerName] forProperty:headerName];
    }
    [jsResult setValue:jsHeaders forProperty:@"responseHeaders"];
    [jsDoneCallback callWithArguments:@[jsResult]];
  };
  
  ABPJSWebRequestDelegateImpl* connectionDelegate = [[ABPJSWebRequestDelegateImpl alloc] initWithOnDoneCallback:doneCallback];
  NSURLConnection* connection = [[NSURLConnection alloc] initWithRequest:request delegate:connectionDelegate];
  if (!connection)
  {
    doneCallback(-1, 0, nil, nil);
  }
}

@end