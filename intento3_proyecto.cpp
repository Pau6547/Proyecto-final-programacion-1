#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAX_DATOS 1000  // Numero maximo de datos que se pueden cargar

// Enumerador para representar el tipo de peligrosidad
typedef enum {
    NOT_HAZARDOUS,        // 0
    POTENTIALLY_HAZARDOUS // 1
} HazardType;

// Funcion para cargar el archivo CSV
int cargarDatos(const char *archivoCSV, float punto[MAX_DATOS][2], HazardType hazard[MAX_DATOS]) { 
    FILE *archivo = fopen(archivoCSV, "r");
    if (!archivo) {
        perror("Error al abrir el archivo");
        return 0; 
    }

    char linea[256];
    int index = 0;

    // Saltar la primera linea (cabecera del CSV)
    fgets(linea, sizeof(linea), archivo);

    while (fgets(linea, sizeof(linea), archivo)) {
        // Eliminar saltos de linea al final de la cadena
        linea[strcspn(linea, "\r\n")] = 0;

        // Parsear la linea
        char *token = strtok(linea, ",");  // Obtener la parte de Asteroid Velocity (X)
        if (token) {
            punto[index][0] = strtod(token, NULL);  // Guardar X
        }

        token = strtok(NULL, ",");  // Obtener la parte de Maximum Palermo Scale (Y)
        if (token) {
            punto[index][1] = strtod(token, NULL);  // Guardar Y
        }

        token = strtok(NULL, ",");  // Obtener la parte de Hazard
        if (token) {
            // Convertir cadena a enum
            if (strcmp(token, "Potentially Hazardous") == 0) {
                hazard[index] = POTENTIALLY_HAZARDOUS;
            } else {
                hazard[index] = NOT_HAZARDOUS;
            }
        }

        index++;
        if (index >= MAX_DATOS) break;  // Limitar al maximo de datos
    }

    fclose(archivo);
    return index;  // Retornar el numero de puntos cargados
}

// Funcion para generar el script de Gnuplot
void generarScriptGnuplot(float punto[MAX_DATOS][2], HazardType hazard[MAX_DATOS], int numDatos, const char *scriptGnuplot, float nuevoPunto[2], HazardType nuevoHazard) {
    FILE *script = fopen(scriptGnuplot, "w");
    if (script) {
        fprintf(script, "set title 'Grafica de Asteroides y Escala Palermo'\n");
        fprintf(script, "set xlabel 'Asteroid Velocity (km/s)'\n");
        fprintf(script, "set ylabel 'Maximum Palermo Scale'\n");
        fprintf(script, "set grid\n");

        // Crear una grafica con tres tipos de puntos (segun la peligrosidad y el nuevo punto)
        fprintf(script, "plot '-' using 1:2 with points pt 7 lc rgb 'green' title 'Potentially Hazardous',\\\n");
        fprintf(script, "     '-' using 1:2 with points pt 7 lc rgb 'blue' title 'Not Hazardous',\\\n");
        fprintf(script, "     '-' using 1:2 with points pt 2 lc rgb 'red' title 'Nuevo Punto'\n");

        // Escribir los datos "Potentially Hazardous"
        for (int i = 0; i < numDatos; i++) {
            if (hazard[i] == POTENTIALLY_HAZARDOUS) {
                fprintf(script, "%f %f\n", punto[i][0], punto[i][1]);
            }
        }
        fprintf(script, "e\n");  // Fin de los datos para 'Potentially Hazardous'

        // Escribir los datos "Not Hazardous"
        for (int i = 0; i < numDatos; i++) {
            if (hazard[i] == NOT_HAZARDOUS) {
                fprintf(script, "%f %f\n", punto[i][0], punto[i][1]);
            }
        }
        fprintf(script, "e\n");  // Fin de los datos para 'Not Hazardous'

        // Escribir el nuevo punto
        fprintf(script, "%f %f\n", nuevoPunto[0], nuevoPunto[1]);
        fprintf(script, "e\n");  // Fin de los datos para 'Nuevo Punto'

        fclose(script);
        printf("Script de Gnuplot generado: %s\n", scriptGnuplot);
    } else {
        perror("Error al crear el script de Gnuplot");
    }
}

int main() {
    const char *archivoCSV = "C:\\Users\\spint\\OneDrive\\Documentos\\proyectofinal_programacion\\DB_Gr7.csv";  // Ruta del archivo 
    const char *scriptGnuplot = "grafica.gnuplot";  // Archivo del Gnuplot

    float punto[MAX_DATOS][2];  // Arreglo bidimensional para almacenar las coordenadas (x, y)
    HazardType hazard[MAX_DATOS];  // Arreglo para almacenar los mensajes de peligro como enum
    int numDatos = cargarDatos(archivoCSV, punto, hazard);
    if (numDatos == 0) {
        fprintf(stderr, "No se cargaron datos validos.\n");
        return 1;
    }

    // Crear un array regular en lugar de un array temporal
    float nuevoPunto[2] = {0.0, 0.0}; // Array regular
    // Suponiendo que el nuevo punto es "No Peligroso"
    HazardType nuevoHazard = NOT_HAZARDOUS; // No es necesario determinar la peligrosidad aquí

    // Generar el script de Gnuplot con los datos cargados
    generarScriptGnuplot(punto, hazard, numDatos, scriptGnuplot, nuevoPunto, nuevoHazard);

    // Ejecutar Gnuplot para mostrar la primera grafica
    char comando[256];
    snprintf(comando, sizeof(comando), "gnuplot -p %s", scriptGnuplot);
    if (system(comando) != 0) {
        fprintf(stderr, "Error al ejecutar Gnuplot.\n");
    }

    // Bucle para que el usuario ingrese nuevos puntos despues de ver la grafica
    char continuar = 'y';
    while (continuar == 'y' || continuar == 'Y') {
        // Solicitar al usuario el nuevo punto
        printf("Ingresa la velocidad del asteroide (km/s): ");
        scanf("%f", &nuevoPunto[0]);
        printf("Ingresa la escala maxima Palermo: ");
        scanf("%f", &nuevoPunto[1]);

        // Generar el nuevo script de Gnuplot con el nuevo punto (sin determinar su peligrosidad aquí)
        generarScriptGnuplot(punto, hazard, numDatos, scriptGnuplot, nuevoPunto, nuevoHazard);

        // Ejecutar Gnuplot para mostrar la grafica actualizada
        snprintf(comando, sizeof(comando), "gnuplot -p %s", scriptGnuplot);
        if (system(comando) != 0) {
            fprintf(stderr, "Error al ejecutar Gnuplot.\n");
        }

        // Preguntar si quiere agregar mas puntos
        printf("Deseas agregar otro punto? (y/n): ");
        scanf(" %c", &continuar);  
    }
    
    return 0;
}

