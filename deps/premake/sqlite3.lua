sqlite3 = {
	source = path.join(dependencies.basePath, "sqlite3"),
	foldername = "sqlite-amalgamation-3500400",
	download = "https://www.sqlite.org/2025/sqlite-amalgamation-3500400.zip"
}

function sqlite3.install_linux()
	local hfile = io.open(string.format("%s/sqlite3.h", sqlite3.source), "r")
	if (hfile) then
		return
	end

    os.execute(string.format("mkdir \"%s\" 2> /dev/null", sqlite3.source))

	local folder = path.join(sqlite3.source, sqlite3.foldername)
	local archive = path.join(sqlite3.source, sqlite3.foldername .. ".zip")

	print("Downloading sqlite3")
	os.execute(string.format("wget \"%s\" -O \"%s\"", sqlite3.download, archive))
	os.execute(string.format("unzip \"%s\" -d \"%s\"", archive, sqlite3.source))

	os.execute(string.format("mv \"%s\"/* \"%s\"", folder, sqlite3.source))
	os.execute(string.format("rm \"%s\"", archive))
	os.execute(string.format("rmdir \"%s\"", folder))
end

function sqlite3.install_win()
	local hfile = io.open(string.format("%s/sqlite3.h", sqlite3.source), "r")
	if (hfile) then
		return
	end

    os.execute(string.format("mkdir \"%s\" 2> nul", sqlite3.source))

	local folder = path.join(sqlite3.source, "sqlite-amalgamation-3500400")
	local archive = path.join(sqlite3.source, "sqlite-amalgamation-3500400.zip")

	print("Downloading sqlite3")
	os.execute(string.format("curl \"%s\" -L -o \"%s\"", sqlite3.download, archive))

    os.execute(string.format("powershell -command \"Expand-Archive -Force \\\"%s\\\" \\\"%s\\\"\"", archive, sqlite3.source))
    os.execute(string.format("powershell -command \"mv \\\"%s/*\\\" \\\"%s\\\"\"", folder, sqlite3.source))
    os.execute(string.format("powershell -command \"rm \\\"%s\\\"\"", archive))
    os.execute(string.format("rmdir \"%s\"", folder))
end

function sqlite3.install()
	if (os.host() == "windows") then
		return sqlite3.install_win()
	elseif (os.istarget("linux")) then
		sqlite3.install_linux()
	else
		error("automatic sqlite3 installation is not supported on your os")
	end
end

function sqlite3.import()
	links { "sqlite3" }

	sqlite3.install()
	sqlite3.includes()
end

function sqlite3.includes()
	includedirs {
		sqlite3.source
	}
end

function sqlite3.project()
	project "sqlite3"
		language "C"

		sqlite3.includes()

		files {
			path.join(sqlite3.source, "*.h"),
			path.join(sqlite3.source, "*.c"),
		}
		
		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, sqlite3)
