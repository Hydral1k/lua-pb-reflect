# Tutorial / Introduction
Using the example from [google documents Protobuf](http://code.google.com/p/protobuf/ "Protobuf")

```
message Person {
  required int32 id = 1;
  required string name = 2;
  optional string email = 3;
}
```

Just like normal, you use protoc to generate C++ classes, that you can use as follows:

```
Person person;
person.set_id(123);
person.set_name("Bob");
person.set_email("bob@example.com");

fstream out("person.pb", ios::out | ios::binary | ios::trunc);
person.SerializeToOstream(&out);
out.close();
```
Assuming you're embedding Lua into your c++ app, once you have a lua_State, all you need to do is:

```
// "QPB" is the name of the lua-pb-reflect class(es)
#include <qpb\qpb.h>

// note: the main QPB object must live longer than the garbage collection of your lua data.
// it can be a static, a new'd singleton, or on the stack in main()
Qpb qpb; 

// later...
// register your protobuffers with qpb.
// registration works recursively, 
// so if Person had fields with other message types,
// those message types are taken care of by this one statement.
// ( L is a lua_State* )
qpb.register_descriptors( L, &Person::descriptor(), 1 );
```
Now, in lua:
```
local person= QPB.new('Person')
person:set_id(123)
person:set_name("Bob")
person:set_email("bob@example.com")
```
All field accessors, array lookups etc, automagically work. Access exactly follows the patterns setup on https://developers.google.com/protocol-buffers/docs/reference/cpp-generated#message, with one exception.


# Note
To compile you need the protobuffer code and an environment variable 
"GOOGLE_DEV" pointing to the google platform build tree.

ex. if GOOGLE_DEV = C:\dev\protobuf-2.4.1\vsprojects
then the qpb.vcproject include directories specifies: 
$(GOOGLE_DEV)\include
