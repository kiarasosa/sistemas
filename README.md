# Laboratorio 1. "Mybash" en C.
Este proyecto ejecuta un shell llamado mybash. Tiene la capacidad de efectuar pipelines y redirecciones (entrada/salida).

## Instalación
Para compilar y ejecutar este proyecto, necesitarás tener instalado un compilador de C (como gcc) y make.
1. Clona el repositorio:
    git clone https://github.com/tu-usuario/tu-repositorio.git
    cd tu-repositorio
    
2. Compila el proyecto, dentro del directorio kickstart:
    make
    
3. Ejecuta los tests general, dentro del directorio kickstart:
    make test
    
## Ejecución
Para ejecutar el shell mybash:
./mybash

## Ayuda
Para obtener ayuda acerca de los comandos: 
help

## Contenido del proyecto
El proyecto mybash incluye una carpeta llamada "so24lab1g48", cuyo contenido será el siguiente:
so24lab1g48 contiene un directorio kickstart, dentro de la misma se encuentran varios archivos .c y un Makefile. Los .c son los archivos fuente de este shell. 
Se encuentra también un directorio tests, cuyo contenido también son archivos .c, pero su onjetivo es checkear componentes de la shell, como así también el programa completo.
Los Makefile son para compilar y ejecutar el proyecto.

## Consideraciones
Al ejecutar "echo -en "ls\nexit\n" | ./mybash", el prompt de "mybash>" aparece antes de salir, puede que se esté ejecutando otra iteración antes de terminar.

## Contribución
Este proyecto fue diseñado por los siguientes integrantes del grupo 48 de Sistemas Operativos, FaMAF:
Salman Mauro, mauro.salman@mi.unc.edu.ar
Lionel Flores, lionel.flores@mi.unc.edu.ar
Sosa Kiara, kiarasosa@mi.unc.edu.ar
