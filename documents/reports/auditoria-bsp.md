# Auditoría de BSP y FSBL

Se comparó la plataforma heredada, idéntica al backup en estos fuentes, con una
plataforma generada desde cero por Vitis 2022.2 usando el XSA seleccionado y las
mismas bibliotecas y parámetros stdin/stdout. Detalle en [bsp-audit.json](bsp-audit.json).

- 2.595 archivos C/H/ensamblador/linker/MSS coinciden byte a byte.
- Siete archivos difieren solo en orden: bloques de MSS equivalentes o definiciones
  de `xparameters.h` con los mismos nombres y valores.
- `system_1.mss` es una copia auxiliar heredada; la configuración activa está en `system.mss`.
- Dos fuentes de lwIP contienen diferencias funcionales que deben conservarse:
  `xadapter.c` y `xemacpsif_physpeed.c`. Se preservan en `config/lwip211/`, con hashes,
  licencia original y una biblioteca derivada en el workspace. No se modifica la instalación.
- No se encontraron cambios propios en los fuentes del FSBL comparados.

La comparación abarca los fuentes activos de Platform, excluyendo copias exportadas,
`hw`, `resources` y temporales; el hardware de entrada se identifica por hash separado.
Los parámetros y bibliotecas se conservan en `config/bsp/*.mss`. La receta aplica los
parámetros OS/librerías; los controladores del hardware son los que selecciona la herramienta
2022.2 para este XSA, contrastados con la referencia. Cambiar hardware o herramienta
requiere revisar de nuevo la selección de controladores y esta auditoría.

La versión de PHY heredada difiere ampliamente de la estándar (incluye soporte Micrel
KSZ9031). Se conserva completa: esta auditoría no pretende atribuir autoría ni determinar
la intención de cada cambio. La comprobación en hardware queda pendiente.
