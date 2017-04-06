project(SPASM-ng)
cmake_minimum_required(VERSION 2.9.5)

set(spasm_SRCS
	main.cpp
	opcodes.cpp
	pass_one.cpp
	pass_two.cpp
	utils.cpp
	export.cpp
	preop.cpp
	directive.cpp
	console.cpp
	expand_buf.cpp
	hash.cpp
	list.cpp
	parser.cpp
	storage.cpp
	errors.cpp
	bitmap.cpp
	modp_ascii.cpp
	opcodes_ez80.cpp
)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_LIST_DIR}/cmake
    ${CMAKE_MODULE_PATH}
)

#set(APPSIGN)

if(APPSIGN)
	find_package(GMP REQUIRED)
	include_directories(${GMP_INCLUDE_DIR})
	
	message(STATUS "Found GMP include: " ${GMP_INCLUDE_DIR})
	message(STATUS "Found GMP library: " ${GMP_LIBRARIES})
	
	find_package(OpenSSL REQUIRED)
	include_directories(${OPENSSL_INCLUDE_DIR})
	
	set(APPSIGN_LIBRARIES ${GMP_LIBRARIES} ${OPENSSL_LIBRARIES} ${CMAKE_CURRENT_LIST_DIR}/lib/mpir.lib)
else()
	set(NO_APPSIGN -DNO_APPSIGN)
endif()

# uncomment if problems arise;
#if(MINGW)
#	set(PLATFORM_DEFINE -DMINGWVER)
#elseif(UNIX)
#	set(PLATFORM_DEFINE -DUNIXVER)
#endif()

add_executable(spasm ${spasm_SRCS})
target_compile_options(spasm PUBLIC -DUSE_REUSABLES -DUSE_BUILTIN_FCREATE ${NO_APPSIGN} ${PLATFORM_DEFINE})
target_link_libraries(spasm -lm ${APPSIGN_LIBRARIES})
