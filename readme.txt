to compile you need the protobuffer code and an environment variable 
"GOOGLE_DEV" pointing to the google platform build tree.

ex. if GOOGLE_DEV = C:\dev\protobuf-2.4.1\vsprojects
then the qpb.vcproject include directories specifies: 
$(GOOGLE_DEV)\include