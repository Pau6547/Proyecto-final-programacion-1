#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAX_DATOS 1000

typedef enum {
    NOT_HAZARDOUS,
    POTENTIALLY_HAZARDOUS
} HazardType;

typedef struct {
    float x, y;
    HazardType hazard;
} Point;

typedef struct {
    int index;
    float distance;
} Distance;

int cargarDatos(const char *archivoCSV, Point puntos[MAX_DATOS]) {
    FILE *archivo = fopen(archivoCSV, "r");
    if (!archivo) {
        perror("Error al abrir el archivo");
        return 0; 
    }

    char linea[256];
    int index = 0;

    fgets(linea, sizeof(linea), archivo);

    while (fgets(linea, sizeof(linea), archivo)) {
        linea[strcspn(linea, "\r\n")] = 0;

        char *token = strtok(linea, ",");
        if (token) {
            puntos[index].x = strtod(token, NULL);
        }

        token = strtok(NULL, ",");
        if (token) {
            puntos[index].y = strtod(token, NULL);
        }

        token = strtok(NULL, ",");
        if (token) {
            if (strcmp(token, "Potentially Hazardous") == 0) {
                puntos[index].hazard = POTENTIALLY_HAZARDOUS;
            } else {
                puntos[index].hazard = NOT_HAZARDOUS;
            }
        }

        index++;
        if (index >= MAX_DATOS) break;
    }

    fclose(archivo);
    return index; 
}

float calcularDistancia(Point p1, Point p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

void encontrarVecinosMasCercanos(Point puntos[MAX_DATOS], int numDatos, Point nuevoPunto, Distance distancias[MAX_DATOS]) {
    for (int i = 0; i < numDatos; i++) {
        distancias[i].index = i;
        distancias[i].distance = calcularDistancia(puntos[i], nuevoPunto);
    }

    for (int i = 0; i < numDatos - 1; i++) {
        for (int j = 0; j < numDatos - i - 1; j++) {
            if (distancias[j].distance > distancias[j + 1].distance) {
                Distance temp = distancias[j];
                distancias[j] = distancias[j + 1];
                distancias[j + 1] = temp;
            }
        }
    }
}

HazardType clasificarNuevoPunto(Point puntos[MAX_DATOS], int numDatos, Point nuevoPunto) {
    Distance distancias[MAX_DATOS];
    encontrarVecinosMasCercanos(puntos, numDatos, nuevoPunto, distancias);

    int cuentaHazard = 0;
    for (int i = 0; i < 3; i++) {
        if (puntos[distancias[i].index].hazard == POTENTIALLY_HAZARDOUS) {
            cuentaHazard++;
        }
    }

    if (cuentaHazard > 1) {
        return POTENTIALLY_HAZARDOUS;
    } else {
        return NOT_HAZARDOUS;
    }
}

void generarScriptGnuplot(Point puntos[MAX_DATOS], int numDatos, const char *scriptGnuplot, Point nuevoPunto) {
    FILE *script = fopen(scriptGnuplot, "w");
    if (script) {
        fprintf(script, "set title 'Grafica de Asteroides y Escala Palermo'\n");
        fprintf(script, "set xlabel 'Asteroid Velocity (km/s)'\n");
        fprintf(script, "set ylabel 'Maximum Palermo Scale'\n");
        fprintf(script, "set grid\n");

        fprintf(script, "plot '-' using 1:2 with points pt 7 lc rgb 'green' title 'Potentially Hazardous',\\\n");
        fprintf(script, "     '-' using 1:2 with points pt 7 lc rgb 'blue' title 'Not Hazardous',\\\n");
        fprintf(script, "     '-' using 1:2 with points pt 2 lc rgb 'red' title 'Nuevo Punto'\n");

        for (int i = 0; i < numDatos; i++) {
            if (puntos[i].hazard == POTENTIALLY_HAZARDOUS) {
                fprintf(script, "%f %f\n", puntos[i].x, puntos[i].y);
            }
        }
        fprintf(script, "e\n");

        for (int i = 0; i < numDatos; i++) {
            if (puntos[i].hazard == NOT_HAZARDOUS) {
                fprintf(script, "%f %f\n", puntos[i].x, puntos[i].y);
            }
        }
        fprintf(script, "e\n");

        fprintf(script, "%f %f\n", nuevoPunto.x, nuevoPunto.y);
        fprintf(script, "e\n");

        fclose(script);
        printf("Script de Gnuplot generado: %s\n", scriptGnuplot);
    } else {
        perror("Error al crear el script de Gnuplot");
    }
}

int main() {
    const char *archivoCSV = "ruta_al_archivo.csv"; 
    const char *scriptGnuplot = "grafica.gnuplot";  

    Point puntos[MAX_DATOS];  
    int numDatos = cargarDatos(archivoCSV, puntos);
    if (numDatos == 0) {
        fprintf(stderr, "No se cargaron datos validos.\n");
        return 1;
    }

    Point nuevoPunto = {0.0, 0.0}; 
    HazardType nuevoHazard;  

    generarScriptGnuplot(puntos, numDatos, scriptGnuplot, nuevoPunto);

    char comando[256];
    snprintf(comando, sizeof(comando), "gnuplot -p %s", scriptGnuplot);
    if (system(comando) != 0) {
        fprintf(stderr, "Error al ejecutar Gnuplot.\n");
    }

    char continuar = 'y';
    while (continuar == 'y' || continuar == 'Y') {
        printf("Ingresa la velocidad del asteroide (km/s): ");
        scanf("%f", &nuevoPunto.x);
        printf("Ingresa la escala maxima Palermo: ");
        scanf("%f", &nuevoPunto.y);

        nuevoHazard = clasificarNuevoPunto(puntos, numDatos, nuevoPunto);
        nuevoPunto.hazard = nuevoHazard;

        generarScriptGnuplot(puntos, numDatos, scriptGnuplot, nuevoPunto);

        snprintf(comando, sizeof(comando), "gnuplot -p %s", scriptGnuplot);
        if (system(comando) != 0) {
            fprintf(stderr, "Error al ejecutar Gnuplot.\n");
        }

        printf("Deseas agregar otro punto? (y/n): ");
        scanf(" %c", &continuar);  
    }
    
    return 0;
}
