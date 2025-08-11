# tpp-server-emulator

![license](https://img.shields.io/github/license/alicealys/tpp-server-emulator.svg)
[![open bugs](https://img.shields.io/github/issues/alicealys/tpp-server-emulator/bug?label=bugs)](https://github.com/alicealys/tpp-server-emulator/issues?q=is%3Aissue+is%3Aopen+label%3Abug)
[![Build](https://github.com/alicealys/tpp-server-emulator/workflows/Build/badge.svg)](https://github.com/alicealys/tpp-server-emulator/actions)

Reimplementation of **Metal Gear Solid V: The Phantom Pain**'s backend server

discord server
https://discord.gg/hYfW9MEEGF

## Downloads

- [Windows x64](https://github.alicent.cat/tpp-server-emulator/tpp-server-emulator.exe)

## Documentation

- [Usage/Installation](https://github.com/alicealys/tpp-server-emulator/blob/main/docs/usage.md)
- [Scripting](https://github.com/alicealys/tpp-server-emulator/blob/main/docs/scripting.md)

## Compile from source

- Clone the Git repo. Do NOT download it as ZIP, that won't work.
- Update the submodules and run `premake5 vs2022` or simply use the delivered `generate.bat`.
- Build via solution file in `build\tpp-server-emulator.sln`.

  ### Premake arguments

  | Argument                    | Description                                    |
  |:----------------------------|:-----------------------------------------------|
  | `--copy-to=PATH`            | Optional, copy the EXE to a custom folder after build, define the path here if wanted. |

## Disclaimer

This software has been created purely for the purposes of
academic research. It is not intended to be used to attack
other systems. Project maintainers are not responsible or
liable for misuse of the software. Use responsibly.

# Credits
* https://github.com/unknown321/mgsv_emulator
