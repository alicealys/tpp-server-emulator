mongoose = {
	source = path.join(dependencies.basePath, "mongoose"),
}

function mongoose.import()
	links "mongoose"

	mongoose.includes()
end

function mongoose.includes()
	includedirs {
		mongoose.source,
	}

	defines {
		"MG_TLS=1"
	}
end

function mongoose.project()
	project "mongoose"
		language "C"

		mongoose.includes()
		mbedtls.import()

		files {
			path.join(mongoose.source, "mongoose.c"),
			path.join(mongoose.source, "mongoose.h"),
		}

		warnings "Off"

		kind "StaticLib"
end

table.insert(dependencies, mongoose)
