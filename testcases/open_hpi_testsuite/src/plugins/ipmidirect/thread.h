/*
 * thread.h
 *
 * thread classes
 *
 * Copyright (c) 2004 by FORCE Computers.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Thomas Kanngieser <thomas.kanngieser@fci.com>
 */

#ifndef dThread_h
#define dThread_h


#include <pthread.h>
#include <unistd.h>


enum tTheadState
{
  eTsUnknown,
  eTsSuspend,
  eTsRun,
  eTsExit
};


// thread class
class cThread
{
protected:
  pthread_t   m_thread;
  bool        m_main;  // true => main thread
  tTheadState m_state;

  static void *Thread( void *param );

public:
  cThread();
  cThread( const pthread_t &thread, bool main_thread, tTheadState state );
  virtual ~cThread();

  // get the current thread class
  static cThread *GetThread();

  // start thread
  virtual bool Start();

  // wait for thread termination
  virtual bool Wait( void *&rv );

  bool IsRunning() { return m_state == eTsRun; }
  bool IsMain()  { return m_main; }

protected:
  virtual void *Run() = 0;
  virtual void Exit( void *rv );
};


// simple locks
class cThreadLock
{
protected:
  pthread_mutex_t m_lock;

public:
  cThreadLock();
  virtual ~cThreadLock();

  virtual void Lock();
  virtual void Unlock();

  virtual bool TryLock();
  virtual bool TimedLock( unsigned int timeout );
};


class cThreadLockAuto
{
  cThreadLock &m_lock;

public:
  cThreadLockAuto( cThreadLock &lock )
    : m_lock( lock )
  {
    m_lock.Lock();
  }
  
  ~cThreadLockAuto()
  {
    m_lock.Unlock();
  }
};



// read/write locks
class cThreadLockRw
{
protected:
  pthread_rwlock_t m_rwlock;

public:
  cThreadLockRw();
  virtual ~cThreadLockRw();

  virtual void ReadLock();
  virtual void ReadUnlock();
  virtual bool TryReadLock();
  virtual bool TimedReadLock( unsigned int timeout );

  virtual void WriteLock();
  virtual void WriteUnlock();
  virtual bool TryWriteLock();
  virtual bool TimedWriteLock( unsigned int timeout );
};


// condition class
class cThreadCond : public cThreadLock
{
protected:
  pthread_cond_t m_cond;

public:
  cThreadCond();
  virtual ~cThreadCond();

  // call Lock before Signal
  virtual void Signal(); 

  // call Lock before Wait
  virtual void Wait();

  // call Lock before TimedWait, return false on timeout
  virtual bool TimedWait( unsigned int timeout = 0 );
};


#endif
