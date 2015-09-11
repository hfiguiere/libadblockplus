#import "JSFileSystem.h"
#import "ObjCHelper.h"

#define PATH_SEPARATOR '/'
#define BLOCK_SIZE 4096

typedef void(^ABPFileReader_OnDoneCallback)(NSData* data, NSString* error);

@interface ABPFileReader : NSObject<NSStreamDelegate>
-(instancetype)initWithPath:(NSString*) path;
-(void)dealloc;
-(void)stream:(NSStream*) aStream handleEvent:(NSStreamEvent)eventCode;

@property(nonatomic, copy) ABPFileReader_OnDoneCallback onDone;
@property(nonatomic, copy) NSString* path;
@property(nonatomic) NSMutableData* data;
@property(nonatomic, weak) NSInputStream* stream;
@end

@implementation ABPFileReader

-(instancetype)initWithPath:(NSString*) path
{
  if (!(self = [super init]))
    return nil;

  NSInputStream* stream = [[NSInputStream alloc]initWithFileAtPath:self.path = path];
  self.stream = stream;
  [stream setDelegate:self];
  [stream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
  [stream open];
  return self;
}

-(void)dealloc
{
  // Restore default delegate if the current one is prematurely destroyed.
  // It can happen if the stream is referenced somewhere else but the current delegate is being
  // destroyed.
  NSStream* stream = self.stream;
  if (stream)
    [stream setDelegate:nil];
}

- (void)stream:(NSStream*) aStream handleEvent:(NSStreamEvent) eventCode
{
  assert((NSStream*)self.stream == aStream && "The stream argument should be the same as the member");
  switch (eventCode) {
    case NSStreamEventOpenCompleted:
      break;
    case NSStreamEventHasBytesAvailable:
      {
        if(!_data)
        {
          _data = [NSMutableData data];
        }
        uint8_t buf[BLOCK_SIZE];
        NSInteger len = [self.stream read:buf maxLength: sizeof(buf)];
        if(len)
        {
          [_data appendBytes:(const void *)buf length:len];
        }
      }
      break;
    case NSStreamEventEndEncountered:
      [aStream close];
      [aStream removeFromRunLoop:[NSRunLoop currentRunLoop]
                        forMode:NSDefaultRunLoopMode];
      self.onDone(self.data, nil);
      break;
    case NSStreamEventNone:
    case NSStreamEventErrorOccurred:
    default:
      {
        NSError* error = [aStream streamError];
        self.onDone(nil, [NSString stringWithFormat:@"Error %ld: %@", [error code], [error description]]);
      }
      break;
  }
}
@end

typedef void(^ABPFileWriter_OnDoneCallback)(NSString* error);

@interface ABPFileWriter : NSObject<NSStreamDelegate>
-(instancetype)initWithPath:(NSString*) path withData:(NSData*) data;
-(void)dealloc;
- (void)stream:(NSStream*) aStream handleEvent:(NSStreamEvent)eventCode;

@property(nonatomic, copy) ABPFileWriter_OnDoneCallback onDone;
@property(nonatomic, copy) NSString* path;
@property(nonatomic, copy) NSData* data;
@property(nonatomic) NSUInteger writtenDataOffset;
@property(nonatomic, weak) NSOutputStream* stream;
@end

@implementation ABPFileWriter

-(instancetype)initWithPath:(NSString*) path withData:(NSData*) data
{
  if (!(self = [super init]))
    return nil;
  
  self.data = data;
  NSOutputStream* stream = [[NSOutputStream alloc] initToFileAtPath: self.path = path append:false];
  self.stream = stream;
  [stream setDelegate:self];
  [stream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
  [stream open];
  return self;
}

-(void)dealloc
{
  // Restore default delegate if the current one is prematurely destroyed.
  // It can happen if the stream is referenced somewhere else but the current delegate is being
  // destroyed.
  NSStream* stream = self.stream;
  if (stream)
    [stream setDelegate:nil];
}

- (void)stream:(NSStream*) aStream handleEvent:(NSStreamEvent) eventCode
{
  assert((NSStream*)self.stream == aStream && "The stream argument should be the same as the member");
  switch (eventCode) {
    case NSStreamEventOpenCompleted:
      break;
    case NSStreamEventHasSpaceAvailable:
      {
        const uint8_t* bufferToWrite = (const uint8_t*)self.data.bytes;
        bufferToWrite += self.writtenDataOffset;
        NSUInteger leftToWrite = self.data.length - self.writtenDataOffset;
        leftToWrite = MIN(BLOCK_SIZE, leftToWrite);
        self.writtenDataOffset += [self.stream write: bufferToWrite maxLength: leftToWrite];
      }
      break;
    case NSStreamEventEndEncountered:
      self.onDone(nil);
      break;
    case NSStreamEventNone:
    case NSStreamEventErrorOccurred:
    default:
      {
        NSError* error = [aStream streamError];
        self.onDone([NSString stringWithFormat:@"Error %ld: %@", [error code], [error description]]);
      }
      break;
  }
}
@end

@implementation ABPJSFileSystemImpl

-(instancetype)init
{
  if (!(self = [super init]))
    return nil;
  
  self.runLoop = [NSRunLoop currentRunLoop];
  self.queue = dispatch_queue_create("org.libadblockplus.filesystem.queue", DISPATCH_QUEUE_CONCURRENT);
  self.fileReadersWriters = [[NSMutableSet alloc] init];
  
  return self;
}

-(void)dealloc
{
  // Wait for the finishing of all tasks in the queue before destroying it.
  // (because I'm not sure whether it waits in its dealloc.
  dispatch_barrier_sync(self.queue, ^{});
}

-(void)read:(NSString*) path onRead:(JSValue*) doneCallback
{
  ABPFileReader* fileReader = [[ABPFileReader alloc]initWithPath: path];
  // We should not capture self by strong reference because otherwise there will be a retain cycle
  // because we keep the strong reference to the delegate (ABPFileReader) which keeps
  // the strong reference to the block. By the same reason we should not capture fileReader by
  // the strong reference because it will keep the strongly referenced block which strong references
  // fileReader.
  ABPFileReader* __weak weakFileReader = fileReader;
  ABPJSFileSystemImpl* __weak weakSelf = self;
  fileReader.onDone = ^(NSData* data, NSString* error)
  {
    JSValue* callbackArg = [JSValue valueWithNewObjectInContext: [doneCallback context]];
    [callbackArg setValue:AdblockPlus::ToNSString(data) forProperty:@"content"];
    [callbackArg setValue:error forProperty:@"error"];
    [doneCallback callWithArguments: @[callbackArg]];
    [weakSelf.fileReadersWriters removeObject: weakFileReader];
  };
  [self.fileReadersWriters addObject:fileReader];
}

-(void)write:(NSString*) path withContent:(NSString*) content onWritten:(JSValue*) doneCallback
{
  ABPFileWriter* fileWriter = [[ABPFileWriter alloc]initWithPath: path withData:[content dataUsingEncoding:NSUTF8StringEncoding]];
  // We should not capture self by strong reference because otherwise there will be a retain cycle
  // because we keep the strong reference to the delegate (ABPFileWriter) which keeps
  // the strong reference to the block. By the same reason we should not capture fileWriter by
  // the strong reference because it will keep the strongly referenced block which strong references
  // fileWriter.
  ABPFileWriter* __weak weakFileWriter = fileWriter;
  ABPJSFileSystemImpl* __weak weakSelf = self;
  fileWriter.onDone = ^(NSString* error)
  {
    [doneCallback callWithArguments: error ? @[error] : @[]];
    [weakSelf.fileReadersWriters removeObject: weakFileWriter];
  };
  [self.fileReadersWriters addObject:fileWriter];
}

-(void)move:(NSString*) sourcePath to:(NSString*) destinationPath onMoved:(JSValue*) doneCallback
{
  dispatch_async(self.queue,
  ^{
    NSError* error = nil;
    [[NSFileManager defaultManager] moveItemAtPath:sourcePath toPath: destinationPath error:&error];
    if (!doneCallback)
      return;
    [self dispatchAction:^
    {
      [doneCallback callWithArguments:error ? @[[error description]] : @[]];
    }];
  });
}

-(void)remove:(NSString*) path onRemoved:(JSValue*) doneCallback
{
  dispatch_async(self.queue, ^
  {
    NSError* error = nil;
    [[NSFileManager defaultManager] removeItemAtPath:path error: &error];
    if (!doneCallback)
      return;
    [self dispatchAction:^
    {
      [doneCallback callWithArguments:error ? @[[error description]] : @[]];
    }];
  });
}

-(void)stat:(NSString*) path onDone:(JSValue*) doneCallback
{
  if (!doneCallback)
    return;

  dispatch_async(self.queue, ^
  {
    JSContext* jsContext = [doneCallback context];
    JSValue* result = [JSValue valueWithNewObjectInContext:jsContext];
    NSFileManager* fileManager = [NSFileManager defaultManager];
    BOOL isDirectory = NO;
    BOOL fileExists = [fileManager fileExistsAtPath:path isDirectory:&isDirectory];
    [result setValue:[JSValue valueWithBool:fileExists inContext:jsContext] forProperty:@"exists"];
    [result setValue:[JSValue valueWithBool:!isDirectory inContext:jsContext] forProperty:@"isFile"];
    [result setValue:[JSValue valueWithBool:isDirectory inContext:jsContext] forProperty:@"isDirectory"];
    if (fileExists == YES)
    {
      NSError* error = nil;
      NSDictionary* attributes = [fileManager attributesOfItemAtPath:path error:&error];
      NSDate* modificationDate = [attributes valueForKey:NSFileModificationDate];
      if (modificationDate)
      {
        const int32_t MSEC_IN_SEC = 1000;
        JSValue* lastModifiedJSValue = [JSValue valueWithDouble:[modificationDate timeIntervalSince1970] * MSEC_IN_SEC inContext:jsContext];
        [result setValue:lastModifiedJSValue forProperty:@"lastModified"];
      }
      if (error)
        [result setValue:[error description] forProperty:@"error"];
    }
    
    [self dispatchAction:^
    {
      [doneCallback callWithArguments:@[result]];
    }];
  });
}

-(NSString*)resolve:(NSString*) path
{
  if (_basePath == nil)
  {
    return path;
  }
  else
  {
    if (path != nil && [path length] && [path characterAtIndex:0] != PATH_SEPARATOR)
    {
      return [NSString pathWithComponents:[NSArray arrayWithObjects: _basePath, path, nil]];
    }
    else
    {
      return path;
    }
  }
}

-(void)dispatchAction:(void (^)())action
{
  if ([NSRunLoop currentRunLoop] == self.runLoop)
  {
    action();
  }
  else
  {
    [self.runLoop performSelector:@selector(dispatchAction:) target:self argument:[action copy] order:0 modes:@[NSDefaultRunLoopMode]];
  }
}

@end
