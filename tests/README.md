# tests

Pruebas automatizadas de dependencias, entrega y procedencia. Aquí se esperan pruebas aisladas, sin modificar el repositorio de trabajo ni conectarse a una placa.

`lwip-source-tests.ps1` prueba las cuatro combinaciones original/personalizado, integridad de las copias mantenidas, archivos ausentes, cambios de bytes y diagnósticos. Ejecuta además Setup/Build/Open con un origen inválido para comprobar que el rechazo sucede antes de modificar un workspace. Usa directorios temporales y no necesita instalar Vitis:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/lwip-source-tests.ps1
```

- `release-tests.ps1`: verifica versiones numéricas, Git sucio, huella del workspace y procedencia del Build. Comprueba que fuentes, BSP, lwIP y hardware activo invalidan lo que corresponde; documentación, empaquetado, flash y hardware inactivo no fuerzan una recompilación. También cubre productos, herramienta, entradas ignoradas sin versionar y registros/workspaces anteriores compatibles. Crea un repositorio temporal y lo retira al terminar. No necesita Vitis.
- `clean-app-tests.tcl`: comprueba que la limpieza retira objetos/dependencias de las aplicaciones, incluso anidados, y conserva fuentes, makefiles y BSP. Usa un directorio temporal y no genera un workspace.

Desde la raíz:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/release-tests.ps1
```

La prueba Tcl se puede ejecutar con Tcl 8.6 o con XSCT de la instalación Xilinx:

```powershell
. .\scripts\vitis.ps1
$vitis = Resolve-VitisHome
& (Join-Path $vitis 'bin\xsct.bat') tests/clean-app-tests.tcl
```

La generación, compilación y Bootgen reales se validan aparte en un clon/workspace de pruebas con Vitis 2022.2. Estas pruebas rápidas no sustituyen la integración ni la validación en placa.

[Volver](../README.md)
