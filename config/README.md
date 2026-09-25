# config

Configuración mantenida con la que se reconstruyen aplicaciones, BSP y paquetes. Aquí se esperan entradas declarativas y personalizaciones auditadas; las copias generadas de BSP, los proyectos Vitis y los productos de compilación pertenecen al workspace externo.

## Archivos

| Archivo | Función |
| --- | --- |
| [applications.tcl](applications.tcl) | Define aplicaciones CPU0/CPU1, dominios, proyectos de sistema, fuentes, includes y bibliotecas. |

## Subcarpetas

- [bootimage](bootimage/README.md): Plantillas portables de los dos productos de entrega. Las rutas se resuelven desde el workspace al empaquetar.
- [bsp](bsp/README.md): Parámetros MSS de los dominios BSP de CPU0, CPU1 y FSBL.
- [lwip211](lwip211/README.md): Personalizaciones auditadas de lwIP 2.1.1, basadas en Xilinx 1.8 y registradas como lwip211 1.08.s.

Los cambios en las recetas y configuración de esta carpeta invalidan la huella del workspace. Guardarlos en Git y generar desde cero antes de compilar. Los README y auxiliares documentados en [scripts](../scripts/README.md#controles-y-mantenimiento) quedan fuera de la huella. Los cambios de propiedades hechos solo en el IDE no se incorporan automáticamente a estas recetas.

[Volver](../README.md)
