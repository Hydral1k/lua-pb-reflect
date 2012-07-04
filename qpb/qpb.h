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
#ifndef __QPB_H__
#define __QPB_H__

#include <string>
#include <map>

#include "qpb_forwards.h"

struct QpbMessage;

class Qpb {
public:
  typedef google::protobuf::Descriptor Descriptor;
  
  ~Qpb();
  Qpb();
  static Qpb* GetUpValue( lua_State * L );

  /**
   * Register a series of the protobuf types with lua.
   *
   * @param L lua state to register with.
   * @param descs Null terminated array of descriptors ( via userMessageType.descriptor() )
   *
   * @return count of ambiguous shortnames; the last shortname registered wins 
   */
  int register_descriptors( lua_State*L, const Descriptor **descs, int count ) {
    return register_descriptors( QPB_GLOBAL_LIBARAY, L, descs, count );
  }

  int register_descriptors( const char * name, lua_State* L , const Descriptor **descs, int count );

  int alloc( lua_State * L ) const;
  int collect( lua_State * L ) const;
  int to_string( lua_State * L ) const;
  int parse_closure( lua_State * L ) const;

private:
  int register_recurse( const Descriptor *desc );
  typedef google::protobuf::Message Message;
  typedef google::protobuf::MessageFactory MessageFactory;

  MessageFactory* _factory; // booost scoped 
  typedef std::map<std::string, const Descriptor*> descriptor_map;
  descriptor_map _fullnames, _shortnames;
};


#endif // #ifndef __QPB_H__
