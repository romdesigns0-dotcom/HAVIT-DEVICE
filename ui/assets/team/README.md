# Recursos de Teams

Origen: Figma `3bmX64I7Rt85DV3boqOlsc`, formulario `464:201`.
Los SVG originales se conservan sin modificar y se convierten con
`tools/generate_team_assets.py` a su tamaño nativo.

El generador compara dimensiones y SHA-256 de píxeles con los recursos ya
presentes en Tarea Rápida y Today. Reutiliza los idénticos: micrófono 24×24,
edición 24×24, reloj 18×18, separador 6×24, X 29×29, fondo de X 33×33 y línea
250×1. Solo genera los seis recursos distintos (calendario, avatares, botón
de añadir miembro y dos líneas). El formulario ya no dibuja esos iconos con
líneas y rectángulos aproximados.

También se conservan los recursos originales de los frames `464:483`,
`464:493`, `464:1211` y `464:617` (prefijos `view1` a `view4`). Se usan
a tamaño nativo para carga, calendario, desglose y envío. Al rasterizar,
el generador aplica las transformaciones de las instancias de Figma:
giro de las flechas de desglose e inversión vertical del símbolo de envío.
Los SVG fuente permanecen intactos.
