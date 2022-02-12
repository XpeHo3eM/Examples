add_executable (Server 
				main.cpp    server.cpp)

target_include_directories (Server PUBLIC ${MY_INCLUDE_PATH})