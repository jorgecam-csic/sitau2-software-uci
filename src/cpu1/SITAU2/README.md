# SITAU2

Controladores y servicios de hardware organizados por bloque funcional.

## Subcarpetas

- [AFE](AFE/README.md): Configuración y calibración del frontal analógico AFE.
- [AXIL2STREAM](AXIL2STREAM/README.md): Interfaz FIFO entre AXI-Lite y streams.
- [BCC_BUSSAR](BCC_BUSSAR/README.md): Comunicación y programación de módulos remotos mediante BCC/BUSSAR.
- [BEAMFORMER](BEAMFORMER/README.md): Programación del bloque de conformación de haces y leyes focales.
- [CH_EXTRACT](CH_EXTRACT/README.md): Selección/extracción de canales del flujo de datos.
- [COMMON](COMMON/README.md): Utilidades numéricas, registro de errores y temporización.
- [DATAMOVER](DATAMOVER/README.md): Movimiento de datos entre memoria y streams, local y remoto.
- [DYNAMIC_PHASE](DYNAMIC_PHASE/README.md): Ajuste de fase dinámica.
- [FIFO_CTRL](FIFO_CTRL/README.md): Control y diagnóstico de FIFO.
- [FILTRO_FIR](FILTRO_FIR/README.md): Configuración de filtrado digital FIR/Hilbert.
- [GTX](GTX/README.md): Control de enlaces GTX y de sus interfaces de datos.
- [HW_UCI](HW_UCI/README.md): Servicios de CPU1: interrupciones, memoria compartida, caché, temporizadores, encoders y triggers.
- [PROM_EMI](PROM_EMI/README.md): Programación de secuencias de emisión.
- [PULSER](PULSER/README.md): Control del pulsador y retardos de emisión.
- [SWITCH](SWITCH/README.md): Encaminamiento de streams entre bloques hardware.
- [TGC](TGC/README.md): Configuración de ganancia variable en el tiempo.
- [UCI](UCI/README.md): Control del equipo, registros, adquisición y transferencias DMA.

Editar los fuentes aquí, no las copias generadas del workspace. Las cabeceras describen interfaces y datos; su presencia no garantiza que todas las rutinas se ejecuten en el flujo habitual.

## Contenido esperado y mantenimiento

Conservar aquí los fuentes, cabeceras y recursos mantenidos del módulo descrito arriba. Añadir archivos junto a los del mismo cometido y actualizar este inventario. Los objetos, bibliotecas compiladas y ejecutables se generan en el workspace, no en esta carpeta.

Las ediciones desde los recursos enlazados de Vitis afectan a estos archivos. Mantener su codificación heredada; al añadir, eliminar o mover archivos, generar de nuevo el workspace para reconstruir los enlaces. Los cambios en registros, direcciones o protocolos deben contrastarse con el hardware seleccionado.

[Volver](../README.md)
