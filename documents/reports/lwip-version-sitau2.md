# Variante lwIP de SITAU2

La dependencia se selecciona como `lwip211 1.08.s`. Está basada en el paquete Xilinx `lwip211 1.8` de Vitis 2022.2 y conserva lwIP 2.1.1 y los dos fuentes personalizados auditados. La letra s identifica nuestra variante; no representa una versión oficial de Xilinx.

## Generación y controles

`create-workspace.tcl` copia el paquete original a `workspace/software-repository/sw_services/lwip211_v1_08_s`, cambia versión y descripción del MLD, aplica los dos archivos de `config/lwip211` y registra el repositorio antes de generar la plataforma. La instalación Vitis permanece intacta.

`config/bsp/cpu0.mss` exige la versión exacta `1.08.s`. El generador valida el resultado antes de marcar el workspace como listo. Abrir, compilar y empaquetar comprueban:

- Existencia e identidad del MLD personalizado.
- Selección de `1.08.s` en el MSS del BSP y en el exportado.
- SHA-256 de los dos parches en el repositorio de bibliotecas y en el BSP generado.

El `makefile.init` mantiene la comprobación también al compilar las aplicaciones desde la GUI. Estas comprobaciones inspeccionan selección y fuentes, no certifican por sí mismas los bytes de una biblioteca previamente compilada. La validación de compilación se realiza aparte. Operaciones manuales que eviten los scripts y los makefiles quedan fuera de este control.

## Formatos probados

En Vitis/HSI 2022.2: `1.8.st2` y `1.8.s` fueron rechazadas por el parser MLD; `1.08.s` fue registrada como componente distinto de la original `1.8`.

## Ausencia de la variante: prueba nativa

Se abrió el XSA activo en una sesión XSCT/HSI nueva, sin repositorios personalizados, y se creó un diseño software standalone para CPU0. La lista disponible contenía las versiones originales 1.5, 1.6, 1.7 y 1.8. La orden `hsi::add_library lwip211 1.08.s` produjo:

```text
ERROR: [Hsi 55-1594] Core lwip211 of version 1.08.s not found in repositories
MISSING_CUSTOM_RESULT=REJECTED
```

No seleccionó silenciosamente la versión 1.8 en esta prueba. Evidencia local: `D:/sitau2/lwip-version-test-20260923-175807/missing-custom.log`.

## Generación y fallos controlados (23/09/2026)

Workspace aislado: `D:/sitau2/sitau2-software-uci-lwip-work/workspace`. El workspace habitual no se modificó.

- Generación desde cero: salida 0 y marcador `SITAU_OK:setup`; MSS, copia del BSP y biblioteca compilada usan `1.08.s`.
- Se retiró temporalmente la biblioteca del repositorio software. `Check`, `Build`, `Open` y `package.ps1` devolvieron salida 1 con «Falta la biblioteca personalizada lwip211 1.08.s. No se permite usar la original.». La biblioteca se restauró y `Check` volvió a terminar correctamente.
- Se sustituyó únicamente el `xadapter.c` generado del BSP por el original de Xilinx. `Check` devolvió salida 1 por diferencia de hash. Un makefile de prueba que incluye el mismo `CPU0/makefile.init` utilizado por el proyecto devolvió salida 2 y detuvo make antes de ejecutar su objetivo.

Evidencias locales: `D:/sitau2/lwip-setup-validation.log`, `D:/sitau2/lwip-guards-validation.log` y `D:/sitau2/lwip-version-test-20260923-175807/{stock-in-bsp,make-guard}.log`.

## Regeneración del BSP

En una sesión XSCT nueva se ejecutaron `setws`, `repo -set`, `platform active Platform`, `domain active standalone_domain` y `bsp regenerate`. `bsp getlibs` mostró `lwip211 1.08.s` antes y después.

La regeneración restauró `xadapter.c` desde la biblioteca personalizada: SHA-256 `1ea10744bd801f479540b73ec93ee71536d4fefd12b7b7ded5d6cd745de25e6b`. `Check` volvió a pasar, incluidos los dos parches y ambos MSS. XSCT terminó con salida 0 y marcador `SITAU_REGENERATE_OK`.

Evidencia local: `D:/sitau2/lwip-regenerate-validation.log`.

## Compilación y empaquetado final

`setup.ps1 -Action Build -WorkRoot D:/sitau2/sitau2-software-uci-lwip-work` terminó con salida 0 y marcador `SITAU_OK:build`. Tras la regeneración, Vitis reconstruyó los BSP, incluida `libsrc/lwip211_v1_08_s`, y generó ambos ELF. Los controles posteriores y el empaquetado terminaron correctamente.

| Producto | Bytes | Particiones |
| --- | ---: | ---: |
| `cpu0-boot.bin` | 13622280 | 3 |
| `cpu1-network.bin` | 366352 | 1 |

Salida: `workspace/packages/20260923-185941-777`. Se verificaron composición, direcciones y checksums de las cabeceras. Los hashes de entradas y productos están en [lwip-build-manifest.json](lwip-build-manifest.json). Log local: `D:/sitau2/lwip-build-validation.log`.

Avisos conservados: retorno ausente en una ruta de `xemacpsif_physpeed.c:800` y solapamientos de rangos de particiones de Bootgen en la imagen de CPU0, ya observados con las recetas heredadas. Este cambio no modifica esos fuentes ni el diseño de memoria. La compilación correcta no sustituye la validación Ethernet sobre placa.

## Aplicar al workspace habitual

Cerrar Vitis y ejecutar `generar-workspace.bat`. Si existe el workspace, el script pide archivarlo antes de generar uno nuevo desde cero. No se migra ni se parchea automáticamente el entorno anterior. La instalación original de Vitis y los dos archivos personalizados del repositorio conservan sus hashes auditados.
