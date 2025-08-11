# Scripting

**tpp-server-emulator** allows you to handle server commands through **lua** scripts  
Scripts must be placed in a folder called `scripts` and all of them will be loaded automatically after server startup.  
You can reload the scripts using the `reload_scripts` command in the console  

All scripts belong to the same state/context which is thread-safe  
(this means commands defined in script are executed synchronously).  

#### Folder structure example:
```
root
├── tpp-server-emulator.exe
├── ...
└── scripts
   └── script.lua
```

#### Example script:

```lua
server.registercommand("CMD_GET_STEAM_SHOP_ITEM_LIST", function(request, player)
    print("executing cmd_get_svrtime")

    local result = json.object()

    result["result"] = "NOERR"
    result["list"] = json.array()
    result["list"][0] = json.object()
    result["list"][0]["product_name"] = "this is a test"
    result["list"][0]["steam_item_id"] = 1
    result["list"][0]["mb_coins"] = 9999

    return result
end)
```

**request: nlohmann::json**: request sent by the client  
**player: database::players::player**: player instance, can be nil  

This handles the **STORE** button in the pause menu

### API list

You can check the c++ source code to find all the methods/functions, some of them will be listed here  
**NOTE**: all methods are lowercase with no underscores, 

#### [json](https://github.com/alicealys/tpp-server-emulator/blob/main/src/server/scripting/types/json.cpp)

Tthe json library used by this project is **nlohmann::json**, this type is exposed to the lua scripts  
The values can be accessed using the bracket `[]` operator:

```lua
local j = json:new()

j["f1"] = "v1"
j["f2"] = 1

j["f3"] = json:array()
j["f3"][0] = "v2"

print(j:dump())

print(j["f1"]) -- "sol.nlohmann::json"
print(j["f1"]:get() == "v1") -- true
print(j["f2"]:get() == 1) -- true

```

---
- *F* **`json.parse(text: string)`**: parses json string into nlohmann::json
- *F* **`json.array()`**: returns nlohmann::json::array
- *F* **`json.object()`**: returns nlohmann::json::object
- *M* **`json:get()`**: converts json value into lua primitive (if possible)
- *M* **`json:dump([, indent: int])`**: serializes json value into a string

#### [server](https://github.com/alicealys/tpp-server-emulator/blob/main/src/server/scripting/types/server.cpp)
---
- *F* **`server.registercommand(name: string, callback: fn)`**: registers a server command
- *F* **`server.geterror(error: int)`**: returns error string
- *F* **`server.getresource(resource: int)`**: return resource of that id
- *F* **`server.error`**: table with name -> id of errors
- *F* **`server.resource`**: table with name -> id of resources

#### [database](https://github.com/alicealys/tpp-server-emulator/blob/main/src/server/scripting/types/database.cpp)
---
- *F* **`database.vars`**: contains database variables that are defined in the config file

#### [player](https://github.com/alicealys/tpp-server-emulator/blob/main/src/server/scripting/types/player.cpp)
---
- *M* **`player:getid()`**: returns id of the player
- *M* **`player:getaccountid()`**: returns account id of the player  
...  
*and many more...*

#### [playerdata](https://github.com/alicealys/tpp-server-emulator/blob/main/src/server/scripting/types/playerdata.cpp)
---

#### [playerrecord](https://github.com/alicealys/tpp-server-emulator/blob/main/src/server/scripting/types/playerrecord.cpp)
---

#### [fob](https://github.com/alicealys/tpp-server-emulator/blob/main/src/server/scripting/types/fob.cpp)
---

#### [item](https://github.com/alicealys/tpp-server-emulator/blob/main/src/server/scripting/types/item.cpp)
---

#### [eventranking](https://github.com/alicealys/tpp-server-emulator/blob/main/src/server/scripting/types/eventranking.cpp)
---

#### [wormhole](https://github.com/alicealys/tpp-server-emulator/blob/main/src/server/scripting/types/wormhole.cpp)
---
