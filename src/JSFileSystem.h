#pragma once
#import <JavaScriptCore/JavaScriptCore.h>

@protocol ABPJSFileSystem <JSExport>
JSExportAs(read, -(void)read:(NSString*) path onRead:(JSValue*) doneCallback);
JSExportAs(write, -(void)write:(NSString*) path withContent:(NSString*) content onWritten:(JSValue*) doneCallback);
JSExportAs(move, -(void)move:(NSString*) sourcePath to:(NSString*) destinationPath onMoved:(JSValue*) doneCallback);
JSExportAs(remove, -(void)remove:(NSString*) path onRemoved:(JSValue*) doneCallback);
JSExportAs(stat, -(void)stat:(NSString*) path onDone:(JSValue*) doneCallback);
-(NSString*)resolve:(NSString*) path;
@end


// The callbacks are executed in the thread in which the instance of this class is created.
@interface ABPJSFileSystemImpl : NSObject<ABPJSFileSystem>
-(instancetype)init;
-(void)dealloc;

-(void)read:(NSString*) path onRead:(JSValue*) doneCallback;
-(void)write:(NSString*) path withContent:(NSString*) content onWritten:(JSValue*) doneCallback;
-(void)move:(NSString*) sourcePath to:(NSString*) destinationPath onMoved:(JSValue*) doneCallback;;
-(void)remove:(NSString*) path onRemoved:(JSValue*) doneCallback;
-(void)stat:(NSString*) path onDone:(JSValue*) doneCallback;
-(NSString*)resolve:(NSString*) path;

// Queues the action to be executed in the object's thread or executes it immediately if
// the current thread is the object's thread.
-(void)dispatchAction:(void(^)()) action;

@property(nonatomic, copy) NSString* basePath;
// We need this member to keep file reader and writer delegates alive. [NSInputStream setDelegate]
// stores the week reference to the delegate and the delegate goes out of scope (by decreasing the
// reference counter) right after the creating of it.
@property(nonatomic) NSMutableSet* fileReadersWriters;
@property(nonatomic) dispatch_queue_t queue;
@property(nonatomic) NSRunLoop* runLoop;
@end