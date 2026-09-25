# tests

Pruebas automatizadas de las reglas de entrega y procedencia. Aquí se esperan pruebas aisladas, sin modificar el repositorio de trabajo ni conectarse a una placa.

- `release-tests.ps1`: verifica versiones numéricas, formatos rechazados, Git sucio, cambios de fuentes, binarios, herramientas y commit. Comprueba que añadir, modificar o retirar respaldos `*.bak` no afecta al registro y que una cabecera ignorada sin versionar sigue bloqueando la entrega. Crea un repositorio temporal y lo retira al terminar. No necesita Vitis.
- `clean-app-tests.tcl`: comprueba que la limpieza retira objetos/dependencias de las aplicaciones, incluso anidados, y conserva fuentes, makefiles y BSP. Usa un directorio temporal y no genera un workspace.

Desde la raíz:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/release-tests.ps1
```

La prueba Tcl se puede ejecutar con Tcl 8.6 o con XSCT de la instalación Xilinx:

```powershell
& 'E:\Xilinx\Vitis\2022.2\bin\xsct.bat' tests/clean-app-tests.tcl
```

La generación, compilación y Bootgen reales se validan aparte en un clon/workspace de pruebas con Vitis 2022.2. Estas pruebas rápidas no sustituyen la integración ni la validación en placa.

[Volver](../README.md)
