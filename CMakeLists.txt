cmake_minimum_required(VERSION 3.10)

project(Translater)

# Incluir el subdirectorio de libpqxx
add_subdirectory(externals/libpqxx)

# Crear el ejecutable
add_executable(Translater ./src/main.cpp)

# Vincular la biblioteca libpqxx
target_link_libraries(Translater pqxx)


# Definir el path de los archivos de audio
target_compile_definitions(Translater PRIVATE AUDIO_PATH="${CMAKE_SOURCE_DIR}/src/sounds/")

# Copiar los archivos de sonido al directorio de compilación solo si cambia algo
add_custom_command(TARGET Translater POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src/sounds ${CMAKE_BINARY_DIR}/src/sounds
    COMMENT "Copiando archivos de sonido al directorio de salida..."
)
