mbedtls = {
	source = path.join(dependencies.basePath, "mbedtls"),
	extra = path.join(dependencies.basePath, "extra/mbedtls"),
}

function mbedtls.import()
	links { "mbedtls" }
	mbedtls.includes()
end

function mbedtls.includes()
	includedirs {
		path.join(mbedtls.source, "include")
	}
end

function mbedtls.project()
	project "mbedtls"
		language "C"

		mbedtls.includes()

		files {
			path.join(mbedtls.source, "library/*.h"),
			path.join(mbedtls.source, "library/*.c"),
			path.join(mbedtls.extra, "*.c"),
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, mbedtls)
