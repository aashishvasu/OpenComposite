# OpenComposite - Play SteamVR games without SteamVR!

[![Discord](https://img.shields.io/discord/499733750209314816.svg)](https://discord.gg/zYA6Tzs)
[![AppVeyor](https://img.shields.io/appveyor/ci/ZNix/openovr.svg)](https://ci.appveyor.com/project/ZNix/openovr)

OpenComposite (previously known as OpenOVR - OpenVR for OculusVR - but renamed due to confusion with OpenVR) is
an implementation of SteamVR's API - OpenVR, forwarding calls directly
to the Oculus runtime. Think of it as a backwards version of ReVive, for the Rift.

This allows you to play SteamVR-based games on an Oculus Rift as though they were native titles, without
the use of SteamVR!

Why would you want to do this? There are several of good reasons:

* Some users have trouble running SteamVR on their machines
* While more work needs to be done on OpenComposite's side for this, it can potentially get slightly better
performance due to several design decisions, due to not having to support the Vive
* When you run a program it's guaranteed to show up in Oculus Home, while with SteamVR if you start the
game when SteamVR is already running this won't happen.
* Again a problem for some people but not others, SteamVR can take some time to start up
* SteamVR is several gigabytes in side - that might be a problem for some people

It currently implements most interfaces, and can play many SteamVR games. However, there are many more games
that use versions of interfaces that I have yet to implement - this will result in a popup followed by a crash. If
you find a game that does this, please let me know (see below).

There are several things missing though:
- Oculus Touch models are not present. Some games (eg, *Skyrim*) will use their own models in this case. For other games, you
might not see a controller model at all. Most games use their own hand models though, rather than displaying a model
of your controller.
- The virtual keyboard does not yet work in OpenGL-, DirectX12- or Vulkan-based games. This should only affect DOOM BFG and older
versions of Vivecraft however.

The games that I can confirm it works with are as follows:
- Skyrim VR
- VTOL VR
- PAYDAY 2 VR

It probably works in quite a few other games, but I have not tried them.

## Downloading and installation

OpenComposite can either be installed system-wide (all SteamVR-based games will use it automatically) or per-game (replace a DLL
file for each game, this usually must be done each time said game is updated).

### System-wide installation

Download the [OpenComposite Launcher](https://znix.xyz/OpenComposite/runtimeswitcher.php). Unzip it to a folder of your choosing,
and run `OpenComposite.exe`. Click 'Switch to OpenComposite', and wait while the DLLs are downloaded. Games will now run via OpenComposite
rather than SteamVR.

To update OpenComposite, run `OpenComposite.exe` again. The text at the bottom of the window will indicate if an update is available, and
if so an update button will appear. Click it and OpenComposite will be updated.

### Per-game installation

Download the DLLs:
[32-bit](https://znix.xyz/OpenComposite/download.php?arch=x86)
[64-bit](https://znix.xyz/OpenComposite/download.php?arch=x64)

These come from [AppVeyor](https://ci.appveyor.com/project/ZNix/openovr) - whenever I push some new code, it will be compiled
and uploaded to those links automatically.

Find your game's `openvr_api.dll` file, and replace it (though I highly recommend keeping a copy of the old file if you want to
switch back) with one of the DLLs available above. Be sure to get the matching platform - if the game is a 32-bit game you need
the 32-bit DLL, even though you're probably running a 64-bit computer. Simple solution: if one doesn't work, try the other.

Some time in the near future I plan to release a system-wide install - run an EXE and all your games will open via OpenComposite, and
you'll be able to switch back by starting SteamVR. This will make updating much easier.

If you have any questions, feel free to jump on our Discord server:
[![Discord](https://img.shields.io/discord/499733750209314816.svg)](https://discord.gg/zYA6Tzs)

## Configuration file

On startup, OpenComposite searches for a file named `opencomposite.ini`, first next to the `openvr_api.dll` file, and if it can't find
that, then it looks in the working directory (usually in the same folder as the EXE). If no file is found, that's fine and all
the default values are used.

This is it's configuration file, where you can specify a number of options. These are listed in `name=value` format, and you
can define a comment by starting a like with `;` or a `#`. Note the option names **are** case sensitive. If you specify an invalid
option, the game will immediately crash on startup. Any whitespace between the name, equals sign, and value is ignored, as is whitespace
on the end of the line. Ensure your lines do not being with whitespace however.

The available options are:

* `renderCustomHands` - boolean, default `enabled`. Should OpenComposite render custom hands. Disable this if you *really* dislike
the hand models, and some games (like Skyrim) come with backup models that will be used instead.
* `handColour` - colour, default `#4c4c4c`. The colour of the hands OpenComposite draws, as they currently lack proper textures.
* `supersampleRatio` - float, default `1.0`. The supersample ratio in use - this is similar to what you would enter into SteamVR,
a value of `145%` in SteamVR is a value of `1.45` here. A value of `80%` in SteamVR is `0.8` here, and so on. Higher numbers improve
graphics, at a major performance cost.
* `haptics` - boolean, default `enabled`. Should haptic feedback to the Touch controllers be enabled.
* `admitUnknownProps` - boolean, default `disabled`. If asked for a tracked device property it does not understand, should OpenComposite
set that as an error and let the game continue. Enabling this may cause severe and hard-to-detect side-effects, and as such if a property
is missing, please report it and let it get fixed like that. However, some (very few) games (namely Vivecraft) will query every property
they know about, even if never using all but a few of them. In these cases, you should enable it.
* `forceConnectedTouch` - boolean, default `enabled`. If this is enabled, games are always told that the Touch controllers are connected,
even if they are not. This ensures controllers will work if they were asleep when the game was started. If you use a gamepad and don't want
the game to think controllers are connected, disable this option. See issue #25.
* `logGetTrackedProperty` - boolean, default `disabled`. Print logging information when the app requests information about tracked devices,
such as the HMD or the Touch controllers. On some games, this causes a log entry to be generated every frame, which isn't great for performance
and clutters up the log. This is potentially useful for troubleshooting, and was enabled by default before the config option existed. In general,
unless you've been told to enable this (or you know what you're doing while troubleshooting) you don't need to enable this.

The possible types are as follows:

* boolean - one of `on`, `true`, `enabled` to enable the setting, or `off`, `false` or `disabled` to do the opposite. These
are not case-sensitive, so `Enabled` or `tRUe` work fine.
* colour - same as the format used in HTML/CSS, it's a `#` followed by six hex characters. Mozilla has a
[tool](https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_Colors/Color_picker_tool) to help with this. Again, it's case-insensitive.
* float - a floating point (decimal) number, eg `123.456`.

Example files:

- Running a simulator on a powerful computer, and wearing external headphones so you don't want the audio redirected:

```
enableAudio=off
supersampleRatio=1.4
```

- Running SkyrimVR with yellow hands, and haptic feedback disabled:

```
handColour=#ffff00
haptics = off
```

## Reporting a bug

If you find an issue, missing interface, crash etc then *please* let me know about it.
([Opening a GitLab issue](https://gitlab.com/znixian/OpenOVR/issues/new) is the preferred
way of doing this and you'll automatically get an email when it's fixed (if you let me know
via a different method I'll probably make an issue and send you the link to help keep track of them).

If you do not or cannot create or use a GitLab account for whatever reason, you can message me
on Discord - I'm at ZNix#6217. If there is enough interest, I'll create a Discord server.

In any case, please include the contents your `openovr_log` file - each time you start
a game, this is created next to the EXE file and contains potentially invaluable debugging
information. Please also say what caused the crash - did it crash on startup, or maybe after clicking
load on the main menu, for example?

## Licence

OpenComposite itself is Free Software under the GPLv3:

```
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
```

[GPL Full Text](https://www.gnu.org/licenses/gpl-3.0-standalone.html). This also can be found in
the `LICENCE.txt` file supplied with the source code.

### OpenXR SDK

The OpenXR SDK is included as a git submodule. It's under the Apache 2 licence, the full text of
which can be found under `libs/openxr-sdk/LICENSE`.

### OpenVR

This program (and it's source code) includes several versions of the
[OpenVR](https://github.com/ValveSoftware/openvr) headers. These are under the
[MIT Licence](https://github.com/ValveSoftware/openvr/blob/master/LICENSE). This also can be
found in the `LICENSE_OPENVR.txt` file supplied with the source code.

```
Copyright (c) 2015, Valve Corporation
All rights reserved.
```

### INIH

This program uses the [inih](https://github.com/benhoyt/inih) INI parsing library by Ben Hoyt. This is available under
the MIT licence, a copy of which can be found in the `LICENSE_INIH` file of this repository.

### Microblink Build

This program uses a part of Psiha's and Microblink's [build](https://github.com/microblink/build) CMake utilities
library. This is available under the two-clause BSD licence, a copy of which can be found in the `LICENCE_BUILD_UTIL.txt`
file of this repository.

## Compiling (**for developers**)

Download the Source Code from [GitLab](https://gitlab.com/znixian/OpenOVR) - it's under the GPLv3 licence.

Download Visual Studio 2017 and it's C++ package. This may work on older versions of VS, but that break
at any time.

Next, if you want to use Vulkan support (enabled by default, remove `SUPPORT_VK` from the preprocessor definitions to compile without it), then
download a [slimmed down copy of the Vulkan SDK with only `.lib` files and the headers](http://znix.xyz/random/vulkan-1.1.85.0-minisdk.7z),
and extract it to your `libs` directory (you can copy in the full SDK if you want, but it's far larger and probably better to install it
by itself).

This is how the resulting file structure should look:

```
OpenOVR: (or whatever folder you cloned/downloaded the repo into)
	libs:
		libovr:
			Place LibOVR Here.txt
			OculusSDK:
				LibOVR:
				LibOVRKernel:
				LICENSE.txt
				... etc
		vulkan:
			Include:
			Lib:
			Lib32:
			LICENSE.txt
			SDK_LICENSE.rtf
			Vulkan.ico

	configure.cmd
	OpenOVR.sln
	... etc
```

Next, we need to configure the project. OpenComposite relies fairly heavily on scripts that
generate code, and this has *vastly* simplified things - I expect the repo would contain
roughly three to four times more code otherwise.

There are two main scripts at work:

1. A script to take various OpenVR header files, and split them into one header file for each
version of each interface. This allows the project to reference many versions of the same interface.
2. A script to generate the interface stubs - in LibOVR, there is one implementation of each interface,
which are the base interfaces (eg `BaseSystem`, `BaseCompositor`, etc). There are then autogenerated 'stub'
classes, which implement a binary-compatible version of one version of that interface (eg `GVRSystem_017`,
`GVRSystem_015`, `GVRCompositor_022`), along with C-like `FnTable`s for them (which allow compatibility
with other languages, very notably C#).

Whenever you add or remove versions of OpenVR headers, or add or remove an interface or tell the
generator to make a stub for one, you MUST re-run `generate.bat` (TODO port to GNU Bash, I only used Batch
due to AppVeyor). If you have added or removed OpenVR headers this will cause Visual Studio to rebuild
the project, otherwise it will only rebuild a couple of files which is very quick.

Whenever you check out a new version of OpenComposite, you should run the configure script. If you don't, you're
liable to get mysterious errors at both runtime and compiletime.

(additionally the configure script can accept `clean` as an argument, which will make it delete all
generated files - this is really only useful when debugging the scripts, though, and there is no need
to use it otherwise)

### Using the interface splitter

To add a new version of OpenVR, download the header to an appropriately named file in `OpenOVR/OpenVR`,
which is `openvr-version.h`. Please always ensure the version matches.

Next, edit `OpenOVR/OpenVR/generate.py`, adding the new version number to the `versions` list at the
top of the file. Ensure these are in the correct order - this makes sure the latest revision of any given
interface is always used (interfaces are sometimes edited without incrementing their version numbers, when
the changes don't affect the ABI - for example, adding items to an enumeration).

Please also add all the interface files to the Visual Studio project, however not doing so will not stop
the project from compiling.

### Using the stub generator

The stub generator has a list of interfaces at the top of it's file (`OpenOVR/Reimpl/generate.py`, in the
`interfaces_list` variable). This lists each implemented interface, and there should be a corresponding
`CVR*.cpp` file for each interface. This defines which versions of the interface are to be used.

The format of the CVR ('stub definition') file is as follows:

```
#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

GEN_INTERFACE("InterfaceName", "123")
// etc
```

(note the version **must** be exactly three digets long)

Each `GEN_INTERFACE` macro (ignored by the compiler) instructs the stub generator to generate a stub
for that interface for that file. There's no technical reason why you can't have one file generating
the stubs for many interfaces - however, by convention each stub definition file corresponds to
a single interface.

Each stub definition file causes the generation of a header - `GVR*.gen.h` ('G' in GVR for
'generated'). This header contains the class declaration for the associated stub, however it does
not contain any definitions. These classes are named `CVR*_123`, where `123` is the version.

The generator also generates a single C++ file (`stubs.gen.cpp`), which contains stub definitions for
each method in the stub, and calls the associated method on the base class.

If the generator finds a type declared in the OpenVR interface header, it takes the base name and prepends
it with `OOVR_`. Therefore, you must declare these types in your base file.

#### Overriding generated stubs

Sometimes the autogenerated stubs are unsuitable in some way. In case this happens, just add the function
definitions to the end of your stub definition file, and the generator will avoid generating it's own definition.

So, prepend this to the end of your stub definition:

```
#include "GVRMyInterface.gen.h"

void CVRMyInterface_123::MyFunc(...) {
}
```

(note that if you have a lot of arguments and you wish to insert a linebreak in
the declaration part of the function, end the line with a backslash or a comma)

The stub generator will thus not generate a stub method for `CVRMyInterface_123::MyFunc`, and your definition will
be used instead. Be careful though, as you need one function declaration per version, and with many versions this
could get messy.
