# Empaquetado de arranque y carga por red

23 de septiembre de 2026. CPU1 siempre se entrega por red y nunca se almacena en flash, según confirmación del programador transmitida por Jorge.

## Implementación

- `config/bootimage/cpu0-boot.bif.in`: FSBL + bitstream + CPU0.
- `config/bootimage/cpu1-network.bif.in`: solo CPU1, sin FSBL ni bitstream.
- `scripts/package.ps1`: sustituye las rutas de las plantillas con las entradas del workspace externo, ejecuta Bootgen 2022.2 y valida cabeceras, límites de partición, composición y direcciones esperadas.
- `scripts/setup.ps1 -Action Build`: compila los ELF y llama al empaquetado explícito. La receta XSCT deja de ejecutar los builds de sistema que generaban un BOOT CPU1 inadecuado para este flujo.

Cada ejecución produce `workspace/packages/<fecha>/` con los dos BIN, sus BIF resueltos y un manifiesto de hashes de entradas y salidas. `packages/latest.txt` se actualiza solo si ambos productos se generan y validan. Las ejecuciones previas se conservan. La orden independiente de empaquetado utiliza los ELF existentes, sin recompilar ni certificar que correspondan a la última edición de fuentes.

La GUI todavía puede generar sus imágenes automáticas si se compilan los proyectos de sistema. No son los productos de distribución de este flujo: para entregar CPU1 se utiliza exclusivamente `cpu1-network.bin`. Si la herramienta cliente requiere el nombre BOOT.bin, se copia con ese nombre al preparar la entrega.

## Limpieza

Las siete recetas, imágenes y logs de nivel superior de los dos bootimage se han trasladado a `D:/sitau2/sitau2-software-uci-work/reference/bootimage-legacy-20260923`. Sus rutas originales, destinos y SHA-256 están en `bootimage-archived.json`. Los BIF activos ya no dependen de rutas C:/PV del ordenador original.

Los 105 archivos de `CPU1_system/_ide/bootimage/BIN` se conservan, al igual que los scripts y lanzamientos de depuración heredados fuera de bootimage. Se verificaron los hashes de los 112 archivos inventariados después del traslado: siete en el archivo externo y 105 en su ubicación original.

## Validación

- Flujo completo `setup.ps1 -Action Build -WorkRoot D:/sitau2/sitau2-software-uci-packaging-work` finalizado con código 0: workspace creado desde cero, plataforma y aplicaciones compiladas, marcadores `SITAU_OK:setup` y `SITAU_OK:build`, y ambos paquetes generados y validados. Salidas en `workspace/packages/20260923-121042-671`; hashes y entradas en [packaging-build-manifest.json](packaging-build-manifest.json). Log completo: `D:/sitau2/packaging-validation.log`.

- Ejecución de `package.ps1` con Windows PowerShell 5 y Bootgen 2022.2 sobre los ELF previamente compilados: ambos productos generados y cabeceras validadas.
- CPU1 generado con la receta nueva y con el BIF heredado apuntando al mismo ELF: igualdad byte a byte; SHA-256 `77dd1036395cc0c3cf470622ba869e4e11be9f120d82c323d4202a7b17b2a266`.
- CPU0 generado con la receta nueva y con el BIF heredado apuntando a las mismas entradas: igualdad byte a byte; SHA-256 `fe62a1546574fb9a4b3db5e543ae9f9135c51f3a5398601c999b02b6dc901e07`. Ambas recetas emiten los mismos avisos de solapamiento.
- Bootgen emite avisos de solapamiento para la imagen CPU0 (FSBL/bitstream y bitstream/CPU0). La generación termina correctamente; esto no se presenta como validación de arranque sobre placa.
- La recarga de CPU1 mientras está ejecutándose, el arranque y la adquisición requieren comprobación sobre hardware. No se ha modificado el firmware.

## Pendiente

Identificar qué variantes MK32, imágenes históricas CPU1, configuraciones y auxiliares siguen en uso antes de archivar o retirar definitivamente el contenido de BIN. La clasificación ya está hecha. También sigue pendiente la revisión de los lanzamientos JTAG heredados y de la secuencia de recarga en caliente; ninguna de ellas impide generar los paquetes.
