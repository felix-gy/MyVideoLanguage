cmake_minimum_required(VERSION 3.30)
project(CPP_Compiler)

set(CMAKE_CXX_STANDARD 20)

# Configuración específica para tu instalación de Python
set(PYTHON_EXECUTABLE "C:/Python310/python.exe")
set(PYTHON_INCLUDE_DIR "C:/Python310/include")
set(PYTHON_LIBRARY "C:/Python310/libs/python310.lib")
# Configuración específica para tu instalación de Python
#set(PYTHON_EXECUTABLE "C:/Users/gordi/AppData/Local/Programs/Python/Python313/python.exe")
#set(PYTHON_INCLUDE_DIR "C:/Users/gordi/AppData/Local/Programs/Python/Python313/include")
#set(PYTHON_LIBRARY "C:/Users/gordi/AppData/Local/Programs/Python/Python313/libs/python313.lib")
find_package(Python REQUIRED COMPONENTS Development)


add_executable(CPP_Compiler main.cpp
        Scanner.cpp
        Scanner.h
        Token.h
        Parser.cpp
        Parser.h
        Grammar.cpp
        Grammar.h
        archivos.h
        PythonCodeGenerator.cpp
        PythonCodeGenerator.h)



# Configurar includes y librerías


target_include_directories(CPP_Compiler PRIVATE ${PYTHON_INCLUDE_DIR})
target_link_directories(CPP_Compiler PRIVATE ${PYTHON_LIBRARY})
target_link_libraries(CPP_Compiler PRIVATE ${PYTHON_LIBRARY})