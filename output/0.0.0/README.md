# Entrega 0.0.0

Commit de origen: 4bfacd12279b915bc46bae312ff6e3592902ca6d.

- cpu0-boot.bin: FSBL + bitstream UCI + CPU0; arranque en flash.
- cpu1-network.bin: solo CPU1; carga por red en RAM. No grabar CPU1 en flash.
- programming/fsbl.elf: auxiliar temporal para grabar la QSPI mediante JTAG.
- debug/CPU0.elf y debug/CPU1.elf: ejecutables con simbolos de esta compilacion.
- manifest.json: procedencia, herramientas, entradas, hashes, despliegue y particiones.

Desde la raiz del repositorio, usar grabar-flash.bat para seleccionar y programar una entrega. La generacion no certifica pruebas en placa. Conservar esta carpeta sin modificar.

[Volver](../README.md)
