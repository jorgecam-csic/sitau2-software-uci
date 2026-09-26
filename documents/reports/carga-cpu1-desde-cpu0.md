# Carga de CPU1 desde CPU0

Análisis estático realizado el 23 de septiembre de 2026, complementado con la confirmación del programador transmitida por Jorge: CPU1 nunca se guarda en flash y siempre se envía por red. No se ha modificado el firmware, el empaquetado ni el hardware. No sustituye una prueba en la placa.

## Conclusión

CPU0 implementa un cargador de imágenes recibidas en RAM mediante TCP que puede iniciar CPU1. El BIN heredado de CPU1 encaja con ese mecanismo. La imagen automática nueva de CPU1 no es un sustituto para ese cargador: empieza por un FSBL con dirección de carga cero, lo que hace que el código actual termine el recorrido de particiones sin cargar CPU1.

El nombre `Both_CPUs_system` puede inducir a error: su receta heredada contiene FSBL, bitstream y CPU0, pero no CPU1. La entrega de CPU1 por red está confirmada por el programador. El código analizado explica cómo CPU0 puede recibirla y ejecutarla desde RAM. Quedan por identificar la herramienta cliente, el archivo concreto utilizado y la secuencia de reinicio.

El empaquetado mantenido tiene dos productos separados: `cpu0-boot.bin`, con FSBL + bitstream + CPU0, y `cpu1-network.bin`, con solo CPU1. `scripts/package.ps1` los genera a partir de los ELF compilados y comprueba sus particiones. El BOOT automático de CPU1 generado por Vitis no cumple el formato de carga por red.

## Recorrido implementado

1. `src/cpu0/main.c` inicializa la plataforma y la red y ejecuta `network_loop()` (líneas 104–173). No llama al cargador de CPU1 al arrancar.
2. `src/cpu0/tcpcom.h` define `PUT_MEM` (0x0100), `LOAD_N_RUN` (0x0103), `WRITE_QSPI` (0x0104) y `READ_QSPI` (0x0105). El despachador de `tcpcom.c` implementa esos comandos.
3. `put_mem()` recibe dirección y longitud y transfiere los datos a RAM. La dirección especial `0xFFFFFFFF` selecciona `buffer_imagen`, de 64 MiB (`tcpcom.c`, líneas 52–57 y 507–523). La recepción permite continuar transferencias en varias entregas TCP.
4. `load_n_run_image()` recibe una dirección, admite el mismo valor especial y llama a `ssbl_ddr(image_addr)` (`tcpcom.c`, líneas 320–346). Es la única llamada a `ssbl_ddr()` encontrada en los fuentes de CPU0.
5. `ssbl_ddr()` configura el origen en RAM mediante `FlashReadBaseAddress`, `LinearBootDeviceFlag=1` y `MoveImage=DDRAccess`; después llama a `LoadBootImage(0, AppStartAddr)` (`ssbl.c`, líneas 293–331). El nombre de la variable no implica que esté leyendo la flash en este recorrido.
6. `LoadBootImage()` recorre desde la partición cero, carga las particiones y recoge sus direcciones de ejecución (`image_mover_mod.c`, líneas 210–349). El código permite imágenes con una sola partición: la restricción heredada que las rechazaba está desactivada en `GetPartitionHeaderInfo()`.
7. Para la primera aplicación, `ssbl_ddr()` escribe su dirección en `0xFFFFFFF0`, ejecuta `dmb()` y `sev()` para señalar a CPU1. `MOD_BOOT_S` está definido en `ssbl.h`. Si hay una segunda aplicación, llama a `FsblHandoff(AppStartAddr[1])` para CPU0 (`ssbl.c`, líneas 338–359). La asignación depende del orden de las aplicaciones.

El bloque que fuerza un reset de CPU1 está dentro de `if (0)` (`ssbl.c`, líneas 302–327). Por tanto, el análisis no demuestra que se pueda recargar CPU1 mientras está ejecutándose: hay que confirmar el estado de CPU1 y la secuencia de reinicio utilizada.

## Comprobación de los BIN existentes

Se leyeron las cabeceras en modo solo lectura, usando los offsets y atributos de `src/cpu0/image_mover_mod.h`. Se comprobó la suma de las cabeceras de partición y el terminador. Esta comprobación describe el contenedor; no valida funcionalmente su contenido.

| Archivo | Tamaño total | Particiones: carga / ejecución |
| --- | ---: | --- |
| `CPU1_system/_ide/bootimage/BOOT.bin` | 349.968 bytes | Una PS: `0x18000000 / 0x18000000` |
| `Both_CPUs_system/_ide/bootimage/BOOT.bin` | 13.622.280 bytes | FSBL: `0 / 0`; PL; CPU0: `0x00100000 / 0x00100000` |
| `../sitau2-software-uci-work/workspace/CPU1_system/Debug/sd_card/BOOT.BIN` | 13.786.128 bytes | FSBL: `0 / 0`; PL; CPU1: `0x18000000 / 0x18000000` |

Las recetas BIF heredadas concuerdan con esas composiciones. El linker de CPU1 sitúa su memoria principal en `0x18000000` y utiliza `_vector_table` como entrada (`src/cpu1/lscript.ld`, líneas 26–34).

### Por qué la imagen automática no sirve para LOAD_N_RUN

El cargador empieza por la partición cero; no salta automáticamente el FSBL. En `image_mover_mod.c`, líneas 322–326, una partición PS sin firma ni cifrado cuya dirección de carga sea cero y menor que `DDR_START_ADDR` provoca un `break`. En los BSP generados, `XPAR_PS7_DDR_0_S_AXI_BASEADDR` vale `0x00100000`, y `fsbl.h` lo utiliza como `DDR_START_ADDR`.

La primera partición de la imagen automática inspeccionada cumple esas condiciones. El resultado deducido del código es cero aplicaciones cargadas, sin alcanzar CPU1 ni el bitstream. Esto se refiere específicamente a su entrega al cargador `LOAD_N_RUN`, no a todos los posibles métodos de arranque o programación.

## RAM, flash y JTAG son recorridos distintos

- `LOAD_N_RUN` carga desde RAM y no programa por sí mismo la QSPI.
- `READ_QSPI` y `WRITE_QSPI` reciben del cliente la dirección de flash y la longitud; la escritura borra y escribe el rango solicitado (`tcpcom.c`, líneas 267–317). Su existencia no demuestra qué imagen se persiste ni en qué offset.
- El script heredado `Both_CPUs_system/_ide/scripts/debugger_cpu0-cpu1.tcl` configura el hardware y descarga por JTAG los ELF de CPU0 y CPU1 en sus respectivos procesadores. Esa vía de depuración no utiliza el BIN de CPU1 recibido por TCP.

## Pendiente de confirmar con el programador

1. ¿El equipo arranca con FSBL + bitstream + CPU0, y el programa del PC envía después el BIN de CPU1 mediante PUT_MEM y LOAD_N_RUN? ¿Qué programa y qué archivo concreto utiliza?
2. Confirmado: CPU1 nunca se guarda en QSPI y siempre se entrega por red. Falta precisar cuándo lo envía el cliente (inicio de sesión, conexión u otra operación).
3. ¿Qué secuencia se usa para reiniciar o recargar CPU1, dado que el reset explícito está desactivado en este cargador?
4. ¿Qué variantes de BIN/bitstream del directorio heredado siguen en uso?

Las recetas mantenidas están en `config/bootimage`. Las siete recetas/salidas de nivel superior heredadas se han archivado fuera del repositorio, con trazabilidad en `bootimage-archived.json`; las rutas de la tabla anterior describen su ubicación original. El contenido de `CPU1_system/_ide/bootimage/BIN` sigue conservado para resolver su uso. La secuencia de recarga en caliente no bloquea este empaquetado, pero sigue sin validarse sobre hardware.
