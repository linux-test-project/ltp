/*
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
#include "test.h"
#include "ipmi_utils.h"


#define dMagic 0x47130815

class cThreadTest : public cThread, public cThreadLock, public cThreadLockRw
{
public:
  cThreadTest()
    : m_magic( dMagic )  {}
  
  unsigned int m_magic;

protected:
  virtual void *Run();
};


void *
cThreadTest::Run()
{
  cTime tn = cTime::Now();
  tn += 1000;

  cThreadTest *thread = (cThreadTest *)cThread::GetThread();

  Test( thread == this );
  Test( thread->m_magic == dMagic );

  while( tn > cTime::Now() )
       usleep( 100000 );

  return 0;
}


int
main()
{
  cThread *mt = cThread::GetThread();

  Test( mt );
  Test( mt->IsRunning() );
  Test( mt->IsMain() );

  cThreadTest thread;

  Test( thread.Start() );
  Test( thread.IsRunning() );

  while( thread.IsRunning() )
       usleep( 100000 );

  return TestResult();
}
