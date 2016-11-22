/*
 * This file is part of Adblock Plus <https://adblockplus.org/>,
 * Copyright (C) 2006-2016 Eyeo GmbH
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

#ifndef WIN32
#include <unistd.h>
#endif

#include "Thread.h"
#include <AdblockPlus/JsEngine.h>

using namespace AdblockPlus;

void AdblockPlus::Sleep(const int millis)
{
#ifdef WIN32
  ::Sleep(millis);
#else
  usleep(millis * 1000);
#endif
}

Mutex::Mutex()
{
#ifdef WIN32
  InitializeCriticalSection(&nativeMutex);
#else
  pthread_mutex_init(&nativeMutex, 0);
#endif
}

Mutex::~Mutex()
{
#ifdef WIN32
  DeleteCriticalSection(&nativeMutex);
#else
  pthread_mutex_destroy(&nativeMutex);
#endif
}

void Mutex::Lock()
{
#ifdef WIN32
  EnterCriticalSection(&nativeMutex);
#else
  pthread_mutex_lock(&nativeMutex);
#endif
}

void Mutex::Unlock()
{
#ifdef WIN32
  LeaveCriticalSection(&nativeMutex);
#else
  pthread_mutex_unlock(&nativeMutex);
#endif
}

Lock::Lock(Mutex& mutex) : mutex(mutex)
{
  mutex.Lock();
}

Lock::~Lock()
{
  mutex.Unlock();
}

Thread::Thread(bool deleteSelfOnFinish)
  : m_deleteSelfOnFinish(deleteSelfOnFinish)
{
}

Thread::~Thread()
{
}

void Thread::Start()
{
#ifdef WIN32
  nativeThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&CallRun, this, 0, 0);
#else
  pthread_create(&nativeThread, 0, (void* (*)(void*)) &CallRun, this);
#endif
}

void Thread::Join()
{
#ifdef WIN32
  WaitForSingleObject(nativeThread, INFINITE);
#else
  pthread_join(nativeThread, 0);
#endif
}

void Thread::CallRun(Thread* thread)
{
  try
  {
    thread->Run();
  }
  catch (const AdblockPlus::JsEngine::JsEngineNotAvailableException&)
  {
  }
  if (thread->m_deleteSelfOnFinish)
    delete thread;
}
