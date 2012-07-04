/**
 * @file qpb_message.h
 *
 * \internal
 * Copyright (c) 2012, everMany, LLC.
 * All rights reserved.
 * 
 * Code licensed under the "New BSD" (BSD 3-Clause) License
 * See License.txt for complete information.
 */
#pragma once
#ifndef __QPB_MESSAGE_H__
#define __QPB_MESSAGE_H__

#include "qpb_forwards.h"
#include "qpb_ref.h"

//---------------------------------------------------------------------------
/**
 * POS-like type managed by lua, we are a proxy to control how garbage colleciton works
 */
struct QpbMessage
{
  typedef google::protobuf::Message Message;
  typedef google::protobuf::Descriptor Descriptor;
  typedef google::protobuf::FieldDescriptor FieldDescriptor;

  static int PushMsg(lua_State*, const QpbRef& msg, bool owned=false);
  static QpbMessage* GetUserData( lua_State *, int idx= QPB_MESSAGE_SELF );

  int collect(lua_State* L);
  int to_string(lua_State*L) const;

  const FieldDescriptor* field( lua_State*L, const char * name ) const;
  int has(lua_State*L, const FieldDescriptor* field) const;
  int size(lua_State*L, const FieldDescriptor* field) const;
  int get(lua_State*L, const FieldDescriptor* field) const;
  int get_mutable(lua_State*L, const FieldDescriptor* field);
  int set(lua_State*L, const FieldDescriptor* field);
  int add(lua_State*L, const FieldDescriptor* field);
  int clear( lua_State * L, const FieldDescriptor* field );
  int release(lua_State*L, const FieldDescriptor* field );

  const Message& GetMessage() const {
    return _msg;
  }

private:  
  QpbRef _msg;
  bool _owned_by_lua;
  QpbMessage(); // unimplemented
};

#endif // #ifndef __QPB_MESSAGE_H__
