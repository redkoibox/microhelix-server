node('cmake') {
	stage 'Checkout'

	checkout scm

	stage 'CMake Build'

	sh '''
		ME_VERSION=$(cat version.txt)
		mkdir -p build
		cd build
		LUA_DIR=/usr/local cmake -G "Unix Makefiles" -DLIBBSON_DIR=/usr/local -DLIBMONGOC_DIR=/usr/local -DCMAKE_INSTALL_PREFIX=/var/lib/builds/microhelix/tmp -DMICROHELIX_INSTALL_DIR=/usr/local ../
		mkdir -p /var/lib/builds/microhelix/archive
		cd /var/lib/builds/microhelix/tmp
		tar czvf /var/lib/builds/microhelix/archive/microhelix-$ME_VERSION.tar.gz ./*
	'''
}
node('docker') {
	stage 'Checkout'
	
	checkout scm
	
	sh '''
		ME_VERSION=$(cat version.txt)
		cd docker
		chmod +x launch_microhelix.sh
		rm -f microhelix-latest.tar.gz
		wget http://jenkins.iseocom.it:8081/microhelix/archive/microhelix-$ME_VERSION.tar.gz
		mv microhelix-$ME_VERSION.tar.gz microhelix-latest.tar.gz
		docker build --tag docker-repo.iseocom.it:5000/microhelix:$ME_VERSION .
		docker tag docker-repo.iseocom.it:5000/microhelix:$ME_VERSION docker-repo.iseocom.it:5000/microhelix:latest
		docker push docker-repo.iseocom.it:5000/microhelix:$ME_VERSION
		docker push docker-repo.iseocom.it:5000/microhelix:latest
	'''
}