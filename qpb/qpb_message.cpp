/**
 * @file qpb_message.cpp
 *
 * \internal
 * Copyright (c) 2012, everMany, LLC.
 * All rights reserved.
 * 
 * Code licensed under the "New BSD" (BSD 3-Clause) License
 * See License.txt for complete information.
 */
#include "qpb_message.h"
#include "qpb_array.h"

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
// QpbMessage
//---------------------------------------------------------------------------
// note we can't get a handle without a valid msg
// and every msg is supposed to have a valid descriptor
int QpbMessage::PushMsg( lua_State*L, const QpbRef& msg, bool owned ) 
{
  // lua 'throws' on failed allocation
  QpbMessage *handle= (QpbMessage *)lua_newuserdata( L, sizeof(QpbMessage) );
  luaL_getmetatable(L, QPB_MESSAGE_METATABLE ); // fetch the object metatable
  if (lua_type(L,-1)!= LUA_TTABLE) {
    QPB_ERR_TYPE(L, QPB_MESSAGE_METATABLE );
  }
  lua_setmetatable( L, -2 ); // set the metatable of the user data
  handle->_msg= msg;
  handle->_owned_by_lua= owned;
  return 1;
}

//---------------------------------------------------------------------------
QpbMessage* QpbMessage::GetUserData( lua_State * L, int idx ) 
{
  QpbMessage* handle= (QpbMessage*) luaL_checkudata( L, idx, QPB_MESSAGE_METATABLE );
  return handle;
}

//---------------------------------------------------------------------------
int QpbMessage::collect(lua_State* state)
{
  if (_owned_by_lua) {
    _owned_by_lua= false;
    delete _msg.demute(0);
  }    
  return 0;
}

//---------------------------------------------------------------------------
const FieldDescriptor* QpbMessage::field( lua_State* L, const char * name ) const
{
  const Descriptor*desc= _msg->GetDescriptor();
  const FieldDescriptor* field= desc->FindFieldByLowercaseName(name);
  if (!field) {
    QPB_ERR_FIELD(L, name);
  }
  return field;
}

//---------------------------------------------------------------------------
/**
 * a QpbMessage is getting pretty printed
 */
int QpbMessage::to_string( lua_State * L ) const
{
  const Descriptor* desc= _msg->GetDescriptor();
  if (!desc) {
      QPB_ERR_DESCRIPTOR( L );
  }
  else {
    lua_pushfstring(L,"qpb:(%d)%s", this, (const char*) desc->full_name().c_str() );
  }  
  return 1;
}

//---------------------------------------------------------------------------
int QpbMessage::has(lua_State*L, const FieldDescriptor* field) const
{
  const Reflection * reflect= _msg->GetReflection();
  const bool has= reflect->HasField( _msg, field );
  lua_pushboolean( L, has );
  return 1;
}

//---------------------------------------------------------------------------
int QpbMessage::size(lua_State*L, const FieldDescriptor* field) const
{
  if (!field->is_repeated()) {
    QPB_ERR_REPEATED_FIELD(L, field->name().c_str() );
  }
  else {
    const Reflection * reflect= _msg->GetReflection();
    int n= reflect->FieldSize( _msg, field );
    lua_pushinteger( L, n );
  }
  return 1;
}

//---------------------------------------------------------------------------
int QpbMessage::get(lua_State*L, const FieldDescriptor* field) const
{
  int ret=0;
  const Reflection * reflect= _msg->GetReflection();
    
  if (field->is_repeated()) {
    // repeated_fields can be accessed two ways through their bare name:
    // pb.repeated_field( index ); and, pb.repeated_field()
    // the second returns an immutable array
    // 
    if (lua_type(L, QPB_GET_REPEATED_INDEX) == LUA_TNONE) {
      ret= QpbArray::PushProxy( L, (const Message&) _msg, field );
    }
    else {
      const int index= lua_tointeger(L, QPB_GET_REPEATED_INDEX );
      ret= QpbArray::ArrayGet( L, _msg, field, index );
    }      
  }
  else {
    switch ( field->cpp_type() ) {
      case FieldDescriptor::CPPTYPE_INT32: {
        int32 val= reflect->GetInt32( _msg, field );
        ret= LUA_PUSH_INT32( L, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_INT64: {
        int64 val= reflect->GetInt64( _msg, field );
        ret= LUA_PUSH_INT64( L, val );
      }
      break;
      case FieldDescriptor::CPPTYPE_UINT32: {
        uint32 val= reflect->GetUInt32( _msg, field );
        ret= LUA_PUSH_UINT32( L, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_UINT64: {
        uint64 val= reflect->GetUInt64( _msg, field );
        ret= LUA_PUSH_UINT64( L, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_DOUBLE: {
        double val= reflect->GetDouble( _msg, field );
        ret= LUA_PUSH_DOUBLE( L, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_FLOAT: {
        float val= reflect->GetFloat( _msg, field );
        ret= LUA_PUSH_FLOAT( L, val );
      }
      break;  
      case FieldDescriptor::CPPTYPE_BOOL: {
        bool val= reflect->GetBool( _msg, field );
        ret= LUA_PUSH_BOOL( L, val );
      }
      break;              
      case FieldDescriptor::CPPTYPE_ENUM: {
        const EnumValueDescriptor* eval= reflect->GetEnum( _msg, field );
        ret= LUA_PUSH_ENUM( L, eval );
      }
      break;
      case FieldDescriptor::CPPTYPE_STRING: {
        std::string scratch;
        const std::string& str= reflect->GetStringReference( _msg, field, &scratch );
        ret= LUA_PUSH_STRING( L, str );
      }              
      break;
      case FieldDescriptor::CPPTYPE_MESSAGE: {
        const Message& val= reflect->GetMessage( _msg, field );
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
int QpbMessage::get_mutable(lua_State*L, const FieldDescriptor* field)
{
  int ret=0;
  if (field->type()==FieldDescriptor::TYPE_STRING) {
    QPB_ERR_MUTE_STRING( L, field->name().c_str() );
  }
  else
  if (field->type()==FieldDescriptor::TYPE_MESSAGE) {
    Message * msg= _msg.demute(L);
    if (msg) {
      const Reflection * reflect= msg->GetReflection();
      Message * src= reflect->MutableMessage( msg, field );
      ret= LUA_PUSH_MESSAGE( L, src );
    }      
  }
  else 
  if (field->is_repeated()) {
    Message* msg= _msg.demute(L);
    if (msg) {
      ret= QpbArray::PushProxy(L, msg, field );
    }      
  }
  else {
    QPB_ERR_MUTABLE(L, field->name().c_str());
  }
  return ret;
}

//---------------------------------------------------------------------------
int QpbMessage::set(lua_State*L, const FieldDescriptor* field)
{
  Message * msg= _msg.demute(L);
  if (msg) {
    if (field->is_repeated()) {
      luaL_checknumber( L, QPB_SET_REPEATED_INDEX );
      const int index= lua_tointeger(L, QPB_SET_REPEATED_INDEX );
      lua_pushvalue( L, QPB_SET_REPEATED_VALUE ); // tge 
      QpbArray::ArraySet( L, msg, field, index );
    }
    else {
      const Reflection * reflect= msg->GetReflection();
      // ex. set_foo( int32 value )  
      switch ( field->cpp_type() ) {
        case FieldDescriptor::CPPTYPE_INT32: {
          const int32 val= LUA_TO_INT32( L, QPB_SET_VALUE );
          reflect->SetInt32( msg, field, val );
        }
        break;              
        case FieldDescriptor::CPPTYPE_INT64: {
          const int64 val= LUA_TO_INT64( L, QPB_SET_VALUE );
          reflect->SetInt64( msg, field,val );
        }
        break;
        case FieldDescriptor::CPPTYPE_UINT32: {
          const uint32 val= LUA_TO_UINT32( L, QPB_SET_VALUE );
          reflect->SetUInt32( msg, field, val );
        }
        break;              
        case FieldDescriptor::CPPTYPE_UINT64: {
          const uint64 val= LUA_TO_UINT64( L, QPB_SET_VALUE );
          reflect->SetUInt64( msg, field, val );
        }
        break;              
        case FieldDescriptor::CPPTYPE_DOUBLE: {
          const double val= LUA_TO_DOUBLE( L, QPB_SET_VALUE );
          reflect->SetDouble( msg, field, val );
        }
        break;              
        case FieldDescriptor::CPPTYPE_FLOAT: {
          const float val= LUA_TO_FLOAT( L, QPB_SET_VALUE );
          reflect->SetFloat( msg, field, val );
        }
        break;  
        case FieldDescriptor::CPPTYPE_BOOL: {
          const bool val= LUA_TO_BOOL( L, QPB_SET_VALUE );
          reflect->SetBool( msg, field, val );
        }
        break;              
        case FieldDescriptor::CPPTYPE_ENUM: {
          const EnumValueDescriptor* val= LUA_TO_ENUM( msg, field, L, QPB_SET_VALUE );
          reflect->SetEnum( msg, field, val );
        }
        break;
        case FieldDescriptor::CPPTYPE_STRING: {
          const std::string & val= LUA_TO_STRING( L, QPB_SET_VALUE );
          reflect->SetString( msg, field, val );
        }              
        break;
        case FieldDescriptor::CPPTYPE_MESSAGE: {
          Message * dst= reflect->MutableMessage( msg, field );
          if (!dst) {
            QPB_ERR_MESSAGE( L, field->name().c_str() );
          }
          else {
            const Message &val= LUA_TO_MESSAGE( L, QPB_SET_VALUE );
            dst->CopyFrom( val );
          }                  
        }
        break;
        default:
          QPB_ERR_TYPE( L, field->name().c_str() );
        break;
      }
    }      
  }    
  return 0;
}

//---------------------------------------------------------------------------
int QpbMessage::add(lua_State*L, const FieldDescriptor* field)
{
  int ret=0;
  if (!field->is_repeated()) {
    QPB_ERR_REPEATED_FIELD(L,field->name().c_str() ); 
  }
  else {
    Message * msg= _msg.demute(L);
    if (msg) {
      const Reflection * reflect= msg->GetReflection();
      switch ( field->cpp_type() ) {
        case FieldDescriptor::CPPTYPE_INT32: {
          const int32 val= LUA_TO_INT32(L,QPB_APPEND_VALUE);
          reflect->AddInt32( msg, field, val );
        }
        break;              
        case FieldDescriptor::CPPTYPE_INT64: {
          const int64 val= LUA_TO_INT64( L, QPB_APPEND_VALUE );
          reflect->AddInt64( msg, field, val );
        }
        break;
        case FieldDescriptor::CPPTYPE_UINT32: {
          const uint32 val= LUA_TO_UINT32( L, QPB_APPEND_VALUE );
          reflect->AddUInt32( msg, field, val );
        }
        break;              
        case FieldDescriptor::CPPTYPE_UINT64: {
          const uint64 val= LUA_TO_UINT64( L, QPB_APPEND_VALUE );
          reflect->AddUInt64( msg, field, val );
        }
        break;              
        case FieldDescriptor::CPPTYPE_DOUBLE: {
          const double val= LUA_TO_DOUBLE( L, QPB_APPEND_VALUE );
          reflect->AddDouble( msg, field, val );
        }
        break;              
        case FieldDescriptor::CPPTYPE_FLOAT: {
          const float val= LUA_TO_FLOAT( L, QPB_APPEND_VALUE );
          reflect->AddFloat( msg, field, val );
        }
        break;  
        case FieldDescriptor::CPPTYPE_BOOL: {
          const bool val= LUA_TO_BOOL( L, QPB_APPEND_VALUE );
          reflect->AddBool( msg, field, val );
        }
        break;              
        case FieldDescriptor::CPPTYPE_ENUM: {
          const EnumValueDescriptor* val= LUA_TO_ENUM( msg, field, L, QPB_APPEND_VALUE );
          reflect->AddEnum( msg, field, val );
        }
        break;
        case FieldDescriptor::CPPTYPE_STRING: {
          const std::string & val= LUA_TO_STRING( L, QPB_APPEND_VALUE );
          if (lua_type(L, QPB_APPEND_VALUE ) == LUA_TNONE) {
            QPB_ERR_MUTE_STRING( L, field->name().c_str() );
          }
          else {
            reflect->AddString( msg, field, val );
          }            
        }              
        break;
        case FieldDescriptor::CPPTYPE_MESSAGE: {
          if (lua_type(L, QPB_APPEND_VALUE ) != LUA_TNONE) {
            QPB_ERR_ADD_MESSAGE( L, field->name().c_str() );  
          }
          else {
            Message * newmsg= reflect->AddMessage( msg, field );
            LUA_PUSH_MESSAGE( L, newmsg );
            ret= 1;
          }
        }
        break;
        default:
          QPB_ERR_TYPE( L, field->name().c_str() );
        break;
      }
    }      
  }      
  return ret;
}

//---------------------------------------------------------------------------
int QpbMessage::clear( lua_State * L, const FieldDescriptor* field ) 
{
  Message * msg= _msg.demute(L);
  if (msg) {
    const Reflection * reflect= msg->GetReflection();
    reflect->ClearField( msg, field );
  }    
  return 0;
}

//---------------------------------------------------------------------------
// Releases the ownership of the field and returns the old value 
// ( valid only for strings and messages )
int QpbMessage::release(lua_State*L, const FieldDescriptor* field)
{
  int ret=0;
  if (field->type()==FieldDescriptor::TYPE_STRING) {
    if (_msg.demute(L)) {
      ret= get( L,field );
      clear( L,field );
    }      
  }
  else 
  if (field->type()==FieldDescriptor::TYPE_MESSAGE) {
    Message * msg= _msg.demute(L);
    if (msg) {
      const Reflection * reflect= msg->GetReflection();
      const Message& val= reflect->GetMessage( _msg, field );
      //
      Message* dst= val.New();
      if (!dst) {
        QPB_ERR_ALLOC(L);
      }
      else {
        dst->CopyFrom(val);
        reflect->ClearField( msg, field );
        ret= LUA_PUSH_MESSAGE( L, dst );
      }        
    }      
  }          
  else {
    QPB_ERR_RELEASE( L, field->name().c_str() );
  }
  return ret;    
}
