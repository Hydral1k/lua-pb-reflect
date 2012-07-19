/**
 * @file qpb_array.h
 *
 * \internal
 * Copyright (c) 2012, everMany, LLC.
 * All rights reserved.
 * 
 * Code licensed under the "New BSD" (BSD 3-Clause) License
 * See License.txt for complete information.
 */
#pragma once
#ifndef __QPB_ARRAY_H__
#define __QPB_ARRAY_H__

#include "qpb_forwards.h"
#include "qpb_ref.h"

struct QpbMessage;

//---------------------------------------------------------------------------
/**
 * POD-like type managed by lua, we are a proxy to iterate through repeated fields
 */
struct QpbArray
{
  typedef google::protobuf::Message Message;
  typedef google::protobuf::FieldDescriptor FieldDescriptor;

  static int PushProxy( lua_State*L, const Message&m, const FieldDescriptor *f) {
    return PushProxy( L, QpbRef(m), f );
  }
  static int PushProxy( lua_State*L, Message*m, const FieldDescriptor *f){
    return PushProxy( L, QpbRef(m), f );
  }
  
  static QpbArray* GetUserData( lua_State *, int idx= QPB_ARRAY_SELF );
  int collect(lua_State*);

  int size() const;
  int size( lua_State * ) const;
  int get( lua_State * ) const;
  int get_raw( lua_State *, int idx ) const;
  int set( lua_State * );
  int clear( lua_State * );
  int to_string( lua_State*) const;
  
  static int ArrayGet( lua_State *, const Message &, const FieldDescriptor*, int i );
  static void ArraySet( lua_State *, Message *, const FieldDescriptor*, int i );

private:
  static int PushProxy( lua_State*, const QpbRef&, const FieldDescriptor *);
  
  QpbArray(); // unimplemented
  QpbRef _msg;
  const FieldDescriptor *_field;  
};


#endif // #ifndef __QPB_ARRAY_H__
