# OpenComposite (OpenXR) - Play SteamVR games without SteamVR!
[![Discord](https://img.shields.io/discord/499733750209314816.svg?style=for-the-badge&logo=discord&label=discord)](https://discord.gg/zYA6Tzs)
[![AppVeyor](https://img.shields.io/appveyor/ci/ZNix/openovr.svg?style=for-the-badge&logo=appveyor)](https://ci.appveyor.com/project/ZNix/openovr)
---------------------------------------------
# [GAME COMPATIBILITY SPREADSHEET](https://docs.google.com/spreadsheets/d/1s2SSuRt0oHm91RUZB-R-ef5BrfOtvP_jwDwb6FuMF9Q/edit#gid=2068512515)
Feel free to comment on the spreadsheet for us to modify its information.
------------------------------------------------------

[TOC]

OpenComposite OpenXR (previously known as OpenOVR - OpenVR for OculusVR - but renamed due to confusion with OpenVR) is
an implementation of SteamVR's API - OpenVR, forwarding calls directly
to the OpenXR runtime. Think of it as a backwards version of ReVive, for the OpenXR compatible headsets.

This allows you to play SteamVR-based games on an OpenXR compatible headset as though they were native titles, without
the use of SteamVR!

Why would you want to do this? There are several good reasons:

* Some users have trouble running SteamVR on their machines
* While more work needs to be done on OpenComposite's side for this, it can potentially get slightly better
performance due to several design decisions, due to not having to support the Vive
* OpenXR is less susceptible to the stuttering that has plagued SteamVR
* Again a problem for some people but not others, SteamVR can take some time to start up
* SteamVR is several gigabytes in side - that might be a problem for some people

It currently implements most interfaces, and can play many SteamVR games. However, there are many more games
that use versions of interfaces that I have yet to implement - this will result in a popup followed by a crash. If
you find a game that does this, please let me know (see below).

There are several things missing though:
- Hand controller models are not present. Some games (eg, *Skyrim*) will use their own models in this case. For other games, you
might not see a controller model at all. Most games use their own hand models though, rather than displaying a model
of your controller.
- The virtual keyboard does not yet.

\** Some Oculus headsets seem to be incompatible with OpenXR initialising with Vulkan for some games. If you have issues with AC or other games try putting initUsingVulkan=false in the opencomposite.ini (see [Configuration](https://gitlab.com/znixian/OpenOVR/-/tree/openxr#configuration-file) section for details.)

# Downloading and installation

OpenComposite can either be installed system-wide (all SteamVR-based games will use it automatically) or per-game (replace a DLL
file for each game, this usually must be done each time said game is updated).

(Note that these instructions are intended for Windows, Linux users can scroll down to the [Linux specific](#linux-specific-info) section)

## System-wide installation

Download the [OpenComposite Launcher](https://znix.xyz/OpenComposite/runtimeswitcher.php?branch=openxr). Unzip it to a folder of your choosing,
and run `OpenComposite.exe`. Click 'Switch to OpenComposite', and wait while the DLLs are downloaded. Games will now run via OpenComposite
rather than SteamVR.

To update OpenComposite, run `OpenComposite.exe` again. The text at the bottom of the window will indicate if an update is available, and
if so an update button will appear. Click it and OpenComposite will be updated.

## Per-game installation

Download the DLLs:
[32-bit](https://znix.xyz/OpenComposite/download.php?arch=x86&branch=openxr)
[64-bit](https://znix.xyz/OpenComposite/download.php?arch=x64&branch=openxr)

These come from [AppVeyor](https://ci.appveyor.com/project/ZNix/openovr) - whenever I push some new code, it will be compiled
and uploaded to those links automatically.

Find your game's `openvr_api.dll` file, and replace it (though I highly recommend keeping a copy of the old file if you want to
switch back) with one of the DLLs available above. Be sure to get the matching platform - if the game is a 32-bit game you need
the 32-bit DLL, even though you're probably running a 64-bit computer. Simple solution: if one doesn't work, try the other.

Some time in the near future I plan to release a system-wide install - run an EXE and all your games will open via OpenComposite, and
you'll be able to switch back by starting SteamVR. This will make updating much easier.

If you have any questions, feel free to jump on our Discord server:
[![Discord](https://img.shields.io/discord/499733750209314816.svg)](https://discord.gg/zYA6Tzs)

## Linux-specific info

There are no builds available for Linux at this time: you'll have to build it manually for now (see [the section below on compiling](#compiling-for-developers)).
After compiling, there are a few steps you'll have to follow:
1. Get the runtime in place. This can be accomplished through one of the following methods:
	- Set the VR_OVERRIDE variable to the build directory (in Steam launch options: `VR_OVERRIDE=<path to build directory> %command%`)
	- Edit the OpenVR config file - typically located at `~/.config/openvr/openvrpaths.vrpath` - and in the "runtime" section, put the path to the build directory. Note that Steam likes to overwrite this file so you may need to set it to read only.
	- For the elusive native Linux title, you can copy the `vrclient.so` over the game's `libopenvr_api.so`
2. For games utilizing the Steam Runtime (most of them): you will need to work around the Steam Runtime. Here are some general tips for working around, as setups can differ:
	- Any files located in your home directory will exist with no problems in the runtime. There is nothing to take into consideration with these paths, they will work fine.
	- Any path located under `/usr` can be found under `/run/host/usr` in the runtime. For example, if you installed Monado system wide, the runtime json would be located at `/run/host/usr/share/openxr/1/openxr_monado.json`.
	- Any other paths can be added using the `PRESSURE_VESSEL_FILESYSTEMS_RW` environment variable.

As an example, here is what your Steam launch options might look like if you cloned OpenComposite to your home directory, used the VR_OVERRIDE environment variable, and installed Monado system wide:

```
VR_OVERRIDE=~/OpenOVR/build XR_RUNTIME_JSON=/run/host/usr/share/openxr/1/openxr_monado.json PRESSURE_VESSEL_FILESYSTEMS_RW=$XDG_RUNTIME_DIR/monado_comp_ipc %command%
```

Note that `$XDG_RUNTIME_DIR/monado_comp_ipc` is a socket used by Monado and required to be present for it to properly detect that Monado is running.

You can read more about the `PRESSURE_VESSEL_FILESYSTEM_RW` variable and other environment variables utilized by the Steam Runtime [here](https://gitlab.steamos.cloud/steamrt/steam-runtime-tools/-/blob/master/pressure-vessel/wrap.1.md).

# Configuration file

On startup, OpenComposite searches for a file named `opencomposite.ini`, first next to the `openvr_api.dll` file, and if it can't find
that, then it looks in the working directory (usually in the same folder as the EXE). If no file is found, that's fine and all
the default values are used.

This is it's configuration file, where you can specify a number of options. These are listed in `name=value` format, and you
can define a comment by starting a line with `;` or a `#`. Note the option names **are** case sensitive. If you specify an invalid
option, the game will immediately crash on startup. Any whitespace between the name, equals sign, and value is ignored, as is whitespace
on the end of the line. Ensure your lines do not being with whitespace however.

The available options are:

* `renderCustomHands` - boolean, default `enabled`. 
	* Should OpenComposite render custom hands. Disable this if you *really* dislike the hand models, and some games (like Skyrim) come with backup models that will be used instead.
* `handColour` - colour, default `#4c4c4c`. 
	* The colour of the hands OpenComposite draws, as they currently lack proper textures.
* `supersampleRatio` - float, default `1.0`. 
	* The supersample ratio in use - this is similar to what you would enter into SteamVR, a value of `145%` in SteamVR is a value of `1.45` here. A value of `80%` in SteamVR is `0.8` here, and so on. Higher numbers improve graphics, at a major performance cost.
* `haptics` - boolean, default `enabled`. 
	* Should haptic feedback to the Touch controllers be enabled.
* `admitUnknownProps` - boolean, default `disabled`. 
	* If asked for a tracked device property it does not understand, should OpenComposite ignore it.
* `forceConnectedTouch` - boolean, default `enabled`. 
	* If this is enabled, games are always told that the Touch controllers are connected, even if they are not. This ensures controllers will work if they were asleep when the game was started. If you use a gamepad and don't want
the game to think controllers are connected, disable this option. See issue #25. 
* `logGetTrackedProperty` - boolean, default `disabled`. 
	* Print logging information when the app requests information about tracked devices, such as the HMD or the Touch controllers. On some games, this causes a log entry to be generated every frame, which isn't great for performance and clutters up the log. This is potentially useful for troubleshooting, and was enabled by default before the config option existed. In general, unless you've been told to enable this (or you know what you're doing while troubleshooting) you don't need to enable this.
* `enableHiddenMeshFix` - boolean, default `enabled`. 
	* Alter the coordinates of the hidden area mesh mask to fit within the view's projection. The hidden area mesh mask defines a region to which the game will not draw and is used to mask off areas of the display that you typically cannot see in the headset. If you see odd black regions around the view then try disabling this fix.
* `invertUsingShaders` - boolean, default `disabled`. 
	* Invert the image for display using shaders rather than replying on XR runtime and inverted FOV values. Some games render the image inverted and rely on the runtime to display correctly. In OpenXR some runtimes don't do the inversion correctly. If so enable this option for OpenComposite to do the inversion when copying the image using shaders. This might have a minor cost in performance.
* `initUsingVulkan` - boolean, default `true`. 
	* If available the temporary graphics adapter at start up will use Vulkan by default. This may be incompatible with some games. This will mostly affect Oculus headsets. 
* `hiddenMeshVerticalScale` - float, default `1.0`. 
	* The scaling factor used for the hidden area mesh if supported by the application. The hidden area mesh is a region that the game doesn't render to. If you set this lower e.g. `0.8` then less will be drawn at the very top and very bottom of the image improving performance. Suggested range is `0.5` to `1.0`.
* `logAllOpenVRCalls` - boolean, default `false`
	* Log every OpenVR call a game makes. Similar to `logGetTrackedProperty`, this clutters logs and should not be enabled unless necessary.

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

# Reporting a bug

If you find an issue, missing interface, crash etc then *please* let me know about it.
([Opening a GitLab issue](https://gitlab.com/znixian/OpenOVR/issues/new) is the preferred
way of doing this and you'll automatically get an email when it's fixed (if you let me know
via a different method I'll probably make an issue and send you the link to help keep track of them).

If you do not or cannot create or use a GitLab account for whatever reason, you can join the [Discord server](https://discord.gg/zYA6Tzs) and report the issue in #bugs.

In any case, please include the contents your `opencomposite.log` file which contains potentially
invaluable debugging information. This log file can be found here:

Windows - %localappdata%\\OpenComposite\\logs (Copy and paste that into explorer)
Linux - $XDG_STATE_HOME/OpenComposite/logs (If XDG_STATE_HOME is not defined then check ~/.local/state/OpenComposite/logs)

If those locations do not exist this may be due to some odd security restrictions, check the exe folder for opencomposite.log

Please also say what caused the crash - did it crash on startup, or maybe after clicking
load on the main menu, for example?

# Compiling (**for developers**)

Download the Source Code from [GitLab](https://gitlab.com/znixian/OpenOVR/-/tree/openxr-input-refactor) - it's under the GPLv3 licence.
This project is built with CMake and follows a standard build process, so tools like CLion can be used to build it.
The resulting library will be placed at `build/bin/linux64/vrclient.so` on Linux, `build/bin/vrclient_x64.dll` on Windows.

## Windows specific

If you want to use Vulkan support (enabled by default, remove `SUPPORT_VK` from the preprocessor definitions to compile without it), then
download a [slimmed down copy of the Vulkan SDK with only `.lib` files and the headers](http://znix.xyz/random/vulkan-1.1.85.0-minisdk.7z),
and extract it to your `libs` directory (you can copy in the full SDK if you want, but it's far larger and probably better to install it
by itself).

This is how the resulting file structure should look:

```
OpenOVR: (or whatever folder you cloned/downloaded the repo into)
	libs:
		vulkan:
			Include:
			Lib:
			Lib32:
			LICENSE.txt
			SDK_LICENSE.rtf
			Vulkan.ico
		openxr-sdk:
	... etc
```

## Linux specific

OpenComposite will search for Vulkan installed via your package manager. Cloning the Vulkan SDK is not necessary. 
If not using an IDE with CMake support, a set of commands to successfully compile might look like this after navigating to the source directory:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Miscellaneous

OpenComposite relies fairly heavily on scripts that generate code, and this has *vastly* simplified things - 
I expect the repo would contain roughly three to four times more code otherwise.

There are two main scripts at work:

1. A script to take various OpenVR header files, and split them into one header file for each
version of each interface. This allows the project to reference many versions of the same interface.
2. A script to generate the interface stubs - in LibOVR, there is one implementation of each interface,
which are the base interfaces (eg `BaseSystem`, `BaseCompositor`, etc). There are then autogenerated 'stub'
classes, which implement a binary-compatible version of one version of that interface (eg `GVRSystem_017`,
`GVRSystem_015`, `GVRCompositor_022`), along with C-like `FnTable`s for them (which allow compatibility
with other languages, very notably C#).

These scripts are automatically run when buildling the project. If you have added or removed OpenVR headers this will cause CMake to rebuild
the project, otherwise it will only rebuild a couple of files which is very quick.
You can read more about why these scripts exist and how they work in the `doc/` folder of this repository.

# Licence

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

## OpenXR SDK

The OpenXR SDK is included as a git submodule. It's under the Apache 2 licence, the full text of
which can be found under `libs/openxr-sdk/LICENSE`.

## OpenVR

This program (and it's source code) includes several versions of the
[OpenVR](https://github.com/ValveSoftware/openvr) headers. These are under the
[MIT Licence](https://github.com/ValveSoftware/openvr/blob/master/LICENSE). This also can be
found in the `LICENSE_OPENVR.txt` file supplied with the source code.

```
Copyright (c) 2015, Valve Corporation
All rights reserved.
```

## INIH

This program uses the [inih](https://github.com/benhoyt/inih) INI parsing library by Ben Hoyt. This is available under
the MIT licence, a copy of which can be found in the `LICENSE_INIH` file of this repository.

## Microblink Build

This program uses a part of Psiha's and Microblink's [build](https://github.com/microblink/build) CMake utilities
library. This is available under the two-clause BSD licence, a copy of which can be found in the `LICENCE_BUILD_UTIL.txt`
file of this repository.

## DebugBreak

This programme uses Scott Tsai's [DebugBreak](https://github.com/scottt/debugbreak) library when built on Linux. It's full
text can be found in `LICENCE_DEBUG_BREAK.txt`.
