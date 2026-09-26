# Plan de entorno reproducible y dependencias

Actualizado: 22 de septiembre de 2026. Sustituye la propuesta de mantener el workspace
de Vitis dentro del repositorio. Ejecución autorizada; validación hardware pendiente.

## Modelo acordado

El repositorio conserva fuentes, configuración y paquetes hardware identificados por hash.
PowerShell prepara un entorno temporal y llama a XSCT/Tcl de Vitis Classic 2022.2 para crear
Platform, BSP, FSBL, aplicaciones y proyectos de sistema en una carpeta hermana.
Los fuentes de las aplicaciones se enlazan al repositorio para poder editarlos desde la GUI.
Los metadatos de Eclipse y productos generados pertenecen al workspace externo.

`artifacts/dependencies-lock.json` selecciona un paquete UCI inmutable y su hash. No se
consumen resultados variables de repositorios hardware vecinos. Volver a otro paquete
consiste en cambiar el lock y regenerar el workspace, conservando los cambios del software.
Los tres XSA heredados distintos se preservan, sin inventar versiones funcionales ni commits.

## Orden de ejecución

1. Guardar fuera del repositorio la referencia de compilación, los logs, binarios y hashes.
2. Comparar workspace actual, backup intacto y generación limpia para identificar
   personalizaciones de BSP/FSBL. Extraer configuración y, si existen, cambios propios.
3. Crear paquetes y lock; conservar los bytes originales de los fuentes y artefactos.
4. Implementar PowerShell y una receta Tcl mínima. Verificar herramientas, hashes,
   rutas, estado del workspace y errores. Archivar el entorno anterior al regenerar.
5. Generar, compilar y empaquetar en el directorio externo. Comprobar referencias,
   fuentes enlazados, secciones de ELF y contenido de las imágenes frente a la referencia.
6. Repetir en una copia que contenga únicamente las entradas mantenidas, sin `.metadata`
   ni resultados heredados. Probar fallos de hash y repetición sin cambios.
7. Retirar del seguimiento los proyectos regenerables solo después de verificarlo;
   conservarlos localmente durante la transición si la GUI heredada sigue abierta.
8. Escribir un README breve en español con el flujo realmente validado y sus límites.

## Material protegido y pendientes

- `D:/sitau2/sitau2_sw_uci` permanece intacto como referencia independiente.
- Se conservan `_ide/bootimage` y todos sus contenidos en su ubicación actual.
  Identificar recetas operativas, firmware MK32, históricos y consumidores externos
  antes de cualquier migración o retirada. Inventariar no equivale a validar su uso.
- Se conservan las configuraciones de depuración para revisarlas; no afirmar que son
  portables mientras mantengan rutas antiguas o números de serie de cables.
- La codificación tiene su [plan independiente](normalizacion-codificaciones.md).
  Preservar bytes y declarar Cp1252 en el editor generado, igual que el workspace heredado.
- Mantener inicialmente Debug, que es la configuración probada; Release necesita
  una validación propia. No trasladar dominios obsoletos de Release como si estuvieran probados.

## Criterios de aceptación

- Setup y Build funcionan sin los proyectos heredados ni artefactos ignorados preexistentes.
- La plataforma usa el XSA seleccionado y falla de forma explícita si falta o cambia.
- Los ELF y las dos imágenes automáticas se generan fuera de Git con Vitis 2022.2.
- Un cambio de configuración o selección exige regenerar el workspace; no se reutiliza
  silenciosamente una plataforma incompatible. No se borra un workspace ajeno.
- Los fuentes enlazados pertenecen a este checkout; distintas copias usan distintos workspaces.
- Los fuentes conservan sus bytes originales. La prueba en hardware y las recetas
  personalizadas requieren una verificación independiente.

La [referencia de compilación](../reports/referencia-compilacion.md) documenta el punto de partida.
La [validación del refactor](../reports/validacion-refactor.md) recoge los resultados de ejecución.
