# lwip211

Personalizaciones auditadas de lwIP 2.1.1, basadas en la biblioteca Xilinx 1.8 y registradas en Vitis como **lwip211 1.08.s** (variante SITAU2, revisión 1).

## Archivos

| Archivo | Función |
| --- | --- |
| [manifest.json](manifest.json) | Hashes de los archivos originales Vitis y las dos personalizaciones auditadas. |
| [xadapter.c](xadapter.c) | Adaptador Ethernet personalizado; conserva los break auditados en selección de controlador. |
| [xemacpsif_physpeed.c](xemacpsif_physpeed.c) | Variante heredada del control de velocidad/PHY Ethernet, incluido Micrel KSZ9031. |

El generador copia la biblioteca original de Vitis a `workspace/software-repository/sw_services/lwip211_v1_08_s`, cambia versión y descripción en el MLD, aplica estos dos archivos y registra el repositorio antes de crear la plataforma. El BSP solicita exactamente `1.08.s`; no se modifica la instalación de Vitis ni se parchea únicamente una salida generada.

Al regenerar el BSP, la entrada es esa biblioteca personalizada. Las comprobaciones previas a compilar y empaquetar exigen su presencia, identidad, selección en los MSS y los hashes de ambos archivos tanto en el repositorio software como en el BSP. Si algo no coincide, el flujo se detiene.

La letra `s` distingue SITAU2; no es una versión oficial de Xilinx. Los cambios futuros deben registrarse como otra revisión documentada y validarse. El nombre enlazado sigue siendo `liblwip4.a`; los includes de la aplicación no cambian. No editar las copias generadas. Una actualización exige auditar diferencias, actualizar hashes y validar Ethernet sobre hardware.

[Volver](../README.md)
