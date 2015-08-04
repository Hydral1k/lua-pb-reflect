This is as list of known incompatibilities between the QPB ( aka `lua-pb-reflect` ) code and the C++ code.

## 64 bit numbers ##
Lua doesn't natively support big numbers. I briefly thought about using strings, but then most major math operations wouldn't work. Instead, `QPB` uses doubles when transferring data in and out of 64 bit fields.

## Mutable Strings ##
There are two methods in the standard [C++ generated code](https://developers.google.com/protocol-buffers/docs/reference/cpp-generated#messagebuffers/docs/reference/cpp-generated#message) which return mutable strings.
QPB **does not** support these methods.

```
// gains access to the memory of a string field
// *does not work in lua with QPB*
xavier= msg.mutable_string_field()
xavier= "alters the msg memory."

// appends a string to the array_of_strings and returns it
// *does not work in lua with QPB*
xander= msg.add_array_of_strings()
xander= "sets the contents of the newly added string."
```

QPB would need a new proxy object to handle these methods, and even then, I'm not sure it would be able to handle the methods well. The proxies would be _like_ lua strings, but never quite _exactly_.

The other string accessors **do** work, though, so you should still be able to achieve whatever you need:
```
   string xavier="i can read your mind."
   msg.set_string( xavier )

   string xander="trying not to think..."
   msg.add_array_of_strings( xander )
   msg.add_array_of_strings( xander )
```