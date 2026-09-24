# MK32 0.0.0

Paquete inicial formado por los tres archivos de `CPU1_system/_ide/bootimage/BIN`, seleccionados por el usuario el 24/09/2026. Se copiaron sin modificar bytes y se verificaron sus SHA-256 contra el inventario previo. Posteriormente se eliminó BIN completo por autorización del usuario; este paquete conserva el trío seleccionado.

`0.0.0` es la versión asignada a esta entrega, no una versión deducida del diseño Vivado. Se desconoce el commit productor y la versión exacta de Vivado. No se ha realizado una validación nueva sobre hardware.

| Archivo | Objeto |
| --- | --- |
| [MK32CH_basics_wrapper.bin](MK32CH_basics_wrapper.bin) | Imagen binaria del hardware MK32; no es firmware de CPU0/CPU1. |
| [MK32CH_basics_wrapper.bit](MK32CH_basics_wrapper.bit) | Bitstream FPGA del mismo conjunto. |
| [MK32CH_basics_wrapper.ltx](MK32CH_basics_wrapper.ltx) | Sondas de depuración asociadas al bitstream. |
| [manifest.json](manifest.json) | Versión, procedencia, tamaños y SHA-256 de los tres archivos. |

Este paquete no modifica el XSA activo de la UCI ni requiere regenerar su workspace.

[Volver](../README.md)
