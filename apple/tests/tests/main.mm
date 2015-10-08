#import <Foundation/Foundation.h>
#import <gtest/gtest.h>

namespace AdblockPlus
{
  void Sleep(const int millis)
  {
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:millis / 1000.]];
  }
}

int main(int argc, char* argv[]) {
  @autoreleasepool {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
  }
  return 0;
}
