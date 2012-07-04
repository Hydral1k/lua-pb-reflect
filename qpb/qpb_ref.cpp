#include "qpb_ref.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

using namespace google::protobuf;

Message * QpbRef::demute(lua_State* L) 
{
  Message * ret=0;
  if (_mutation==QPB_IMMUTABLE) {
    if (L) { 
      QPB_ERR_IMMUTABLE( L );
    }      
  }
  else {
      ret= const_cast<Message*>(this->_message);
  }
  return ret;
}
