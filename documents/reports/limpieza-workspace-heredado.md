# Limpieza del workspace heredado

**Actualización 24/09/2026:** tras la limpieza de bootimage, el usuario autorizó retirar también los nueve Tcl y los dos `.launch` heredados. Se eliminaron los contenedores `Both_CPUs_system` y `CPU1_system` completos, incluidos los README residuales. El respaldo original permanece intacto. La depuración JTAG portable queda para un trabajo futuro; la generación, compilación y empaquetado actuales no dependen de estos archivos. El texto siguiente describe el estado histórico anterior.

23 de septiembre de 2026.

Tras validar la GUI del workspace externo, se retiraron del repositorio las carpetas
CPU0, CPU1, Platform, .metadata, .Xil y RemoteSystemsTempFiles, el XSA suelto, los logs
y temporales de la raíz, y los metadatos y Debug de los dos proyectos de sistema.
Ninguno de los elementos retirados estaba en el índice actual de Git. Antes de borrar,
se verificó que todos los archivos de los antiguos src tenían copias idénticas en src/.
El respaldo sitau2_sw_uci y el workspace externo permanecen intactos.

Se conserva todo el contenido de ambos _ide, incluidos bootimage, scripts y launch.

## Revisión preliminar de bootimage

- Both_CPUs_system.bif: FSBL + bitstream + CPU0; contiene rutas absolutas C:/PV del
  equipo original. Los binarios locales BOOT.bin y Flash_v2_0_8.bin se conservan.
- CPU1_system.bif: solo CPU1. No equivale al BOOT.BIN automático nuevo, que incluye
  también FSBL y bitstream. Su uso como imagen de actualización debe investigarse.
- CPU1_system/_ide/bootimage/BIN mezcla variantes MK32 (.bin, .bit y .ltx), imágenes
  BOOT, configuraciones XML/CSV, comprimidos, logs y una hoja de cálculo. Los nombres
  de carpetas no demuestran qué variante es operativa.

Propuesta pendiente, sin mover ni borrar estos archivos:

1. Identificar el procedimiento real de carga, actualización y arranque, especialmente
   el consumidor de la imagen que contiene solo CPU1.
2. Mantener las recetas necesarias como configuración reproducible, parametrizando
   las rutas al workspace externo y generando sus resultados fuera del repositorio.
3. Incorporar las variantes MK32 confirmadas como paquetes inmutables de artifacts,
   con hashes y procedencia; no seleccionar una variante solo por su nombre.
4. Separar la configuración del equipo de los binarios y trasladar los históricos
   descartados a un archivo externo documentado, después de confirmar su utilidad.

Los scripts de depuración y launch se revisarán aparte, antes de retirar los _ide.
