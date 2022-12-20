 # Проверка версии CMake.
									# Если версия установленой программы
									# старее указаной, произайдёт аварийный выход.

		# Создает исполняемый файл с именем main
									# из исходника main.cpp

cmake_minimum_required(VERSION 2.8)
project(final)
									
set(CMAKE_CXX_STANDARD 14)
									
add_executable(final main.cpp)
									
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")