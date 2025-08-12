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

#### scripting

---

- *F* **`scripting.onevent(eventname: string, callback: fn)`**: registers an event handler
- *F* **`scripting.dispatchevent(eventname: string, args)`**: dispatches an event
- *F* **`scripting.ontimeout(callback: fn, ms: int)`**: executes callback after set delay
- *F* **`scripting.oninterval(callback: fn, ms: int)`**: executes callback after set delay (looping)

#### [json](https://github.com/alicealys/tpp-server-emulator/blob/main/src/server/scripting/types/json.cpp)

Tthe json library used by this project is **nlohmann::json**, this type is exposed to the lua scripts  
The values can be accessed using the bracket `[]` operator  

---
- *F* **`json.parse(text: string)`**: parses json string into nlohmann::json
- *F* **`json.convert(value)`**: convert lua value into nlohmann::json
- *F* **`json.array()`**: returns nlohmann::json::array
- *F* **`json.object()`**: returns nlohmann::json::object
- *M* **`json:get()`**: converts json value into lua primitive (if possible)
- *M* **`json:dump([, indent: int])`**: serializes json value into a string

#### [server](https://github.com/alicealys/tpp-server-emulator/blob/main/src/server/scripting/types/server.cpp)
---
- *F* **`server.registercommand(name: string, callback: fn)`**: registers a server command
- *F* **`server.executehandler()`**: executes original c++ code for the current command being executed (only works in the context of server.registercommand's callback)
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

### Examples

- Creating a command handler
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

    print(request:dump(), player)

    return result
end)
```

-- Using `server.executehandler`
```lua
server.registercommand("CMD_REQAUTH_HTTPS", function(request, player)
    local result = server.originalhandler()
    print(result:dump())
    return result
end)
```

- Using database functions
```lua
-- execute custom query
local result = database.execute("update `player_records` set `player_records`.`fob_point` = ? where `player_records`.`player_id` = ?", 99999, 1)
print(result:dump(4))

-- use predefined database function
local player = database.players.find(1) -- find player with `id` = 1
print(player:getaccountid()) -- returns player's account id (steam id if using konami auth)

local playerdata = database.playerdata.find(player:getid(), true, true, true) -- gets player's player data (and optionally parses motherbase, loadout, emblem)
local loadout = playerdata:getloadout() -- gets loadout
print(loadout:dump(4))

loadout["hand"]["id"] = 1337 -- not sure what this is
database.playerdata.syncloadout(playerdata:getplayerid(), loadout) -- updates player's loadout
```

- Using *scheduler* and *event handler*
```lua
-- prints "hello" every 1 second
scripting.oninterval(function()
    print("hello")
end, 1000)

-- prints "hello" when event: "event_name" is dispatched
scripting.onevent("event_name", function(arg1)
    print("hello", arg1)
end)

scripting.dispatchevent("event_name", "argument")
```

- Using json api
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

local j2 = json.convert({
    cat = 1,
    dog = {
        giraffe = 2
    }
})

print(j2:dump())
```