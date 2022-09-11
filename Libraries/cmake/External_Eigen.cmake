ExternalProject_Add( EIGEN
	URL "https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz"
	URL_HASH "SHA1=d222db69a9e87d9006608e029d1039039f360b52"
	UPDATE_COMMAND ""
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
)
ExternalProject_Get_Property( EIGEN SOURCE_DIR )
SET( EIGEN_INSTALL_DIR ${SOURCE_DIR} )