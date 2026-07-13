# tpp-server-emulator

![license](https://img.shields.io/github/license/alicealys/tpp-server-emulator.svg)
[![open bugs](https://img.shields.io/github/issues/alicealys/tpp-server-emulator/bug?label=bugs)](https://github.com/alicealys/tpp-server-emulator/issues?q=is%3Aissue+is%3Aopen+label%3Abug)
[![Build](https://github.com/alicealys/tpp-server-emulator/workflows/Build/badge.svg)](https://github.com/alicealys/tpp-server-emulator/actions)

Reimplementation of **Metal Gear Solid V: The Phantom Pain**'s backend server

discord server
https://discord.gg/hYfW9MEEGF

## Downloads
- [tpp-server-emulator-linux_x64.tar](https://github.alicent.cat/tpp-server-emulator/tpp-server-emulator-linux_x64.tar)
- [tpp-server-emulator-windows_x64.tar](https://github.alicent.cat/tpp-server-emulator/tpp-server-emulator-windows_x64.tar)

### Binary only

- [tpp-server-emulator.exe](https://github.alicent.cat/tpp-server-emulator/tpp-server-emulator.exe)
- [tpp-server-emulator](https://github.alicent.cat/tpp-server-emulator/tpp-server-emulator) (linux x64)

### Version history

https://archive.alicent.cat/tpp-server-emulator

## Documentation

- [Usage/Installation](https://github.com/alicealys/tpp-server-emulator/blob/main/docs/usage.md)
- [Scripting](https://github.com/alicealys/tpp-server-emulator/blob/main/docs/scripting.md)
- [Progress](https://github.com/alicealys/tpp-server-emulator/blob/main/docs/progress.md)

## Compile from source

- Clone the Git repo. Do NOT download it as ZIP, that won't work.
- Install [premake5](https://premake.github.io/download) on your PATH
- **Windows** Run `generate.bat` and build via solution file in `build\tpp-server-emulator.sln`.
- **Linux** Run `generate.sh` then run `build-release.sh` or `build-debug.sh`  
  **NOTE**: make sure you are using `clang` or, if you are using `gcc`, use the [Mold](https://github.com/rui314/mold) linker
  ### Premake arguments

  | Argument                    | Description                                    |
  |:----------------------------|:-----------------------------------------------|
  | `--copy-to=PATH`            | Optional, copy the EXE to a custom folder after build, define the path here if wanted. |

## Disclaimer

This software has been created purely for the purposes of
academic research. It is not intended to be used to attack
other systems. Project maintainers are not responsible or
liable for misuse of the software. Use responsibly.

## Credits
* https://github.com/unknown321/mgsv_emulator
