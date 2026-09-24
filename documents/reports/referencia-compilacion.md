# Referencia de compilación del workspace heredado

Verificada el 21–22 de septiembre de 2026 con Vitis Classic 2022.2 en Windows.
Commit previo al refactor: `85901ef`. La GUI usó el workspace heredado y sus metadatos.
No es todavía una prueba de clonación limpia ni de funcionamiento en hardware.

| Componente | Resultado |
|---|---|
| Platform | BSP de ambas CPU, FSBL y exportación regenerados después de Clean. |
| CPU0 Debug | Clean + Build: 25 unidades, 0 errores, 79 advertencias. |
| CPU1 Debug | Clean + Build: 53 unidades, 0 errores, 402 advertencias. |
| Both_CPUs_system Debug | BOOT.BIN con FSBL + bitstream + CPU0. |
| CPU1_system Debug | BOOT.BIN con FSBL + bitstream + CPU1. |

La especificación leída fue `Platform/hw/design_1_wrapper.xsa`, SHA-256
`3ed161d88aff0e0c9244fbacf1fc15ac816d9b40662f01281b2efc3adf6b8286`.
Se exportó con Vivado 2022.2.2 para `xc7z045ffg676-2`.
El bitstream utilizado coincide con el incluido en ese XSA.

Los hashes de las salidas conservadas están en [baseline-manifest.json](baseline-manifest.json).
La copia local de fuentes, configuración, logs y binarios de referencia está fuera de Git,
en `D:/sitau2/sitau2-software-uci-work/reference`. No se ha modificado
`D:/sitau2/sitau2_sw_uci`, el backup procedente del ordenador del programador.

## Hallazgos

- Un PATH largo bloqueaba la lectura de `which sdscc` durante el arranque. El lanzador
  prepara un PATH mínimo solo para Vitis, sin cambiar las variables permanentes de Windows.
- El Clean desde Assistant también limpió Platform. Fue necesario regenerarla antes
  de reconstruir CPU1; su exportación `Platform.xpfm` había desaparecido.
- Vitis actualizó las referencias de plataforma en CPU0.prj y CPU1.prj a la nueva ruta,
  pero quedaron campos históricos `C:/PV/...`. Las órdenes compiladas usaron la ruta nueva.
- Hay advertencias relevantes (declaraciones implícitas, conversiones de tipos y retornos);
  se conservan para una revisión posterior, sin cambios funcionales durante este refactor.
- Las recetas personalizadas de `_ide/bootimage` no se han probado. En particular,
  la receta personalizada CPU1 contiene solo CPU1, a diferencia del sistema automático.
- La comparación byte a byte de los archivos versionados actuales con sus homólogos
  existentes en el backup solo encontró cambios en CPU0.prj, CPU1.prj y platform.tcl.

## Pendiente de intervención en el equipo

Validar arranque, adquisición, comunicaciones y depuración sobre hardware; identificar
qué recetas de `_ide/bootimage` se utilizan para actualizar el equipo. No se consideran
validadas por el mero hecho de generar BOOT.BIN.
