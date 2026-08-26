# Usage

## Installation

- Download binary 
  - **[Windows x64](https://github.alicent.cat/tpp-server-emulator/tpp-server-emulator-windows_x64.tar)**  
  - **[Linux x64](https://github.alicent.cat/tpp-server-emulator/tpp-server-emulator-linux_x64.tar)**  
- \***OPTIONAL**\* Setup a mysql server with a database called `mgstpp`.
- In the EXE's path create a file called `config.json` and add the following values:
    * `database_type`: (default: `"mysql"`, options: "mysql", "sqlite3")
    * `database_user`: (default: `"root"`)
    * `database_password`: (default: `"root"`)
    * `database_host`: (default: `"localhost"`)
    * `database_port`: (default: `3306`)
    * `database_name`: (default: `"mgstpp"`)
- TPP:
  Change the string `"https://mgstpp-game.konamionline.com/tppstm/gate"` in `mgsvtpp.exe` to `"{your base url}/tppstm/gate"`.  
- MGO:
  Change the string `"https://mgstpp-game.konamionline.com/mgostm/gate"` in `mgsvmgo.exe` to `"{your base url}/mgostm/gate"`.  
  Alternatively (recommended), run [tpp-mod](https://github.com/alicealys/tpp-mod) with the `+net_custom_server <url>` command line param.

## Config

Example config

```json
{
    "base_url": "http://localhost:80",
    "http_port": 80,
    "https_port": 443,
    "cert_file": "",
    "key_file": "",
    "database_type": "sqlite3",
    "database_password": "root",
    "database_name": "mgstpp",
    "auth_mode": "offline",
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
| database_type | database type (mysql, sqlite3) | "mysql" |
| database_user | database username | "root" |
| database_password | database password | "root" |
| database_host | database hostname | "localhost" |
| database_port | database port | 3306 |
| database_name | database name | "mgstpp" |
| auth_mode | authentication mode, must be one of: "offline", "konami", "custom", "hybrid", **if set to `offline` steam tickets will not be verified** | "offline" |
| vars.session_heartbeat | interval between each CMD_UPDATE_SESSION sent by the client (in seconds) | 60 |
| vars.session_timeout | time until client is considered offline (seconds) | 200 |
| vars.nuclear_find_probability | probability that a player with nukes will appear in the nuke tab (0.0-1.0) | 1.0 |
| vars.wormhole_duration | wormhole duration in days | 31 |
| vars.max_server_gmp | maximum server gmp | 25000000 |
| vars.item_dev_limit | maximum number of concurrent items in development | 4 |
| vars.unlock_all_items | unlock all items | false |
| vars.cost_factor_generic | factor to calculate mb coins to reduce dev time (remaning_seconds * factor) | 0.01565 |
| vars.cost_factor_item_dev | factor to calculate mb coins to reduce dev time of **items** | vars.cost_factor_generic |
| vars.cost_factor_platform_construction | factor to calculate mb coins to reduce dev time of **platform construction** | vars.cost_factor_generic |
| vars.cost_factor_troops_completion | factor to calculate mb coins to reduce dev time of **troops completion** | vars.cost_factor_generic |
| vars.server_version_tpp | tpp server version | 18 |
| vars.server_version_mgo | mgo server version | 15 |
| vars.signup_bonus | award new players with resources and fobs | false |
| vars.no_fob_damage | disable fob infiltration damage | false |
| vars.pvp_mode | enable pvp mode | false |
| vars.run_pf_league | enable pf league logic | true |
| vars.use_real_client_ip | store actual client ip adress (workaround for steam networking p2p) | false |

### Authentication mode

tpp-server-emulator allows for 4 different authentication modes to verify players that log in:

- **offline**: no verification is done, steam id is extracted from the steam ticket.
- **konami**: steam ticket is relayed to KONAMI servers, and verified.
- **custom**: requires a custom steam ticket, and for the user to have linked their steam account to the server.  
    the structure for the ticket is the following:
    ```c
    struct auth_ticket_custom_t
    {
    	std::uint64_t account_id; // steam id
    	char auth_token[32]; 
    }; // sizeof(auth_ticket_custom_t) == 40
    ```
    The auth token must be generated using Steam OpenID, by setting "/api/steam_openid" as return address.   
    This API will return the following response if valid:
    ```json
    {
        "is_valid": true,
        "account_id": 76561197960287930,
        "auth_token": "0123456789ABCDEF0123456789ABCDEF"
    }
    ```

    The auth token will be valid for 365 days, or until a new one is generated.

    The game client must then send a ticket with this information using the format described above in the **CMD_AUTH_STEAMTICKET** request.  
    An example of the redirect URL you should provide the client:

    ```
    https://steamcommunity.com/openid/login?openid.ns=http://specs.openid.net/auth/2.0&
    openid.claimed_id=http://specs.openid.net/auth/2.0/identifier_select&
    openid.identity=http://specs.openid.net/auth/2.0/identifier_select&
    openid.return_to=http://your-server/api/steam_openid&
    openid.realm=https://your-server&
    openid.mode=checkid_setup
    ```
- **hybrid**: automatically chooses **konami** or **custom** authentication based on ticket size.

## Web API

tpp-server-emulator exposes some web APIs that can be accessed through regular HTTP requests (not using the game's protocol or format).  
The API is enabled by default, but can be disabled by setting `enable_web_api` to `false` in the config.  
All APIs return in JSON format.  

### API list

- `api/steam_openid?{Steam OpenID redirect url}`: generates a new auth token from a valid Steam OpenID redirect url.
- `api/steam_openid_url?target={target endpoint}`: formats the Steam OpenID redirect url given a target api.
- `api/status`: returns some server status information (player count, current event, uptime, ...).
- `api/delete_account?{Steam OpenID redirect url}`: allows a user to delete all their data, requires a valid Steam OpenID redirect url.

## Resource files

Files in https://github.com/alicealys/tpp-server-emulator/tree/main/src/server/resources/data can be overridden if they exist on the disk.  
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
        └── ...
```

Most resource files should not be modified, however some can be customized to your liking:
- `tpp_informationlist2.json`: list of messages visible when loading tpp.  
- `mgo_informationlist2.json`: list of messages visible when loading mgo.  
    available format variables:
    - {online_players}: replaced with online player count
    - {total_players}: replaced with total player count

Feel free to explore the rest of the files and tweak any setting you want.

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

## Commands

- `set_motd_title`: Sets the message of the day title (visible in iDroid)
- `set_motd_text`: Sets the message of the day text
- `max_resources <player_id>`: Gives all resources to the player
- `reload_lists`: Reloads allow/deny lists
- `reload_scripts`: Reloads scripts
- `query`: Runs an SQL query (mysql only)
- `quit`: Shuts down the server

## Command line params

- `-config <path>`: sets the config file path
- `-working_dir <path>`: sets the working directory path
