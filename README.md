# tpp-server-emulator

![license](https://img.shields.io/github/license/fedddddd/tpp-server-emulator.svg)
[![open bugs](https://img.shields.io/github/issues/fedddddd/tpp-server-emulator/bug?label=bugs)](https://github.com/fedddddd/tpp-server-emulator/issues?q=is%3Aissue+is%3Aopen+label%3Abug)
[![Build](https://github.com/fedddddd/tpp-server-emulator/workflows/Build/badge.svg)](https://github.com/fedddddd/tpp-server-emulator/actions)

Reimplementation of **Metal Gear Solid V: The Phantom Pain**'s backend server

## Usage

- Setup a mysql server with a database called `mgstpp`.
- In the EXE's path create a file called `config.json` and add the following values:
    * `database_user`: (default: `"root"`)
    * `database_password`: (default: `"root"`)
    * `database_host`: (default: `"localhost"`)
    * `database_port`: (default: `3306`)
    * `database_name`: (default: `"mgstpp"`)
  * Additionally you can also specify these values:
    * `http_port`: (default: `80`)
    * `https_port`: (default: `443`)
    * `cert_file`: Path to the cert file (default: `""`)
    * `key_file`: Path to the key file (default: `""`)
    * `base_url`: Base url for the url list command (default: `"http://localhost:80"`)
    * `use_konami_auth`: Relay authentication to konami servers, **if set to `false` steam tickets will not be verified** (default: `true`)
- Change the string `"https://mgstpp-game.konamionline.com/tppstm/gate"` in mgsvtpp.exe to `"{your base url}/tppstm/gate"`.  
  Alternatively, run [tpp-mod](https://github.com/fedddddd/tpp-mod) with the `-custom-server url` flag.

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
