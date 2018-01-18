cmake_minimum_required (VERSION 2.8.1)

######## Project settings ########
PROJECT(i2c_poll)
SET(LICENSE "TBD")

######## Build and include settings ########
include_directories(
	inc
)

link_directories(
	${LINK_DIRECTORIES}
)


file(GLOB SOURCES
	"src/*.cpp"
)

add_executable(
	i2c_poll
	${SOURCES}
)

TARGET_LINK_LIBRARIES(
	i2c_poll
	pq
	pthread
)

######## Install targets ########
INSTALL(TARGETS i2c_poll
	RUNTIME DESTINATION usr/bin
)
