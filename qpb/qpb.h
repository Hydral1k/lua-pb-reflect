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
  /**
   * @param L lua state to register with.
   */
  Qpb(lua_State*L);

  /**
   * Register a series of the protobuf types with lua.
   *
   * @param descs array of descriptors ( via userMessageType.descriptor() )
   * @param count number of descriptors in array
   *
   * @return count of ambiguous shortnames; the last shortname registered wins 
   */
  int register_descriptors( const Descriptor **descs, int count ) {
    return register_descriptors( QPB_GLOBAL_LIBARAY, descs, count );
  }

  int register_descriptors( const char * name, const Descriptor **descs, int count );

  int alloc() const;
  int parse_closure() const;
  static Qpb* GetUpValue( lua_State * L );

protected:
  lua_State*L;
  
  int register_recurse( const Descriptor *desc );
  typedef google::protobuf::Message Message;
  typedef google::protobuf::MessageFactory MessageFactory;

private:
  MessageFactory* _factory; // booost scoped 
  typedef std::map<std::string, const Descriptor*> descriptor_map;
  descriptor_map _fullnames, _shortnames;
};


#endif // #ifndef __QPB_H__
