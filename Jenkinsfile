node('cmake') {
	// Mark the code checkout 'stage'....
	stage 'Checkout'

	// Get some code from a GitHub repository
	checkout scm

	stage 'CMake Build'

	sh '''
		mkdir -p build
		cd build
		LUA_DIR=/usr/local cmake -G "Unix Makefiles" -DLIBBSON_DIR=/usr/local -DLIBMONGOC_DIR=/usr/local -DCMAKE_INSTALL_PREFIX=/var/lib/builds/microhelix/tmp -DMICROHELIX_INSTALL_DIR=/usr/local ../
	'''
}