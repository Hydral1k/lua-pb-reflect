# Introduction #

There are some aspects of the C++ interface that are not implemented. I plan to whittle away at them as as need requires and time allows.

# Details #

| Enum listings: Foo\_descriptor(), and Foo\_IsValid(int) | The normal set and get enum methods work, just these two "meta" ones do not |
|:--------------------------------------------------------|:----------------------------------------------------------------------------|
| Serialization                                           |  I'm not sure what the right interface is yet, it needs some thought.       |
| Message Utility Methods                                 | Merge, CopyFrom, IsInitialized, GetTypeName, InitializationErrorString, DebugString, ... |
| Descriptors                                             |                                                                             |
| Reflection                                              | I can't imagine these would be needed in lua if the descriptors existed.    |
| Extensions                                              |                                                                             |
| RPC                                                     |                                                                             |