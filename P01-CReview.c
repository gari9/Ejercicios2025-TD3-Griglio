
//  Programa para almacenar HUMEDAD/VELOCIDAD VIENTO/TEMPERATURA en un arreglo de estructuras
// a Se  calcula el promedio de humedad y la temperatura máxima   

#include <stdio.h>

// Definimos estructura con las variables.
struct CReview 
{
    int humedad;
    float velocidadViento;
    float temperatura;
};

int main()
{
    // Arreglo de valores aleatorios de tipo CReview
    // Humedad en %, Velocidad del viento en km/h, Temperatura en °C

    struct CReview climaArray[5] = 
    {
        { 50, 12.5, 24.5 },
        { 0, 15.0, 25.0  },
        { 20, 10.2, 26.5 },
        { 20, 20.3, 28.5 },
        { 30, 13.4, 30.2 }
    };
    
    // Imprimir los elementos de UN arreglo de tipo CReview 
    printf("Humedad: %i [%%], Velocidad del viento: %.1f [km/h], Temperatura: %.1f [C]\n", climaArray[1].humedad, climaArray[1].velocidadViento, climaArray[1].temperatura); 
 
    // Función que recorre el arreglo de tipo CReview calculando promedio de humedad y maximo de temperatura
    // Primero se inicializan las variables promedioHumedad y maxTemperatura en 0
    float promedioHumedad = 0;
    float maxTemperatura = 0;

    // Se recorre el arreglo de tipo CReview

    for (int i = 0; i < 5; i++) {
        promedioHumedad += climaArray[i].humedad;                   // Sumar humedad
        if (climaArray[i].temperatura > maxTemperatura) {           // Comparar temperatura sobreescribiendo si es mayor a la variable maxTemperatura
            maxTemperatura = climaArray[i].temperatura;             
        }
    }

    printf("Promedio humedad: %.1f\n", promedioHumedad / 5);       // Mostrar promedio humedad
    printf("Temperatura maxima: %.1f\n", maxTemperatura);          // Mostrar temperatura maxima

 
}
