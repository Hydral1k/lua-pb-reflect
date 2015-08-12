/**
 * @file qpb_convert.h
 *
 * \internal
 * Copyright (c) 2012, everMany, LLC.
 * All rights reserved.
 * 
 * Code licensed under the "New BSD" (BSD 3-Clause) License
 * See License.txt for complete information.
 */
#pragma once
#ifndef __QPB_CONVERT_H__
#define __QPB_CONVERT_H__

//#include "qpb_message.h"
//#include <google/protobuf/descriptor.h>
//#include <google/protobuf/message.h>
//#include <lua.h>
//#include <string>
// using namespace google::protobuf;

//---------------------------------------------------------------------------
/**
 * conversions from lua to protobuf types
 *
 * i played with the idea of using printf and scanf for 64bit numbers
 * ex. lua_pushfstring( L, "%"PRId64, val )
 * but since lua internally handles numbers as doubles 
 * it seems like its better not to convert to string
 */
#define LUA_TO_INT32( L, idx )  lua_tointeger(L,idx)

#define LUA_TO_INT64( L, idx ) ((int64) lua_tonumber( L, idx ))

#define LUA_TO_UINT32( L, idx ) ((uint32) lua_tointeger(L,idx))

#define LUA_TO_UINT64( L, idx ) ((uint64) lua_tonumber( L, idx ))

#define LUA_TO_DOUBLE( L, idx ) lua_tonumber(L,idx)

#define LUA_TO_FLOAT( L, idx )  ((float)lua_tonumber(L,idx))

#define LUA_TO_BOOL( L, idx )   lua_toboolean(L,idx)!=0

inline const EnumValueDescriptor * LUA_TO_ENUM( const Message*msg, const FieldDescriptor*field, lua_State* L, int idx ){
  const char * ename= lua_tostring(L,idx);
  const EnumValueDescriptor * eval= msg->GetDescriptor()->FindEnumValueByName( ename );
  if (!eval) {
    QPB_ERR_FIELD_ENUM( L, field->name().c_str(), ename );
  }
  return eval;
}

inline std::string LUA_TO_STRING( lua_State* L, int idx ) {
  size_t len=0;
  const char * str= lua_tolstring(L, idx, &len);
  return std::string( str, len );
}

inline const Message & LUA_TO_MESSAGE( lua_State * L, int idx ) {
  const QpbMessage *handle= QpbMessage::GetUserData( L, idx );
  return handle->GetMessage();
}  

//---------------------------------------------------------------------------
/**
 * conversions to lua from protobuf types
 */
inline int LUA_PUSH_INT32( lua_State* L, int32 val ) {
  lua_pushinteger( L, val );
  return 1;
}  

inline int LUA_PUSH_INT64( lua_State* L, int64 val ) {
  lua_pushnumber( L, (lua_Number)val );
  return 1;
}

inline int LUA_PUSH_UINT32( lua_State* L, uint32 val ) {
  lua_pushinteger( L, val );
  return 1;
}

inline int LUA_PUSH_UINT64( lua_State* L, uint64 val ) {
  lua_pushnumber( L, (lua_Number)val );
  return 1;
}

inline int LUA_PUSH_DOUBLE( lua_State* L, double val ) {
  lua_pushnumber( L, val );
  return 1;
}

inline int LUA_PUSH_FLOAT( lua_State* L, float val ) {
  lua_pushnumber( L, val );
  return 1;
}

inline int LUA_PUSH_BOOL( lua_State* L, bool val ) {
  lua_pushboolean( L, val );
  return 1;
}  

inline int LUA_PUSH_ENUM( lua_State* L, const EnumValueDescriptor* eval ) {
  if (eval) {
    lua_pushstring( L, eval->name().c_str() );
  } else {
    lua_pushnil(L);
  }
  return 1;    
}  

inline int LUA_PUSH_STRING( lua_State* L, const std::string & str ) {
  lua_pushlstring( L,str.c_str(), str.size() );
  return 1;
}  

inline int LUA_PUSH_MESSAGE( lua_State * L, const Message & msg, int owner ) {
  return QpbMessage::PushMsg( L, QpbRef(msg), owner );
}

inline int LUA_PUSH_MESSAGE( lua_State * L, Message * msg, int owner ) {
  // passes false b/c the message is assumed owned by some other pb.
  return QpbMessage::PushMsg( L, QpbRef(msg), owner  );
}

#endif // #ifndef __QPB_CONVERT_H__
