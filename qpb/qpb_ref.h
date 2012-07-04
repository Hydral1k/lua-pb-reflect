/**
 * @file qpb_ref.h
 *
 * \internal
 * Copyright (c) 2012, everMany, LLC.
 * All rights reserved.
 * 
 * Code licensed under the "New BSD" (BSD 3-Clause) License
 * See License.txt for complete information.
 */
#pragma once
#ifndef __QPB_REF_H__
#define __QPB_REF_H__

#include "qpb_forwards.h"

/**
 * placeholder for referencing counting object
 * so that lua owned messages don't get arbitrarily deleted out under array proxies
 */
struct QpbRef {
  typedef google::protobuf::Message Message;
  QpbRef( Message* msg ) 
    : _message(msg)
    ,_mutation(QPB_MUTABLE) {
  }

  QpbRef( const Message& msg ) 
    : _message(&msg)
    ,_mutation(QPB_IMMUTABLE) {
  }
/*
  QpbRef( Message* msg, QpbMutation mute ) 
    : _message(msg)
    ,_mutation(mute) {
  }

  void set( Message* msg, QpbMutation mute ) {
    _message= msg;
    _mutation= mute;
  }
*/

  const Message * operator->() const {
    return _message;
  }
  operator const Message &() const {
    return *_message;
  }

  /**
   * if the object is mutable, then casts off the const.
   * if the object is immutable,  if the passed L is valid raises an error, otherwise returns NULL,
   */
  Message * demute( lua_State * L );

private:
  const Message* _message;
  QpbMutation _mutation;
};
#endif // #ifndef __QPB_REF_H__
