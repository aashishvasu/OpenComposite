# OpenOVR - Play SteamVR games without SteamVR!

OpenOVR (OpenVR for OculusVR) is an implementation of SteamVR's API - OpenVR, forwarding calls directly
to the Oculus runtime. Think of it as a backwards version of ReVive, for the Rift.

Why would you want to do this? There are several of good reasons:

* Some users have trouble running SteamVR on their machines
* While more work needs to be done on OpenOVR's side for this, it can potentially get slightly better
performance due to several design decisions, due to not having to support the Vive
* When you run a program it's guaranteed to show up in Oculus Home, while with SteamVR if you start the
game when SteamVR is already running this won't happen.
* Again a problem for some people but not others, SteamVR can take some time to start up
* SteamVR is several gigabytes in side - that might be a problem for some people

It currently implements most interfaces, and can play many SteamVR games. However, there are many more games
that use versions of interfaces that I have yet to implement - this will result in a popup followed by a crash. If
you find a game that does this, please let me know (see below).

There are several things missing though:
- Most notably, the virtual keyboard (games that try to open it will crash). I do plan to implement this, however.
- Oculus Touch models are not present. Some games (eg, *Skyrim*) will use their own models in this case. For other games, you
might not see a controller model at all. Most games use their own hand models though, rather than displaying a model
of your controller.

The games that I can confirm it works with are as follows:
- Skyrim VR (though note you'll crash while naming your character due to the aforementioned lack of a keyboard, so you
must start new saves in SteamVR)
- VTOL VR (again, you can't name your pilot)
- PAYDAY 2 VR

It probably works in quite a few other games, but I have not tried them.

## Downloading and installation

Download:
[32-bit](https://ci.appveyor.com/api/projects/ZNix/openovr/artifacts/x86/openvr_api.dll?branch=mas64ter&job=Platform%3A+x86)
[64-bit](https://ci.appveyor.com/api/projects/ZNix/openovr/artifacts/x64/openvr_api.dll?branch=master&job=Platform%3A+x64)

These come from [AppVeyor](https://ci.appveyor.com/project/ZNix/openovr) - whenever I push some new code, it will be compiled
and uploaded to those links automatically.

Find your game's `openvr_api.dll` file, and replace it (though I highly recommend keeping a copy of the old file if you want to
switch back) with one of the DLLs available above. Be sure to get the matching platform - if the game is a 32-bit game you need
the 32-bit DLL, even though you're probably running a 64-bit computer. Simple solution: if one doesn't work, try the other.

Some time in the near future I plan to release a system-wide install - run an EXE and all your games will open via OpenOVR, and
you'll be able to switch back by starting SteamVR. This will make updating much easier.

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

OpenOVR itself is Free Software under the GPLv3:

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

### LibOVR

The Oculus SDK (LibOVR) is also included in the binaries, however not in the source
code repository.

```
Copyright © Facebook Technologies, LLC and its affiliates. All rights reserved.

The text of this may be found at: [https://developer.oculus.com/licenses/sdk-3.4.1/](https://developer.oculus.com/licenses/sdk-3.4.1/)
```

### OpenVR

This program (and it's source code) includes several versions of the
[OpenVR](https://github.com/ValveSoftware/openvr) headers. These are under the
[MIT Licence](https://github.com/ValveSoftware/openvr/blob/master/LICENSE). This also can be
found in the `LICENSE_OPENVR.txt` file supplied with the source code.

```
Copyright (c) 2015, Valve Corporation
All rights reserved.
```

## Compiling (**for developers**)

Download the Source Code from [GitLab](https://gitlab.com/znixian/OpenOVR) - it's under the GPLv3 licence.

Download Visual Studio 2017 and it's C++ package. This may work on older versions of VS, but that break
at any time.

After cloning this repository, you must download [LibOVR](https://developer.oculus.com/downloads/package/oculus-sdk-for-windows/).
The archive should contain a single folder, named `OculusSDK`. Place this into the `libovr` folder in the repository root.

This is how the resulting file structure should look:

```
OpenOVR: (or whatever folder you cloned/downloaded the repo into)
	libovr:
		Place LibOVR Here.txt
		OculusSDK:
			LibOVR:
			LibOVRKernel:
			LICENSE.txt
			... etc

	configure.cmd
	OpenOVR.sln
	... etc
```

Next, we need to configure the project. OpenOVR relies fairly heavily on scripts that
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

Whenever you check out a new version of OpenOVR, you should run the configure script. If you don't, you're
liable to get mysterious errors at both runtime and compiletime.

(additionally the configure script can accept `clean` as an argument, which will make it delete all
generated files - this is really only useful when debugging the scripts, though, and there is no need
to use it otherwise)

