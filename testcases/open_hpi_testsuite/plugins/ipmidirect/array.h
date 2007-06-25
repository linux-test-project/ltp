/*
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

#ifndef dArray_h
#define dArray_h


#include <assert.h>


template <class T> class cArray
{
  T **m_array;
  int m_num;
  int m_size;
  int m_resize;

public:
  cArray( int r = 1 )
     : m_array( 0 ), m_num( 0 ), m_size( 0 ), m_resize( r )
  {
    assert( r > 0 );
  }

  cArray( const cArray &array )
    : m_array( 0 ), m_num( 0 ), m_size( 0 ), m_resize( array.m_resize )
  {
  }
  
  ~cArray()
  {
    Clear();
  }

  int Add( T *t )
  {
    if ( m_num == m_size )
       {
         T **newa = new T *[m_size+m_resize];

         if ( m_num )
              memcpy( newa, m_array, sizeof( T * ) * m_num );

         delete[] m_array;
         m_array = newa;
         m_size += m_resize;
       }

    m_array[m_num++] = t;

    return m_num-1;
  }

  T *Rem( int idx )
  {
    assert( idx >= 0 && idx < m_num );

    T *rv = m_array[idx];
    m_num--;

    if ( m_num == 0 )
	 return rv;

    int n = m_num/m_resize*m_resize+m_resize-1;

    if ( m_size > n )
       {
         m_size = n;

         T **newa = new T *[n];

         if ( idx != 0 )
              memcpy( newa, m_array, sizeof( T * ) * idx );

         if ( idx != m_num )
              memcpy( newa+idx, m_array+idx+1, (m_num - idx)*sizeof( T * ) );

         delete[] m_array;
         m_array = newa;

         return rv;
       }

    if ( idx != m_num )
         memmove( m_array+idx, m_array+idx+1, (m_num - idx)*sizeof( T * ) );

    return rv;
  }

  void RemAll()
  {
    if ( m_array )
       {
         delete[] m_array;
         m_num   = 0;
         m_array = 0;
         m_size  = 0;
       }
  }
  
  T *operator[]( int idx ) const
  {
    assert( idx >= 0 && idx < m_num );

    return m_array[idx];
  }

  T * & operator[]( int idx )
  {
    assert( idx >= 0 && idx < m_num );

    return m_array[idx];
  }

  cArray<T> &operator+=( T *t )
  {
    Add( t );
    return *this;
  }

  cArray<T> &operator-=( T *t )
  {
    int idx = Find( t );
    assert( idx != -1 );

    if ( idx != -1 )
         Rem( idx );

    return *this;
  }

  int Num() const { return m_num; }

  int Find( T *t ) const
  {
    for( int i = 0; i < m_num; i++ )
	 if ( m_array[i] == t )
	      return i;

    return -1;
  }

  void Sort( int (*cmp)( T **t1, T **t2 ) )
  {
    qsort( m_array, m_num, sizeof( T * ), (int (*)(const void *, const void *) )cmp );
  }

  int Search( T *key, int (*cmp)( T **t1, T **t2 ), int mmax = -1 ) const
  {
    int n = m_num;
    
    if ( mmax >= 0 && mmax < m_num )
	 n = mmax;
    
    T **e = (T **)bsearch( &key, m_array, n, sizeof( T * ), (int (*)(const void *, const void *) )cmp );

    if ( e == 0 )
         return -1;

    int idx = (e - m_array);

    assert( idx >= 0 && idx < n );

    return idx;
  }

  void Clear()
  {
    if ( m_array )
       {
         for( int i = 0; i < m_num; i++ )  delete m_array[i];

         delete[] m_array;
         m_num   = 0;
         m_array = 0;
         m_size  = 0;
       }
  }

  int Insert( int befor, T *t )
  {
    assert( befor <= m_num );
    
    if ( befor == -1 || befor == m_num )
         return Add( t );

    if ( m_num == m_size )
       {
         T **newa = new T *[m_size+m_resize];

         if ( m_num )
              memcpy( newa, m_array, sizeof( T * ) * m_num );

         delete[] m_array;
         m_array = newa;
         m_size += m_resize;
       }

    for( int i = m_num-1; i >= befor; i-- )
         m_array[i+1] = m_array[i];

    m_num++;
    m_array[befor] = t;

    return befor;
  }

  cArray &operator=( const cArray & /*array*/ )
  {
    // this is not a real copy operator !
    Clear();

    return *this;
  }
};


#endif
