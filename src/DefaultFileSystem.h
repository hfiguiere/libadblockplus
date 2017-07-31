/*
 * This file is part of Adblock Plus <https://adblockplus.org/>,
 * Copyright (C) 2006-2017 eyeo GmbH
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

#ifndef ADBLOCK_PLUS_DEFAULT_FILE_SYSTEM_H
#define ADBLOCK_PLUS_DEFAULT_FILE_SYSTEM_H

#include <AdblockPlus/IFileSystem.h>
#include <AdblockPlus/Scheduler.h>

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

namespace AdblockPlus
{
  /**
   * File system implementation that interacts directly with the operating
   * system's file system.
   * All paths are considered relative to the base path, or to the current
   * working directory if no base path is set (see `SetBasePath()`).
   */
  class DefaultFileSystemSync
  {
  public:
    IFileSystem::IOBuffer Read(const std::string& path) const;
    void Write(const std::string& path, const IFileSystem::IOBuffer& data);
    void Move(const std::string& fromPath, const std::string& toPath);
    void Remove(const std::string& path);
    IFileSystem::StatResult Stat(const std::string& path) const;
    std::string Resolve(const std::string& path) const;

    /**
     * Sets the base path, all paths are considered relative to it.
     * @param path Base path.
     */
    void SetBasePath(const std::string& path);

  protected:
    std::string basePath;
  };

  class DefaultFileSystem : public IFileSystem
  {
  public:
    explicit DefaultFileSystem(const Scheduler& scheduler, std::unique_ptr<DefaultFileSystemSync> syncImpl);
    void Read(const std::string& path,
              const ReadCallback& callback) const;
    void Write(const std::string& path,
               const IOBuffer& data,
               const Callback& callback);
    void Move(const std::string& fromPath,
              const std::string& toPath,
              const Callback& callback);
    void Remove(const std::string& path, const Callback& callback);
    void Stat(const std::string& path,
              const StatCallback& callback) const;

    std::string Resolve(const std::string& path) const;
  private:
    Scheduler scheduler;
    std::unique_ptr<DefaultFileSystemSync> syncImpl;
  };
}

#endif