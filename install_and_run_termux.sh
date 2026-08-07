#!/bin/bash
set -e

echo -e "\033[1;36m========================================================\033[0m"
echo -e "\033[1;32m      Instalando y Compilando Nuby en Termux / Linux     \033[0m"
echo -e "\033[1;36m========================================================\033[0m"

# Verificar dependencias
if ! command -v clang++ &> /dev/null && ! command -v g++ &> /dev/null; then
    echo "Instalando compilador C++ (clang / make)..."
    pkg install clang make git -y || true
fi

# Navegar al directorio de Nuby
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [ -d "$SCRIPT_DIR/nuby_engine" ]; then
    cd "$SCRIPT_DIR/nuby_engine"
else
    cd "$SCRIPT_DIR"
fi

echo "Compilando el motor Nuby en C++20 con optimización -O3..."
make all

echo -e "\n\033[1;32m[✔] ¡Compilación exitosa!\033[0m"
echo -e "Iniciando Nuby en el puerto 8080...\n"
echo -e "Abre tu navegador en: \033[1;34mhttp://localhost:8080\033[0m o \033[1;34mhttp://127.0.0.1:8080\033[0m\n"

./bin/nuby_engine --port 8080
