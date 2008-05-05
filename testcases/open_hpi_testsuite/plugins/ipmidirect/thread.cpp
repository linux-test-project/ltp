/*
 * thread.cpp
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

#include "thread.h"
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>


//////////////////////////////////////////////////
//                  cThread
//////////////////////////////////////////////////


static pthread_key_t thread_key;

class cThreadMain : public cThread
{
public:
  cThreadMain( const pthread_t &thread, bool main_thread, tTheadState state )
    : cThread( thread, main_thread, state ) {}

protected:
  virtual void *Run() { return 0; }
};


class cInit
{
public:
  cInit();
  ~cInit();
};


cInit::cInit()
{
  pthread_key_create( &thread_key, 0 );
  cThreadMain *thread = new cThreadMain( pthread_self(), true, eTsRun );
  pthread_setspecific( thread_key, thread );
}


cInit::~cInit()
{
  cThreadMain *thread = (cThreadMain *)pthread_getspecific( thread_key );

  if ( thread )
  {
      delete thread;
      pthread_key_delete( thread_key );  
  }
}


static cInit init;


cThread::cThread()
  : m_main( false ), m_state( eTsSuspend )
{
}


cThread::cThread( const pthread_t &thread, bool main_thread, tTheadState state )
  : m_thread( thread ), m_main( main_thread ), m_state( state )
{
}


cThread::~cThread()
{
}


cThread *
cThread::GetThread()
{
  cThread *thread = (cThread *)pthread_getspecific( thread_key );

  return thread;
}


void *
cThread::Thread( void *param )
{
  cThread *thread = (cThread *)param;

  pthread_setspecific( thread_key, thread );

  thread->m_state = eTsRun;
  void *rv = thread->Run();
  thread->m_state = eTsExit;

  return rv;
}


bool
cThread::Start()
{
  if ( m_state == eTsRun )
     {
       return false;
     }

  m_state = eTsSuspend;

  int rv = pthread_create( &m_thread, 0, Thread, this );

  if ( rv )
       return false;

  // wait till the thread is runnung
  while( m_state == eTsSuspend )
       // wait 100 ms
       usleep( 10000 );

  return true;
}


bool
cThread::Wait( void *&rv )
{
  if ( m_state != eTsRun )
       return false;

  void *rr;

  int r = pthread_join( m_thread, &rr );

  if ( r )
       return false;

  rv = rr;

  return true;
}


void
cThread::Exit( void *rv )
{
  m_state = eTsExit;
  pthread_exit( rv );
}


//////////////////////////////////////////////////
//                  cThreadLock
//////////////////////////////////////////////////

#if defined(__sun) && defined(__SVR4)
cThreadLock::cThreadLock()
{
  pthread_mutexattr_t attr;

  pthread_mutexattr_init( &attr );
  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
  pthread_mutex_init( &m_lock, &attr );
  pthread_mutexattr_destroy( &attr );
}
#else
static pthread_mutex_t lock_tmpl = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


cThreadLock::cThreadLock()
  : m_lock( lock_tmpl )
{
}
#endif


cThreadLock::~cThreadLock()
{
  pthread_mutex_destroy( &m_lock );
}


void
cThreadLock::Lock()
{
  pthread_mutex_lock( &m_lock );
}


void
cThreadLock::Unlock()
{
  pthread_mutex_unlock( &m_lock );
}


bool
cThreadLock::TryLock()
{
  return pthread_mutex_trylock( &m_lock ) == 0;
}


//////////////////////////////////////////////////
//                  cThreadLockRw
//////////////////////////////////////////////////

#if defined(__sun) && defined(__SVR4)
cThreadLockRw::cThreadLockRw()
{
  pthread_rwlock_init( &m_rwlock, NULL );
}
#else
static pthread_rwlock_t rwlock_tmpl = PTHREAD_RWLOCK_INITIALIZER;

cThreadLockRw::cThreadLockRw()
{
  m_rwlock = rwlock_tmpl;
}
#endif


cThreadLockRw::~cThreadLockRw()
{
  pthread_rwlock_destroy( &m_rwlock );
}


void
cThreadLockRw::ReadLock()
{
  pthread_rwlock_rdlock( &m_rwlock );
}


void
cThreadLockRw::ReadUnlock()
{
  pthread_rwlock_unlock( &m_rwlock );
}


bool
cThreadLockRw::TryReadLock()
{
  int rv = pthread_rwlock_trywrlock( &m_rwlock );

  return rv == 0;
}


void
cThreadLockRw::WriteLock()
{
  pthread_rwlock_wrlock( &m_rwlock );
}


void
cThreadLockRw::WriteUnlock()
{
  pthread_rwlock_unlock( &m_rwlock );
}


bool
cThreadLockRw::TryWriteLock()
{
  int rv = pthread_rwlock_trywrlock( &m_rwlock );

  return rv == 0;
}


bool
cThreadLockRw::CheckLock()
{
  bool rv = TryWriteLock();
  
  if ( rv )
       WriteUnlock();

  return rv;
}


//////////////////////////////////////////////////
//                  cThreadCond
//////////////////////////////////////////////////

#if defined(__sun) && defined(__SVR4)
cThreadCond::cThreadCond()
{
  pthread_cond_init( &m_cond, NULL );
}
#else
static pthread_cond_t cond_tmpl = PTHREAD_COND_INITIALIZER;

cThreadCond::cThreadCond()
{
  m_cond = cond_tmpl;
}
#endif


cThreadCond::~cThreadCond()
{
  pthread_cond_destroy( &m_cond );
}


void
cThreadCond::Signal()
{
  pthread_cond_signal( &m_cond );
}


void
cThreadCond::Wait()
{
  pthread_cond_wait( &m_cond, &m_lock );
}

