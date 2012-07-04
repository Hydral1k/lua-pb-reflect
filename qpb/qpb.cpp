/**
 * @file qpb.cpp
 *
 * \internal
 * Copyright (c) 2012, everMany, LLC.
 * All rights reserved.
 * 
 * Code licensed under the "New BSD" (BSD 3-Clause) License
 * See License.txt for complete information.
 */
#include "qpb.h"
#include <assert.h>
#include "qpb_array.h"
#include "qpb_message.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/dynamic_message.h>


using namespace google::protobuf;

//---------------------------------------------------------------------------
#define MATCH(key, a,b,c) (key[0] ==a && key[1] ==b && key[2] ==c && key[3] =='_')

//---------------------------------------------------------------------------
static int qpb_array_collect( lua_State * L ) 
{
  QpbArray * array= QpbArray::GetUserData(L);
  return array->collect(L);
}

//---------------------------------------------------------------------------
static int qpb_array_set( lua_State * L ) { 
  QpbArray * array= QpbArray::GetUserData(L);
  return array->set(L);
}

//---------------------------------------------------------------------------
static int qpb_array_get( lua_State * L ){ 
  QpbArray * array= QpbArray::GetUserData(L);
  return array->get( L );
}

//---------------------------------------------------------------------------
static int qpb_array_size( lua_State * L ) { 
  QpbArray * array= QpbArray::GetUserData(L);
  return array->size( L );
}

//---------------------------------------------------------------------------
// two possibilities when array is userdata:
// array[i]= value is metatable(array)->__index( array, valid )
// a:func( params ) is sugar for a.func( a, params ) is sugar for a[func]( a,params )
// when func doesnt exist -> metatable(a)->__index( a, func ) ( a,params )
static int qpb_array_index( lua_State * L ) 
{
  QpbArray * array= QpbArray::GetUserData(L);
  // if its a number ( not simply convertible )
  // treat it like a normal indexed get
  if (lua_type(L,2)==LUA_TNUMBER) {
    array->get(L);
  }
  else 
  // if its a string key ( not simply convertible )
  // look up in the metatable the named function
  if (lua_type(L,2)==LUA_TSTRING) {
    lua_getmetatable( L,1 ); // we know its the right table or we wouldnt be here
    lua_pushvalue( L,2 ),lua_gettable( L,1 );  // push table[key], 
    lua_remove( L, -2 ); // remove the metatable, leaving whatever we had, func or nil
  }
  return 1;
}


//---------------------------------------------------------------------------
// qpb global type
//---------------------------------------------------------------------------

// pb= qpb.new( name );
static int qpb_alloc( lua_State * L ) {
  Qpb*qpb= Qpb::GetUpValue(L);
  return qpb->alloc(L);
}

//---------------------------------------------------------------------------
// qpb message user data
//---------------------------------------------------------------------------

// delete pb
static int qpb_msg_collect( lua_State * L ){
  QpbMessage* msg= QpbMessage::GetUserData(L);
  return msg->collect(L);
}
// print( pb )
static int qpb_msg_to_string( lua_State * L ) {
  QpbMessage* msg= QpbMessage::GetUserData(L);
  return msg->to_string(L);
}

// pb:index
static int qpb_parse_closure( lua_State * L ) {
  Qpb*qpb= Qpb::GetUpValue(L);
  return qpb->parse_closure(L);
}

// pb.unknown_field
static int qpb_msg_index( lua_State * L ) 
{
  Qpb*qpb= Qpb::GetUpValue(L);
  lua_pushlightuserdata( L, qpb );  // prepares the QPB_CLASS_UPVALUE
  lua_pushvalue( L, QPB_META_FIELD ); // prepares the QPB_FIELD_UPVALUE
  lua_pushcclosure( L, qpb_parse_closure, 2 ); // closure with 2 upvalues, pops values.
  return 1;
}

//---------------------------------------------------------------------------
// Qpb
//---------------------------------------------------------------------------
Qpb::~Qpb() 
{
  if (_factory) {
    delete _factory;
  }
}
//---------------------------------------------------------------------------
Qpb::Qpb() 
  : _factory(0)
{
  
}
//---------------------------------------------------------------------------
Qpb * Qpb::GetUpValue( lua_State * L ) 
{
  void * qpb= lua_touserdata( L, lua_upvalueindex( QPB_CLASS_UPVALUE )  );
  return (Qpb*) qpb;
}

//---------------------------------------------------------------------------
/**
 * allocate a new QpbMessage as userdata, return it to the user
 */
int Qpb::alloc( lua_State * L ) const
{
  int ret=0;
  
  // name of pb type
  const char * name= luaL_checkstring( L, QPB_NEW_PBNAME );

  // find the type
  const Descriptor* desc= 0;
  descriptor_map::const_iterator it= _fullnames.find( name );
  if (it != _fullnames.end()) {
      desc= it->second;
  }
  else {
    it= _shortnames.find( name );
    if (it != _shortnames.end()) {
      desc= it->second;
    }
  }

  if (!desc) {
      lua_pop( L, -1 );
      QPB_ERR_TYPE(L,name);
  }
  else {
  // create an instance of the type type
    const Message* prototype= _factory->GetPrototype( desc );
    Message * msg= prototype ? prototype->New(): 0;
    if (!msg) {
      lua_pop( L, -1 );
      QPB_ERR_ALLOC(L);
    }
    else {
      // return it.
      ret= QpbMessage::PushMsg( L, msg, true );
    }      
  }      

  return ret;
}

//---------------------------------------------------------------------------
// the unknown field access always returns this function
// we then expect user data as the first parameter
// second parameter depends on the operation
// set takes a value...

#define parselen( x ) (sizeof(x)/sizeof(*x)-1)
static const char parse_clear[]= "clear_";
static const char parse_size[]= "_size";
static const char parse_mute[]="mutable_";
static const char parse_release[]="release_";


// https://developers.google.com/protocol-buffers/docs/reference/cpp-generated#message
int Qpb::parse_closure( lua_State * L ) const
{
  int ret=0;
  QpbMessage* handle= QpbMessage::GetUserData(L);
  if (handle) {
    size_t keylen;
    const char * key= luaL_checklstring( L, lua_upvalueindex( QPB_FIELD_UPVALUE ), &keylen );
    if (key) {
      const int _sizetail=keylen-parselen(parse_size);
    /**
     * TODO: 
     *      enum listings via Foo_descriptor, and Foo_IsValid(int)
     */
      if (MATCH(key, 'h','a','s')){
        const FieldDescriptor*field= handle->field( L, key+4 );
        ret= handle->has( L, field );
      }
      else
      if (MATCH(key, 's','e','t')){
        const FieldDescriptor*field= handle->field( L, key+4 );
        ret= handle->set( L,  field ); 
      }
      else
      if (MATCH(key, 'a','d','d')){
        const FieldDescriptor*field= handle->field( L, key+4 );
        ret= handle->add( L,  field );
      }
      else
      if (_sizetail>0 && strcmp(key+_sizetail, "_size")==0) {
        char* name_nosize= (char*) alloca( _sizetail+1 );
        if (!name_nosize) {
          QPB_ERR_ALLOC( L );
        }
        else {
          strncpy( name_nosize, key, _sizetail );
          name_nosize[_sizetail]=0;
          const FieldDescriptor*field= handle->field( L, name_nosize );
          ret= handle->size( L, field );
        }              
      }
      else
      if (strncmp(key, parse_clear, parselen(parse_clear))==0) {
        const FieldDescriptor*field= handle->field( L, key+parselen(parse_clear) );
        ret= handle->clear( L, field );
      }
      else
      if (strncmp(key, parse_mute, parselen(parse_mute))==0) {
        const FieldDescriptor*field= handle->field( L, key+parselen(parse_mute) );
        ret= handle->get_mutable( L, field );
      }
      else
      if (strncmp(key, parse_release, parselen(parse_release))==0) {
        const FieldDescriptor*field= handle->field( L, key+ parselen(parse_release) );
        ret= handle->release( L, field );
      }      
      // plain name, could be: a value get, an array proxy get, or array index value set.
      else {
        const FieldDescriptor*field= handle->field( L, key );
        ret= handle->get(L, field);
      }
    }        
  }      
  return ret;
}    

//---------------------------------------------------------------------------
// register the descriptor, and its contents
int Qpb::register_recurse( const Descriptor *desc )
{
  int ambiguous_names=0;
  
  const std::string & fullname= desc->full_name();
  if (_fullnames.find( fullname ) == _fullnames.end()) {
    _fullnames[ fullname ]= desc;
    const std::string  &shortname= desc->name();
    if (_shortnames.find( shortname ) != _shortnames.end()) {
      ++ambiguous_names;
    }
    _shortnames[ shortname ]= desc;
    for (int i=0;i< desc->field_count(); ++i) {
      const FieldDescriptor* field= desc->field(i);
      assert( field );
      if (field->type()==FieldDescriptor::TYPE_MESSAGE) {
        const Descriptor* fieldtype= field->message_type();
        assert( fieldtype );
        ambiguous_names+= register_recurse( fieldtype );
      }
    }
  }      
  return ambiguous_names;
}

//---------------------------------------------------------------------------
// register functions into a mettable with an upvalue
static void qpb_register( lua_State* L, const char * table, luaL_Reg*reglist, void * up)
{
  if (luaL_newmetatable(L, table )) {
    const int metatable=lua_gettop(L);
    for (luaL_Reg*reg=reglist;reg->name;++reg) {
      lua_pushstring(L, reg->name );
      if (up) {
        lua_pushlightuserdata( L, up );  // prepare upvalue
      }        
      lua_pushcclosure( L, reg->func, up ? 1 : 0 );
      lua_settable( L, metatable );
    }
    lua_pop(L,1);
  }    
}

//---------------------------------------------------------------------------
// c++ public main method
//---------------------------------------------------------------------------
int Qpb::register_descriptors( const char * name, lua_State* L , const Descriptor **descs,int count )
{
  int ambiguous_names=0;

  // register the descriptions
  for (int i=0; i< count;++i) {
    const Descriptor * desc= descs[i];
    ambiguous_names+=register_recurse( desc );
  }     

  // create the message factory and the metatables
  // we only need to do this once.
  if (!_factory) {
    _factory= new DynamicMessageFactory();
    if (_factory) {

      // create the library type
      static luaL_Reg qpb_class_fun[] = {
        { "new", qpb_alloc },
        { 0 }
      };
      lua_pushlightuserdata( L, this );  // prepare QPB_CLASS_UPVALUE
      luaI_openlib( L, name, qpb_class_fun, 1 );   // create a table in the registry @ 'type' with the passed named c-functions
      lua_pop( L, 1 ); // openlib removes upvalues, but returns result
      
      // create the qpb message type
      static luaL_Reg qpb_member_fun[]= {
        { "__gc", qpb_msg_collect },
        { "__tostring", qpb_msg_to_string },
        { "__index", qpb_msg_index },
        { 0 }
      };
      qpb_register( L, QPB_MESSAGE_METATABLE, qpb_member_fun, this);
      
      // create the array proxy type
      static luaL_Reg qpb_array_fun[]= {
        { "__gc", qpb_array_collect },
        { "__index", qpb_array_index }, // a[i]= 5, a:set(i,5)
        { "__newindex", qpb_array_set  },
        { "__len", qpb_array_size   },
        // TODO: { "__tostring", qpb_array_to_string }, 
        { "set", qpb_array_set }, // a:set -> no. b/c these dont exist on a,
        { "get", qpb_array_get }, // they exist on 
        { "size", qpb_array_size },
        { 0 }
      };
      qpb_register( L, QPB_ARRAY_METATABLE, qpb_member_fun, 0);
    }      
  }

  return ambiguous_names;
}

