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
- Change the string `"https://mgstpp-game.konamionline.com/tppstm/gate"` in mgsvtpp.exe to `"{your base url}/tppstm/gate"`.  
  Alternatively, run [tpp-mod](https://github.com/fedddddd/tpp-mod) with the `-custom-server url` flag.

## Config

Example config

```json
{
    "base_url": "http://192.168.1.175:80",
    "http_port": 80,
    "https_port": 443,
    "cert_file": "",
    "key_file": "",
    "database_user": "root",
    "database_password": "root",
    "database_host": "localhost",
    "database_port": 3306,
    "database_name": "mgstpp",
    "use_konami_auth": false,
    "vars": {
        "session_heartbeat": 60,
        "session_timeout": 200,
        "nuclear_find_probability": 1.0,
        "max_server_gmp": 25000000,
        "item_dev_limit": 4,
        "unlock_all_items": false
    }
}
```

### Config values

(`vars.*` values must be defined in a `"vars"` object)

| name | desc | default |
| --- | --- | --- |
| base_url | base url for the url list command | http://localhost:80 |
| http_port | https port | 80 |
| https_port | https port | 443 |
| cert_file | file path for ssl certificate | "" |
| key_file | file path for tls key | "" |
| database_user | database username | "root" |
| database_password | database password | "root" |
| database_host | database hostname | "localhost" |
| database_port | database port | 3306 |
| database_name | database name | "mgstpp" |
| use_konami_auth | relay authentication to konami servers, **if set to `false` steam tickets will not be verified** | false |
| vars.session_heartbeat | interval between each CMD_UPDATE_SESSION sent by the client (in seconds) | 60 |
| vars.session_timeout | time until client is considered offline (seconds) | 200 |
| vars.nuclear_find_probability | probability that a player with nukes will appear in the nuke tab (0.0-1.0) | 1.0 |
| vars.max_server_gmp | maximum server gmp | 25000000 |
| vars.item_dev_limit | maximum number of concurrent items in development | 4 |
| vars.unlock_all_items | unlock all items | false |
| vars.cost_factor_generic | factor to calculate mb coins to reduce dev time (remaning_seconds * factor) | 0.01565 |
| vars.cost_factor_item_dev | factor to calculate mb coins to reduce dev time of **items** | vars.cost_factor_generic |
| vars.cost_factor_platform_construction | factor to calculate mb coins to reduce dev time of **platform construction** | vars.cost_factor_generic |

## Resource files

Files in https://github.com/fedddddd/tpp-server-emulator/tree/main/src/server/resources/data can be overridden if they exist on the disk.  
These files contain data that is sent to the client, such as item lists, fob area lists, ...  
Example file structure:

```
[root]
├── tpp-server-emulator.exe
├── config.json
├── allow_list.json
├── deny_list.json
├── libcrypto-3-x64.dll
├── libmysql.dll
├── libssl-3-x64.dll
└── resources
    └── data
        └── area_list.json  
```

## Allow/Deny lists

You can filter players connecting to your server using by creating `allow_list.json` or `deny_list.json` in your server folder  
These files must contain an array of steam ids to either allow or deny  
If the allow list doesn't exist all players will be allowed, if its an empty array no one will be allowed  
If the deny list doesn't exist or is empty no players will be denied  
Deny list takes priority over the allow list  

Example:

`allow_list.json`
```json
[
   76560000000001337,
   76560000000001339,
]
```

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
