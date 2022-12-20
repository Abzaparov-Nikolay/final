 # Проверка версии CMake.
									# Если версия установленой программы
									# старее указаной, произайдёт аварийный выход.

		# Создает исполняемый файл с именем main
									# из исходника main.cpp

									cmake_minimum_required(VERSION 2.8)

									project(final)
									
									set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
									set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
									
									set (LIBS "-pthread")
									
									aux_source_directory(src SRCS)
									
									set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
									set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpermissive")
									
									add_executable(${PROJECT_NAME} ${SRCS})
									
									target_link_libraries(${PROJECT_NAME} ${LIBS})