# AudioRendering

# Resumen
Esta aplicación permite resolver la simulación y auralización de una señal de audio para una escena utilizando trazado de caminos. Esta escena puede cargarse desde un archivo de configuración XML donde se especificar los parámetros a utilizar, el modelo se carga desde un archivo OBJ.  

La simulación calcula la respuesta al impulso de la escena en cuestión mientras que la auralización realiza la convolución de ésta con un archivo de audio o una señal proveniente desde un micrófono.

# Instalación
En este repositorio se incluye un archivo de un proyecto de visual studio 2017. Para instalar la aplicación basta con instalar visual studio 2017 en su PC, abrir el proyecto AudioRendering.sln y compilarlo.

En este repositorio también se incluyen todas las versiones de las bibliotecas utilizadas, de modo que no es necesario descargarlas. Las bibliotecas utilizadas son:

- RtAudio
- glew
- glm
- SDL
- Embree

Una vez compilado el proyecto asegurarse de copiar todas las .dll que se encuentran dentro de la carpeta libs (embree3.dll, glew32.dll, glfw3.dll, rtaduio.dll, SDL2.dll y tbb.dll) en la carpeta que contiene al ejecutable.

# Modo de uso
La aplicación se ejecuta desde línea de comandos y tiene 2 modos de ejecución:

```
> ./AudioRendering [simulate|auralize] [ruta_del_archivo_de_configuración]
```

- El modo 'simulate' realiza solo la simulación para obtener la respuesta al impulso. Al finalizar la simulación se tendrán los valores de intensidad de la respuesta al impulso en el archivo rs.txt.

- El modo 'auralize' realiza la simulación y luego la auralización en tiempo real.

- La ruta del archivo de audio es relativa a la ruta donde se encuentra el ejecutable.

## Archivo de configuración
Pueden encontrarse ejemplos de archivos de configuración validos [aqui](https://github.com/cameelo/AudioRendering/tree/master/AudioRendering/assets/scenes). Dentro de los archivos de configuración se permite definir los siguientes parámetros:

- MODEL: La ruta relativa al archivo .obj del modelo.
- SIZE: La escala del modelo. Una escala de 2.0 aumentara el modelo al doble de su tamaño.
- MAX_REFLEXIONS: El límite de rebotes para cada camino.
- NUM_RAYS: La cantidad de rayos emitidos.
- SOURCE
  - POWER: Nivel sonoro en potencia de la fuente.
  - POS_X: Coordenada x de la posición de la fuente.
  - POS_Y: Coordenada y de la posición de la fuente.
  - POS_Z: Coordenada z de la posición de la fuente.
- LISTENER:
  - SIZE: Radio de la esfera que modela al receptor.
  - POS_X: Coordenada x de la posición del receptor.
  - POS_Y: Coordenada y de la posición del receptor.
  - POS_Z: Coordenada z de la posición del receptor.
- MEASUREMENT: Solo necesario para el modo simulate. El programa toma las propiedades de una medición (por ejemplo, frecuencia de muestreo y bit depth) con la que se desea comparar el resultado de la simulación.
  - FILE: Ruta relativa del archivo de medición.
  - LENGTH: Duración de la medición que se desea considerar. Medido en milisegundos.
- ANALYZE: Opcional. Registra la cantidad de caminos que llegar al receptor entre los tiempos BEGIN y END (medidos en milisegundos).
  - BEGIN:
  - END:
- OUT_SAMPLERATE: Solo necesario para el modo auralize. Es la frecuencia de muestreo con la que se quiere generar la respuesta al impulso y la señal auralizada.
- SOUND_SAMPLE: Opcional para el modo auralize. Especifica la ruta relativa al archivo de audio .wav que se quiere auralizar.

## Controles
En el modo auralize es posible utilizar algunos comandos:

- ESC: Cierra el programa.
- M: Habilita mover al receptor de lugar dentro de la escena. Volver a presionar M inhabilita mover al receptor.
- A, W, S, D: Mueve al receptor.
- E: Coloca el emisor en la posición actual del receptor.
- R: Realiza la simulación nuevamente. En el modo de auralización la simulación se realiza automáticamente solo una vez al inicio del programa.
- T: Habilita la simulación de forma automática. El programa realizara los cálculos de la simulación una vez por frame.
- J, K: Baja y sube el volumen de la señal auralizada respectivamente.


