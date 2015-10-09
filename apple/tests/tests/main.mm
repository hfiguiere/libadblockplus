#import <Foundation/Foundation.h>
#import <gtest/gtest.h>
#import <AdblockPlus/FilterEngine.h>

namespace AdblockPlus
{
  void Sleep(const int millis)
  {
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:millis / 1000.]];
  }
}

using namespace AdblockPlus;
FilterEnginePtr FilterEngine::Create(const JsEnginePtr& jsEngine,
                                     const Prefs& preconfiguredPrefs)
{
  FilterEnginePtr retValue;
  bool initialized = false;
  CreateAsync(jsEngine, [&retValue, &initialized](const FilterEnginePtr& filterEngine)
  {
    retValue = filterEngine;
    initialized = true;
  }, preconfiguredPrefs);
  while (!initialized)
    AdblockPlus::Sleep(40);
  return retValue;
}

int main(int argc, char* argv[]) {
  @autoreleasepool {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
  }
  return 0;
}
