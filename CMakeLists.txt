# Версия CMake
cmake_minimum_required(VERSION 3.15)

# Название проекта
project(gameServer VERSION 1.0)

# Стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Настройки директорий
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Настройки для исполняемого файла
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

# Генерация compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Опции компиляции
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Wpedantic -Werror -pthread")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra -Wpedantic -pthread")

# Сбор исходных файлов
file(GLOB SOURCES
    ${SRC_DIR}/*.cpp
)

# Поиск необходимых компонентов Qt
find_package(Qt5 REQUIRED COMPONENTS Core Network Sql)
set(CMAKE_AUTOMOC ON)

# Проверка версии Qt
if(Qt5_VERSION VERSION_LESS "5.12")
  message(FATAL_ERROR "Qt5 version must be at least 5.12")
endif()

# Создание исполняемого файла
add_executable(${PROJECT_NAME} ${SOURCES})

# Линковка библиотек Qt и папки include
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Network Qt5::Sql)
target_include_directories(${PROJECT_NAME} PRIVATE ${INC_DIR})
