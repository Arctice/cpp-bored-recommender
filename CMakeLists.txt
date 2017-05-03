cmake_minimum_required (VERSION 3.7.2)
project (anime_birb_recommender)

add_subdirectory(vs)
include_directories("${PROJECT_BINARY_DIR}")
set(SRCDIR ${PROJECT_BINARY_DIR}/vs)

set(SOURCES 
	${SRCDIR}/main.cpp 
	${SRCDIR}/data_store.h 
	${SRCDIR}/data_store.cpp 
	${SRCDIR}/recommendations.h 
	${SRCDIR}/recommendations.cpp 
	${SRCDIR}/user_data.h 
	${SRCDIR}/user_data.cpp
    ${SRCDIR}/config.h)
add_executable(anime_recommender_cpp ${SOURCES})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(/usr/local/include)
find_library(CPP_REDIS_LIB cpp_redis HINTS /usr/local/lib)
find_library(TACOPIE_LIB tacopie HINTS /usr/local/lib)

target_link_libraries(anime_recommender_cpp PUBLIC
	${CPP_REDIS_LIB} ${TACOPIE_LIB} Threads::Threads)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

