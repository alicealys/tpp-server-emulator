if (os.host() ~= "windows") then
    error("automatic mysql installation is not supported on your os")
end

mysql = {
	source = path.join(dependencies.basePath, "mysql"),
	version = "8.1.0",
	download = "https://dev.mysql.com/get/Downloads/MySQL-8.1/mysql-8.1.0-winx64.zip",
}

function mysql.install()
	local hfile = io.open(string.format("%s/include/mysql.h", mysql.source), "r")
	if (hfile) then
		return
	end

    os.execute(string.format("mkdir \"%s\" 2> nul", mysql.source))

	local folder = path.join(mysql.source, "mysql-8.1.0-winx64")
	local archive = path.join(mysql.source, "mysql-8.1.0-winx64.zip")

	print("Downloading MYSQL")
	os.execute(string.format("powershell -command \"curl \\\"%s\\\" -o \\\"%s\\\"\"", mysql.download, archive))

    os.execute(string.format("powershell -command \"Expand-Archive -Force \\\"%s\\\" \\\"%s\\\"\"", archive, mysql.source))
    os.execute(string.format("powershell -command \"mv \\\"%s/*\\\" \\\"%s\\\"\"", folder, mysql.source))
    os.execute(string.format("powershell -command \"rm \\\"%s\\\"\"", archive))
    os.execute(string.format("rmdir \"%s\"", folder))
end

function mysql.import()
	mysql.install()
	mysql.includes()
end

function mysql.includes()
	includedirs {
		path.join(mysql.source, "include"),
	}
end

function mysql.project()
	project "mysql"
		language "C"

		mysql.includes()

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, mysql)
