Provides protocol buffer support in lua without having to pre-generate lua bindings. Instead, it uses reflection over the same protocol buffers compiled for CPlusPlus via protoc.

| Take the [TenCentTour](TenCentTour.md) to see how it works. |
|:------------------------------------------------------------|



Note: So far, I've only compiled it via Visual Studio 2010, so it might need some tweaking for other platforms. That said, it has no dependencies other than lua and libprotobuf, and weighs in with 10 files totaling 1500 lines of C++ code.