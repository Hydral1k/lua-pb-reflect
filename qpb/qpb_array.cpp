/**
 * @file qpb_array.cpp
 *
 * \internal
 * Copyright (c) 2012, everMany, LLC.
 * All rights reserved.
 * 
 * Code licensed under the "New BSD" (BSD 3-Clause) License
 * See License.txt for complete information.
 */
#include "qpb_array.h"
#include "qpb_message.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#include <string>

using namespace google::protobuf;
#include "qpb_convert.h"


//---------------------------------------------------------------------------
int QpbArray::PushProxy( lua_State* L, const QpbRef & msg, const FieldDescriptor *field )
{
  // lua 'throws' on failed allocation
  QpbArray* proxy= (QpbArray*)lua_newuserdata( L, sizeof(QpbArray) );
  luaL_getmetatable( L, QPB_ARRAY_METATABLE ); // fetch the object metatable
  lua_setmetatable( L, -2 ); // set the metatable of the user data
  proxy->_msg= msg;
  proxy->_field= field;
  return 1;
}

//---------------------------------------------------------------------------
QpbArray* QpbArray::GetUserData( lua_State * L, int idx ) 
{
  QpbArray* array= (QpbArray*) luaL_checkudata( L, idx, QPB_ARRAY_METATABLE );
  return array;
}

//---------------------------------------------------------------------------
int QpbArray::collect( lua_State * L ) 
{
  return 0;
}  

//---------------------------------------------------------------------------
int QpbArray::get( lua_State * L ) const
{
  int index = luaL_checkint(L, QPB_ARRAY_INDEX);
  return ArrayGet( L, _msg, _field, index );
}

//---------------------------------------------------------------------------
int QpbArray::set( lua_State * L )
{
  Message* msg= _msg.demute(L);
  if (msg) {
    int index = luaL_checkint(L, QPB_ARRAY_INDEX);
    lua_pushvalue( L, QPB_ARRAY_VALUE );
    ArraySet( L, msg, _field, index );
  }    
  return 0;
}

//---------------------------------------------------------------------------
int QpbArray::size( lua_State * L ) const
{
  const Reflection * reflect= _msg->GetReflection();
  int size= reflect->FieldSize( _msg, _field );
  lua_pushinteger( L, size );
  return 1;
}

//---------------------------------------------------------------------------
int QpbArray::clear( lua_State * L )
{
  Message* msg= _msg.demute(L);
  if (msg) {
    const Reflection * reflect= msg->GetReflection();
    reflect->ClearField(msg, _field);
  }
  return 0;
}

//---------------------------------------------------------------------------
int QpbArray::ArrayGet( lua_State *L, const Message & msg, const FieldDescriptor*field, int index )
{
  int ret=0;
  const Reflection * reflect= msg.GetReflection();
  const int size= reflect->FieldSize( msg, field );
  index-=1; // lua-to-c
  if (index <0 || index>=size) {
    QPB_ERR_RANGE( L, field->name().c_str(), index, size );
  }
  else {
    switch ( field->cpp_type() ) {
      case FieldDescriptor::CPPTYPE_INT32: {
        int32 val= reflect->GetRepeatedInt32( msg, field, index );
        ret= LUA_PUSH_INT32( L, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_INT64: {
        int64 val= reflect->GetRepeatedInt64( msg, field, index );
        ret= LUA_PUSH_INT64( L, val );
      }
      break;
      case FieldDescriptor::CPPTYPE_UINT32: {
        uint32 val= reflect->GetRepeatedUInt32( msg, field, index );
        ret= LUA_PUSH_UINT32( L, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_UINT64: {
        uint64 val= reflect->GetRepeatedUInt64( msg, field, index );
        ret= LUA_PUSH_UINT64( L, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_DOUBLE: {
        double val= reflect->GetRepeatedDouble( msg, field, index );
        ret= LUA_PUSH_DOUBLE( L, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_FLOAT: {
        float val= reflect->GetRepeatedFloat( msg, field, index );
        ret= LUA_PUSH_FLOAT( L, val );
      }
      break;  
      case FieldDescriptor::CPPTYPE_BOOL: {
        bool val= reflect->GetRepeatedBool( msg, field, index );
        ret= LUA_PUSH_BOOL( L, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_ENUM: {
        const EnumValueDescriptor* val= reflect->GetRepeatedEnum( msg, field, index );
        ret= LUA_PUSH_ENUM(L,val);
      }
      break;
      case FieldDescriptor::CPPTYPE_STRING: {
        std::string scratch;
        const std::string& val= reflect->GetRepeatedStringReference( msg, field, index, &scratch );
        ret= LUA_PUSH_STRING( L, val );
      }              
      break;
      case FieldDescriptor::CPPTYPE_MESSAGE: {
        const Message& val= reflect->GetRepeatedMessage( msg, field, index );
        ret= LUA_PUSH_MESSAGE( L, val );
      }
      break;
      default:
        QPB_ERR_TYPE( L, field->name().c_str() );
      break;
    }          
  }    
  return ret;
}

//---------------------------------------------------------------------------
/**
 * @param L       lua; data we are setting needs to be on top of the stack; it gets popped  
 * @param msg     protobuf that holds the array
 * @param field   member within the protbuf that represents the array
 * @param index   array element we're interested in
 */
void QpbArray::ArraySet( lua_State *L, Message * msg, const FieldDescriptor*field, int index )
{
  const Reflection * reflect= msg->GetReflection();
  const int size= reflect->FieldSize( *msg, field );
  index-=1; // lua-to-c
  if (index <0 || index>=size) {
    QPB_ERR_RANGE( L, field->name().c_str(), index, size );
  }
  else {
    switch ( field->cpp_type() ) {
      case FieldDescriptor::CPPTYPE_INT32:{
        const int32 val= LUA_TO_INT32(L,-1);
        reflect->SetRepeatedInt32( msg, field, index, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_INT64:{
        const int64 val= LUA_TO_INT64( L, -1 );
        reflect->SetRepeatedInt64( msg, field, index, val);
      }
      break;
      case FieldDescriptor::CPPTYPE_UINT32:{
        const uint32 val= LUA_TO_UINT32( L, -1 );
        reflect->SetRepeatedUInt32( msg, field, index, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_UINT64:{
        const uint64 val= LUA_TO_UINT64( L, -1 );
        reflect->SetRepeatedUInt64( msg, field, index, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_DOUBLE:{
        const double val= LUA_TO_DOUBLE( L, -1 );
        reflect->SetRepeatedDouble( msg, field, index, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_FLOAT:{
        const float val= LUA_TO_FLOAT( L, -1 );
        reflect->SetRepeatedFloat( msg, field, index, val );
      }
      break;  
      case FieldDescriptor::CPPTYPE_BOOL:{
        const bool val= LUA_TO_BOOL( L, -1 );
        reflect->SetRepeatedBool( msg, field, index, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_ENUM: {
        const EnumValueDescriptor* val= LUA_TO_ENUM( msg, field, L, -1 );
        reflect->SetRepeatedEnum( msg, field, index, val );
      }
      break;
      case FieldDescriptor::CPPTYPE_STRING: {
        const std::string & val= LUA_TO_STRING( L, -1 );
        reflect->SetRepeatedString( msg, field, index, val );
      }              
      break;
      case FieldDescriptor::CPPTYPE_MESSAGE: {
        Message* dst= reflect->MutableRepeatedMessage( msg, field, index );
        if (!dst) {
          QPB_ERR_MESSAGE(L, field->name().c_str() );
        }
        else {
          const Message& val= LUA_TO_MESSAGE( L, -1 );
          dst->CopyFrom( val );
        }                  
      }
      break;
      default:
        QPB_ERR_TYPE( L, field->name().c_str() );
      break;
    }
  }    
  lua_pop( L, -1 );
}
