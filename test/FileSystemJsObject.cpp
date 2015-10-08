/*
 * This file is part of Adblock Plus <https://adblockplus.org/>,
 * Copyright (C) 2006-2015 Eyeo GmbH
 *
 * Adblock Plus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * Adblock Plus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Adblock Plus.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sstream>
#include "BaseJsTest.h"
#include "../src/Thread.h"
#include <mutex>
#include <condition_variable>

namespace
{
  class ScopedWaiter
  {
#ifndef ABP_JAVASCRIPT_CORE
    std::mutex waitMutex;
    std::condition_variable waitCV;
#endif
    bool result;
  public:
    ScopedWaiter() : result(false)
    {}
    void notify()
    {
#ifndef ABP_JAVASCRIPT_CORE
      std::lock_guard<std::mutex> lock(waitMutex);
#endif
      result = true;
#ifndef ABP_JAVASCRIPT_CORE
      waitCV.notify_one();
#endif
    }
    void wait()
    {
#ifndef ABP_JAVASCRIPT_CORE
      std::unique_lock<std::mutex> lock(waitMutex);
#endif
      while(!result)
#ifndef ABP_JAVASCRIPT_CORE
        waitCV.wait(lock);
#else
        AdblockPlus::Sleep(40);
#endif
    }
  };
  class MockFileSystem : public AdblockPlus::FileSystem
  {
  public:
    bool success;
    std::string contentToRead;
    std::string lastWrittenPath;
    std::string lastWrittenContent;
    std::string movedFrom;
    std::string movedTo;
    std::string removedPath;
    mutable std::string statPath;
    bool statExists;
    bool statIsDirectory;
    bool statIsFile;
    int statLastModified;

    MockFileSystem() : success(true)
    {
    }

    std::shared_ptr<std::istream> Read(const std::string& path) const
    {
      if (!success)
        throw std::runtime_error("Unable to read " + path);
      std::stringstream* const stream = new std::stringstream;
      *stream << contentToRead;
      return std::shared_ptr<std::istream>(stream);
    }

    void Write(const std::string& path, std::shared_ptr<std::istream> data)
    {
      if (!success)
        throw std::runtime_error("Unable to write to " + path);
      lastWrittenPath = path;

      std::stringstream content;
      content << data->rdbuf();
      lastWrittenContent = content.str();
    }

    void Move(const std::string& fromPath, const std::string& toPath)
    {
      if (!success)
        throw std::runtime_error("Unable to move " + fromPath + " to "
                                 + toPath);
      movedFrom = fromPath;
      movedTo = toPath;
    }

    void Remove(const std::string& path)
    {
      if (!success)
        throw std::runtime_error("Unable to remove " + path);
      removedPath = path;
    }

    StatResult Stat(const std::string& path) const
    {
      if (!success)
        throw std::runtime_error("Unable to stat " + path);
      statPath = path;
      StatResult result;
      result.exists = statExists;
      result.isDirectory = statIsDirectory;
      result.isFile = statIsFile;
      result.lastModified = statLastModified;
      return result;
    }

    std::string Resolve(const std::string& path) const
    {
      if (!success)
        throw std::runtime_error("Unable to stat " + path);
      return path;
    }
  };

  void ReadFile(AdblockPlus::JsEnginePtr jsEngine, std::string& content,
                std::string& error)
  {
    ScopedWaiter waiter;
    jsEngine->SetEventCallback("_onReadDone", [&](const AdblockPlus::JsValueList&)
    {
      waiter.notify();
      jsEngine->RemoveEventCallback("_onReadDone");
    });
    jsEngine->Evaluate("_fileSystem.read('', function(r) {result = r;this._triggerEvent('_onReadDone');})");
    waiter.wait();
    content = jsEngine->Evaluate("result.content")->AsString();
    error = jsEngine->Evaluate("result.error")->AsString();
  }

  typedef std::shared_ptr<MockFileSystem> MockFileSystemPtr;

  class FileSystemJsObjectTest : public BaseJsTest
  {
  protected:
    MockFileSystemPtr mockFileSystem;

    void SetUp()
    {
      BaseJsTest::SetUp();
      mockFileSystem = MockFileSystemPtr(new MockFileSystem);
      jsEngine->SetFileSystem(mockFileSystem);
    }
  };
}

TEST_F(FileSystemJsObjectTest, Read)
{
  mockFileSystem->contentToRead = "foo";
  std::string content;
  std::string error;
  ReadFile(jsEngine, content, error);
  ASSERT_EQ("foo", content);
  ASSERT_EQ("", error);
}
#ifndef ABP_JAVASCRIPT_CORE
TEST_F(FileSystemJsObjectTest, ReadIllegalArguments)
{
  ASSERT_ANY_THROW(jsEngine->Evaluate("_fileSystem.read()"));
  ASSERT_ANY_THROW(jsEngine->Evaluate("_fileSystem.read('', '')"));
}
#endif

TEST_F(FileSystemJsObjectTest, ReadError)
{
  mockFileSystem->success = false;
  std::string content;
  std::string error;
  ReadFile(jsEngine, content, error);
  ASSERT_NE("", error);
  ASSERT_EQ("", content);
}

TEST_F(FileSystemJsObjectTest, Write)
{
  jsEngine->Evaluate("_fileSystem.write('foo', 'bar', function(e) {error = e})");
  AdblockPlus::Sleep(50);
  ASSERT_EQ("foo", mockFileSystem->lastWrittenPath);
  ASSERT_EQ("bar", mockFileSystem->lastWrittenContent);
  ASSERT_EQ("", jsEngine->Evaluate("error")->AsString());
}

#ifndef ABP_JAVASCRIPT_CORE
TEST_F(FileSystemJsObjectTest, WriteIllegalArguments)
{
  ASSERT_ANY_THROW(jsEngine->Evaluate("_fileSystem.write()"));
  ASSERT_ANY_THROW(jsEngine->Evaluate("_fileSystem.write('', '', '')"));
}
#endif

TEST_F(FileSystemJsObjectTest, WriteError)
{
  mockFileSystem->success = false;
  jsEngine->Evaluate("_fileSystem.write('foo', 'bar', function(e) {error = e})");
  AdblockPlus::Sleep(50);
  ASSERT_NE("", jsEngine->Evaluate("error")->AsString());
}

TEST_F(FileSystemJsObjectTest, Move)
{
  jsEngine->Evaluate("_fileSystem.move('foo', 'bar', function(e) {error = e})");
  AdblockPlus::Sleep(50);
  ASSERT_EQ("foo", mockFileSystem->movedFrom);
  ASSERT_EQ("bar", mockFileSystem->movedTo);
  ASSERT_EQ("", jsEngine->Evaluate("error")->AsString());
}

#ifndef ABP_JAVASCRIPT_CORE
TEST_F(FileSystemJsObjectTest, MoveIllegalArguments)
{
  ASSERT_ANY_THROW(jsEngine->Evaluate("_fileSystem.move()"));
  ASSERT_ANY_THROW(jsEngine->Evaluate("_fileSystem.move('', '', '')"));
}
#endif

TEST_F(FileSystemJsObjectTest, MoveError)
{
  mockFileSystem->success = false;
  jsEngine->Evaluate("_fileSystem.move('foo', 'bar', function(e) {error = e})");
  AdblockPlus::Sleep(50);
  ASSERT_NE("", jsEngine->Evaluate("error")->AsString());
}

TEST_F(FileSystemJsObjectTest, Remove)
{
  jsEngine->Evaluate("_fileSystem.remove('foo', function(e) {error = e})");
  AdblockPlus::Sleep(50);
  ASSERT_EQ("foo", mockFileSystem->removedPath);
  ASSERT_EQ("", jsEngine->Evaluate("error")->AsString());
}

#ifndef ABP_JAVASCRIPT_CORE
TEST_F(FileSystemJsObjectTest, RemoveIllegalArguments)
{
  ASSERT_ANY_THROW(jsEngine->Evaluate("_fileSystem.remove()"));
  ASSERT_ANY_THROW(jsEngine->Evaluate("_fileSystem.remove('', '')"));
}
#endif

TEST_F(FileSystemJsObjectTest, RemoveError)
{
  mockFileSystem->success = false;
  jsEngine->Evaluate("_fileSystem.remove('foo', function(e) {error = e})");
  AdblockPlus::Sleep(50);
  ASSERT_NE("", jsEngine->Evaluate("error")->AsString());
}

TEST_F(FileSystemJsObjectTest, Stat)
{
  mockFileSystem->statExists = true;
  mockFileSystem->statIsDirectory= false;
  mockFileSystem->statIsFile = true;
  mockFileSystem->statLastModified = 1337;
  jsEngine->Evaluate("_fileSystem.stat('foo', function(r) {result = r})");
  AdblockPlus::Sleep(50);
  ASSERT_EQ("foo", mockFileSystem->statPath);
  ASSERT_EQ("", jsEngine->Evaluate("result.error")->AsString());
  ASSERT_TRUE(jsEngine->Evaluate("result.exists")->AsBool());
  ASSERT_FALSE(jsEngine->Evaluate("result.isDirectory")->AsBool());
  ASSERT_TRUE(jsEngine->Evaluate("result.isFile")->AsBool());
  ASSERT_EQ(1337, jsEngine->Evaluate("result.lastModified")->AsInt());
}

#ifndef ABP_JAVASCRIPT_CORE
TEST_F(FileSystemJsObjectTest, StatIllegalArguments)
{
  ASSERT_ANY_THROW(jsEngine->Evaluate("_fileSystem.stat()"));
  ASSERT_ANY_THROW(jsEngine->Evaluate("_fileSystem.stat('', '')"));
}
#endif

TEST_F(FileSystemJsObjectTest, StatError)
{
  mockFileSystem->success = false;
  jsEngine->Evaluate("_fileSystem.stat('foo', function(r) {result = r})");
  AdblockPlus::Sleep(50);
  ASSERT_NE("", jsEngine->Evaluate("result.error")->AsString());
}
