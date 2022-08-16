[TOC]

# The OpenVR Interface and Versioning System

This describes how OpenVR's interfaces work. This is from the application's point of view, and mostly ignores what's
happening on the runtime (SteamVR or OpenComposite) side to make it happen. This also assumes the per-game (replace
the DLL/SO) installation method is used; the system-wide method (involving the `openvrpaths.vrpath` JSON file) is very
similar, and basically has an abstraction layer in the middle.

The application is built using `openvr.h`, which has a tiny amount of code in it to make the described API more
ergonomic to use. I'll consider that as being part of the application for the purpose of this discussion. It's not long,
you can read the end of the openvr.h header to see what it does.

The first thing an application can do is call `VR_IsRuntimeInstalled` and `VR_IsHmdPresent`. These do what they say on
the tin: they tell the application whether there's a VR runtime installed and headset available. Most VR-only games
don't use them and will try to initialise without a headset attached, but it's frequently used by applications like
flight and driving simulators that can also be used outside of VR.

## Initialisation and shutdown

The application calls `VR_InitInternal2` and receives a token. This token increases each time the OpenVR session is
restarted, to help with caching on the application side and isn't really of concern. The application then gets it's
first interface, `IVRSystem_xxx` - more on that later.

When the application is done using VR, it'll call `VR_ShutdownInternal` to exit the session.

## Interfaces

A fairly fundamental problem when making an API like this - where the runtime and application must maintain
compatibility across enormous periods - is versioning. If the runtime just exported all it's functions using the normal
OS dllexport (or not marking the symbol private on Linux) mechanism, then that poses a problem: what if the runtime
wants to modify functions over time, particularly by changing their signature? This would require either using some
platform-specific function versioning system, or adding a number to the end of the function (or something similar).

This isn't particularly appealing, and one of the many problems is an application can mix-and-match different versions
of functions, so you can end up with very old functions still being used. Simply not putting something in a header won't
necessarily stop people from using it in new applications, unfortunately.

A better solution is to supply pointers to all the functions, in return for a version number. The application supplies
the version of the API it's using, and gets back a struct-full of function pointers. This might look something like:

```c++
void *Lookup(int version);

// Only contained in v1 of the header
struct MyAPI_v1 {
    void (*DoSomething)(int);
};

// Only contained in v2 of the header
struct MyAPI_v2 {
    void (*DoSomethingElse)();
    void (*DoSomething)(const char*);
};
```

The real header would only actually have one of those structs, wouldn't have the version number in the struct name, etc.
This is pretty close to the system the OpenVR uses: the first difference is that it supplies a C++ object extending a
pure virtual class, and the second is that OpenVR has more than one of these API classes: it separates the core system
functions, the compositor functions, overlay functions, input functions, etc. into their own independently-versioned
classes. The `Lookup` function then takes a string that specifies what interface and version thereof is needed.

This looks something like:

```c++
// The basic system functions, 'IVR' meaning VR interface and is a convention used through all public interface classes.
// This is very roughly copied from openvr.h, see LICENCE_OPENVR.txt in the repo root.
class IVRSystem {
    public:
	virtual void GetRecommendedRenderTargetSize( uint32_t *pnWidth, uint32_t *pnHeight ) = 0;
    virtual HmdMatrix44_t GetProjectionMatrix( EVREye eEye, float fNearZ, float fFarZ ) = 0;
    virtual void GetProjectionRaw( EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom ) = 0;
    // ... etc
};
static const char * const IVRSystem_Version = "IVRSystem_022";

class IVRCompositor {
    public:
    virtual EVRCompositorError WaitGetPoses(TrackedDevicePose_t *outDevicePositions) = 0;
    virtual EVRCompositorError Submit(EVREye eEye, const Texture_t *pTexture) = 0;
};
static const char * const IVRCompositor_Version = "IVRCompositor_027";

void* VR_GetGenericInterface(const char *interfaceVersion, EVRInitError *outError);
```

The application then gets a pointer to an implementation of one of these interfaces and use it like so:

```c++
IVRSystem *sys = (IVRSystem*) VR_GetGenericInterface(IVRSystem_Version, &err);
sys->GetRecommendedRenderTargetSize(&width, &height);
```

This is (without the helpers from openvr.h that do all this for you) basically how an OpenVR application interface with
the runtime. As a relevant bonus, it's easy to add indirection with basically no performance cost: the DLL that contains
the `VR_GetGenericInterface` function can just delegate it to another DLL and so forth, and it doesn't matter if that's
relatively slow since it's only called a couple dozen times at most. Whereas if `WaitGetPoses` for example was a DLL
export, that long chain of calls would have to happen on every invocation. It's probably not a particularly important
amount, but it's nice to squeeze out every last bit.

This doesn't work very nicely in C or C# though: C doesn't have a concept of a class or virtual functions, and C# does
have classes but when working with native applications using a struct of function pointers is much easier. Thus, OpenVR
has it's FnTable system. In `openvr_capi.h` a set of structs are defined like so:

```c
struct VR_IVRCompositor_FnTable {
    EVRCompositorError (*WaitGetPoses)(TrackedDevicePose_t *outDevicePositions);
    EVRCompositorError (*Submit)(EVREye eEye, const Texture_t *pTexture);
};
static const char * IVRCompositor_Version = "FnTable:IVRCompositor_027";

void* VR_GetGenericInterface(const char *interfaceVersion, EVRInitError *outError);
```

Used like so:

```c
VR_IVRCompositor_FnTable *comp = (VR_IVRCompositor_FnTable*) VR_GetGenericInterface(IVRCompositor_Version, &err);
comp->WaitGetPoses(...);
```

(Note the `FnTable:` prefix isn't actually present in the real string constants, and must be added manually by the
application)

The C# bindings work in largely the same way, with structs of function pointers retrieved with the `FnTable:` prefix.

# The OpenComposite implementation

After the interface splitting documented in [SplitAndGen.md] runs, we're left with a bunch of headers for every version
of `IVRSystem` (and any other interface) that we support, like this somewhat highly edited example:

```c++
// In <build dir>/generated/interfaces/IVRCompositor_012.h
namespace IVRCompositor_012
{
    class IVRCompositor {
        public:
        EVRCompositorError (*Submit)(EVREye eEye, const Texture_t *pTexture);
    };
}

// In <build dir>/generated/interfaces/IVRCompositor_013.h
namespace IVRCompositor_013
{
    class IVRCompositor {
        public:
        virtual EVRCompositorError (*Submit)(EVREye eEye, const Texture_t *pTexture) = 0;
        virtual EVRCompositorError WaitGetPoses(TrackedDevicePose_t* outHmdPose) = 0;
    };
}
```

So we need to be able to supply the application with an instance of either of these. There's various approaches to this.
One would be to manually implement all of these interfaces, putting all the real functionality in the latest one and
making the old interfaces call the latest implementation. That'd be a huge pain to do manually, however.

Instead, there's a code generation system. One single implementation is defined manually in the `BaseCompositor` class,
and a script (`scripts/split_headers.py`) parses the OpenVR headers with regexes (yes, it does work, and no it's not
very pretty) and produces a bunch of implementing classes (the 'C' in 'CVR' means 'concrete' IIRC):

```c++
//////////////
/// <build dir>/generated/GVRCompositor.gen.h
//////////////

class CVRCompositor_012 : public vr::IVRCompositor_012::IVRCompositor, public CVRCommon {
private:
	const std::shared_ptr<BaseCompositor> base;
public:
	virtual void** _GetStatFuncList() = 0; // Get the FnTable: function array pointer
    // Interface methods:
    virtual EVRCompositorError (*Submit)(EVREye eEye, const Texture_t *pTexture) override;
    virtual EVRCompositorError WaitGetPoses(TrackedDevicePose_t* outHmdPose) override;
};

//////////////
///  <build dir>/generated/stubs.gen.cpp:
//////////////

vr::IVRCompositor_012::EVRCompositorError CVRCompositor_012::WaitGetPoses(TrackedDevicePose_t* outHmdPose) {
    return (vr::IVRCompositor_012::EVRCompositorError) base->WaitGetPoses(outHmdPose);
}
// all other functions here

static CVRCompositor_012 *fntable_Compositor_012_instance = NULL;
static vr::IVRCompositor_012::EVRCompositorError fntable_Compositor_012_impl_WaitGetPoses(TrackedDevicePose_t* outHmdPose) {
    return fntable_Compositor_012_instance->WaitGetPoses(outHmdPose);
}
// all other fntable functions
static void *fntable_Compositor_012_funcs[] = {
	(void*) fntable_Compositor_012_impl_Submit,
	(void*) fntable_Compositor_012_impl_GetLastPoses,
    // etc
};
void** CVRCompositor_012::_GetStatFuncList() {
    fntable_Compositor_012_instance = this;
    return fntable_Compositor_012_funcs;
}

// One copy of this function, which all interfaces are squeezed into
void *CreateInterfaceByName(const char *name) {
	if (strcmp(vr::IVRCompositor_012::IVRCompositor_Version, name) == 0) return new CVRCompositor_012();
	if (strcmp(vr::IVRCompositor_013::IVRCompositor_Version, name) == 0) return new CVRCompositor_013();
    // etc
    return NULL;
}
```

This way, we have classes implementing each required interface, but only have a single copy of the code. When the
application requests an interface for the first time, we just have to call `CreateInterfaceByName`.

If the application asks for the `FnTable:` version of an interface, calling `interface->_GetStatFuncList()` gives us the
required result.
