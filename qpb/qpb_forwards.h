/**
 * @file qpb_forwards.h
 *
 * \internal
 * Copyright (c) 2012, everMany, LLC.
 * All rights reserved.
 * 
 * Code licensed under the "New BSD" (BSD 3-Clause) License
 * See License.txt for complete information.
 */
#pragma once
#ifndef __QPB_FORWARDS_H__
#define __QPB_FORWARDS_H__

typedef struct lua_State lua_State;
namespace google {
  namespace protobuf {
    class Message;
    class MessageFactory;
    class Descriptor;
    class FieldDescriptor;
  }
};

#define QPB_GLOBAL_LIBARAY    "QPB"
#define QPB_MESSAGE_METATABLE "qpb.proto.buffer.message"
#define QPB_ARRAY_METATABLE   "qpb.proto.buffer.array"

enum QpbMutation {
  QPB_IMMUTABLE,
  QPB_MUTABLE,
};


// all of the permissible parameters indicies
// to the various qpb functions
enum QpbParameters 
{
  QPB_CLASS_UPVALUE = 1,
  QPB_FIELD_UPVALUE = 2,  // index sets an upvalue of the fieldname 

  // qpb global object:
  QPB_NEW_PBNAME =1, // pb= qpb.new( pbname )

  // pb message userdata:
  // __index for unknown fields:
  QPB_META_TABLE=1, // pb.fieldname => pb.__index( metatable, fieldname )
  QPB_META_FIELD=2,

  // pb message __index returns a closure to handle requests for data from the fieldname
  // ex. pb:repeated_field( 5 ) is a request for the 5 value in an array
  // index returns pb.repeated_field returns the closure, 
  // (pb, 5) gets sent to the qpb_msg_parse_closure.
  QPB_MESSAGE_SELF=1,        // pb:....
  QPB_SET_REPEATED_INDEX=2,  // pb:set_field( index, value )
  QPB_SET_REPEATED_VALUE=3,
  QPB_SET_VALUE=2,           // pb:set_field( value )
  QPB_APPEND_VALUE=2,        // pb:add_field( value ); 
  QPB_GET_REPEATED_INDEX=2,  // pb:get_field( index )

  // pb array proxy:
  QPB_ARRAY_SELF =1,         // array:
  QPB_ARRAY_INDEX=2,         // array[index], array:get(index)
  QPB_ARRAY_VALUE=3,         // array[index]= value, array:set(index, value)

  // qpb array iteration
  QPB_NEXT_INVARIENT=1,
  QPB_NEXT_CONTROL  =2,

  // qpb array ipairs
  QPB_ARRAY_IPAIRS=1,

  // qpb message index lookup
  QPB_MESSAGE_INDEX=1,
};

#define QPB_ERR_ALLOC(L)    luaL_error( L, "QPB: couldn't allocate memory.")
#define QPB_ERR_DESCRIPTOR(L)    luaL_error( L, "QPB: invalid descriptor in pb.")
#define QPB_ERR_IMMUTABLE(L) luaL_error( L, "QPB: trying to change an immutable object.")

#define QPB_ERR_TYPE( L, name ) luaL_error( L, "QPB: unknown type %s", (const char*) name );
#define QPB_ERR_MESSAGE( L, name ) luaL_error( L, "QPB: invalid message %s", (const char*) name );
#define QPB_ERR_FIELD( L, name ) luaL_error( L, "QPB: unknown field requested %s", (const char*) (name) );
#define QPB_ERR_REPEATED_FIELD( L, name ) luaL_error( L, "QPB: field %s not a repeated field", (const char*) name )
#define QPB_ERR_FIELD_ENUM( L, field, ename ) luaL_error( L, "QPB: enum name %s invalid for field %s", (const char*) ename, (const char*) field )
#define QPB_ERR_ADD_MESSAGE( L, name ) luaL_error( L, "QPB: add message returns a new message, it doesnt append one. for field: %s", (const char*) (name) );
#define QPB_ERR_RELEASE(L, name) luaL_error( L, "QPB: invalid release request for field %s", (const char*) (name) );
#define QPB_ERR_MUTABLE(L, name) luaL_error( L, "QPB: invalid mutable request for field %s", (const char*) (name) );
#define QPB_ERR_RANGE(L, name, i, size ) luaL_error( L, "QPB: %d out of range %d for field %s", i, size, (const char*) (name) );

// protobuf defines string* msg:add_string(), string* mutable_string()
// and allows the user to mutate the string contents; 
// qpb would need a string proxy like the array proxy to support that.
#define QPB_ERR_MUTE_STRING( L, name )  luaL_error( L, "QPB: mutable strings not supported: %s", (const char*) (name) );

#endif // #ifndef __QPB_FORWARDS_H__
