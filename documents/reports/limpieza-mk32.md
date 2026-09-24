# Limpieza de imágenes y selección MK32

**Actualización final (24/09/2026):** el usuario autorizó eliminar BIN completo, incluidos los tres originales MK32. Se verificó el paquete `artifacts/mk32/0.0.0` y se retiraron los 17 archivos restantes. Comprimidos, configuraciones, auxiliares y registros de BIN ya no quedan pendientes. Los detalles anteriores se conservan a continuación como historial; el JSON incluye esta eliminación final. El respaldo `sitau2_sw_uci` permanece intacto. También se retiraron por autorización del usuario las carpetas heredadas `CPU1_system/_ide/bootimage` y `Both_CPUs_system/_ide/bootimage`, incluido el README residual. Las recetas activas de `config/bootimage` se conservan.

24/09/2026. Por indicación del usuario, se retiraron del repositorio las cinco imágenes CPU1 que permanecían en BIN y las 29 subcarpetas de variantes MK32 (28 con tríos BIN/BIT/LTX y una vacía). Las otras imágenes CPU0/CPU1 ya habían sido archivadas fuera del repositorio en la limpieza anterior. No se modificaron respaldos ni workspaces externos.

El trío `MK32CH_basics_wrapper.{bin,bit,ltx}` de la raíz BIN se conserva allí y se copia como [MK32 0.0.0](../../artifacts/mk32/0.0.0/README.md). Sus hashes coinciden con el inventario original; corresponde al conjunto `736cb3958cdb`, también presente anteriormente en MK32_debug_mem. La copia mantiene intactos los bytes y agrupa los tres archivos con su manifiesto.

La revisión automática rechazó retirar los tres originales de BIN al interpretar que debían conservarse allí; se mantuvieron. No se realizó ese traslado.

El [registro JSON](limpieza-mk32.json) detalla copias, eliminaciones y hashes. Los informes de clasificación anteriores se conservan como inventarios históricos, no como descripción del árbol actual. `dependencies-lock.json` no cambia: este paquete no se consume al compilar UCI.

## Archivos conservados inicialmente en BIN y eliminados después

- Comprimidos: `impl_1.zip` y `impl_1.rar`. El ZIP contenía otra variante; el RAR sigue sin inspección interna.
- Configuraciones: seis XML (`SITAU.XML`, `_SITAU.XML`, `SITAU_daqsonics.XML`, `SITAU_hypertronix.XML`, `SITAU_NoAssignment.XML`, `SITAU_vernom.XML`) y `SITAU.TXT`.
- Auxiliares: `orden_canales.csv` y `Table.xlsx`.
- Registros: `LOG_SERVER_SEND.TXT` y `output_dasel_2024-03-11T14-23-36.csv`.

Después de esta limpieza también se retiraron, por autorización del usuario, los nueve scripts Tcl y los dos `.launch` heredados junto con sus contenedores de proyecto. Véase [el registro de limpieza del workspace](limpieza-workspace-heredado.md). La preparación de una depuración JTAG portable queda para un trabajo futuro.
