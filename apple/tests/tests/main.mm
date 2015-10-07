#import <Foundation/Foundation.h>
#import <gtest/gtest.h>

int main(int argc, char* argv[]) {
  @autoreleasepool {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
  }
  return 0;
}
