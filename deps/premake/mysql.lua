mysql = {
	source = path.join(dependencies.basePath, "mysql"),
	download_win = {
		url = "http://cdn.alicent.cat/mysql-9.7.1-winx64.zip",
		folder = "mysql-9.7.1-winx64"
	},
	download_linux = {
		url = "http://cdn.alicent.cat/mysql-9.7.1-linux-glibc2.28-x86_64-minimal.zip",
		folder = "mysql-9.7.1-linux-glibc2.28-x86_64-minimal"
	}
}

function mysql.install_linux()
	local hfile = io.open(string.format("%s/include/mysql.h", mysql.source), "r")
	if (hfile) then
		return
	end

    os.execute(string.format("mkdir \"%s\" 2> /dev/null", mysql.source))

	local folder = path.join(mysql.source, mysql.download_linux.folder)
	local archive = path.join(mysql.source, mysql.download_linux.folder .. ".zip")

	print("Downloading MySQL")
	os.execute(string.format("wget \"%s\" -O \"%s\"", mysql.download_linux.url, archive))
	os.execute(string.format("unzip \"%s\" -d \"%s\"", archive, mysql.source))

	os.execute(string.format("mv \"%s\"/* \"%s\"", folder, mysql.source))
	os.execute(string.format("rm \"%s\"", archive))
	os.execute(string.format("rmdir \"%s\"", folder))
end

function mysql.install_win()
	local hfile = io.open(string.format("%s/include/mysql.h", mysql.source), "r")
	if (hfile) then
		return
	end

    os.execute(string.format("mkdir \"%s\" 2> nul", mysql.source))

	local folder = path.join(mysql.source, mysql.download_win.folder)
	local archive = path.join(mysql.source, mysql.download_win.folder .. ".zip")

	print("Downloading MySQL")
	os.execute(string.format("curl \"%s\" -L -o \"%s\"", mysql.download_win.url, archive))

    os.execute(string.format("powershell -command \"Expand-Archive -Force \\\"%s\\\" \\\"%s\\\"\"", archive, mysql.source))
    os.execute(string.format("powershell -command \"mv \\\"%s/*\\\" \\\"%s\\\"\"", folder, mysql.source))
    os.execute(string.format("powershell -command \"rm \\\"%s\\\"\"", archive))
    os.execute(string.format("rmdir \"%s\"", folder))
end

function mysql.install()
	if (os.host() == "windows") then
		return mysql.install_win()
	elseif (os.istarget("linux")) then
		mysql.install_linux()
	else
		error("automatic MySQL installation is not supported on your os")
	end
end

function mysql.import()
	if (os.host() == "windows") then
		linkoptions {"/DELAYLOAD:libmysql.dll"}
	else
		libdirs {
			path.join(mysql.source, "lib")
		}

		links {
			"mysqlclient"
		}
	end

	defines {"MYSQL_SUPPORTED"}

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
