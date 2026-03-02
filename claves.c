#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "claves.h"

// Estructura interna para almacenar los nodos de la lista enlazada
typedef struct Elemento {
    char key[256];
    char value1[256];
    int N_value2;
    float V_value2[32];
    struct Paquete value3;
    struct Elemento *next;
} Elemento;

// Puntero al inicio de la lista
static Elemento *lista = NULL;

// Mutex para garantizar la atomicidad de las operaciones
static pthread_mutex_t mutex_lista = PTHREAD_MUTEX_INITIALIZER;

int destroy(void) {
    // OBS: ¿En qué caso podría dar error?
    pthread_mutex_lock(&mutex_lista);
    Elemento *actual = lista;
    while (actual != NULL) {
        Elemento *siguiente = actual->next;
        free(actual);   // Liberamos memoria para cada elem eliminado
        actual = siguiente;
    }
    lista = NULL;
    pthread_mutex_unlock(&mutex_lista);
    return 0;
}

int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    // Se considera también error que el valor N_value2 esté fuera de rango.
    if (N_value2 < 1 || N_value2 > 32) return -1;
    if (key == NULL || value1 == NULL) return -1;

    pthread_mutex_lock(&mutex_lista);
    
    // Comprobar si la clave ya existe
    Elemento *temp = lista;
    while (temp != NULL) {
        if (strcmp(temp->key, key) == 0) {
            pthread_mutex_unlock(&mutex_lista);
            return -1; // Error: clave duplicada
        }
        // Para recorrer los elementos
        temp = temp->next;
    }

    // Alocamos el espacio en memoria para el nuevo elem
    Elemento *nuevo = (Elemento *)malloc(sizeof(Elemento));
    if (nuevo == NULL) {
        pthread_mutex_unlock(&mutex_lista);
        return -1;
    }

    strncpy(nuevo->key, key, 255);
    nuevo->key[255] = '\0'; // End of string
    strncpy(nuevo->value1, value1, 255);
    nuevo->value1[255] = '\0';  // End of string
    nuevo->N_value2 = N_value2;
    // Para cadaa valor que estará en el vector
    for (int i = 0; i < N_value2; i++) {  
        nuevo->V_value2[i] = V_value2[i];
    }
    nuevo->value3 = value3;
    nuevo->next = lista;
    lista = nuevo;

    pthread_mutex_unlock(&mutex_lista);
    return 0;
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    if (key == NULL) return -1;
    
    //OBS: ¿Se está revisando que V_value2 y value1 espacio reservado para poder almacenar el máximo número 
    //de elementos posibles 256 en el caso de la cadena de caracteres y 32 en el caso del vector de floats?   
    pthread_mutex_lock(&mutex_lista);
    Elemento *actual = lista;
    while (actual != NULL) {
        if (strcmp(actual->key, key) == 0) {
            strcpy(value1, actual->value1);
            *N_value2 = actual->N_value2;
            for (int i = 0; i < actual->N_value2; i++) {
                V_value2[i] = actual->V_value2[i];
            }
            *value3 = actual->value3;
            pthread_mutex_unlock(&mutex_lista);
            return 0;
        }
        actual = actual->next;
    }
    pthread_mutex_unlock(&mutex_lista);
    return -1; // No encontrado (no existe dicho elem)
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    // Siendo N_value2 el tamaño del vector, ver si está en el rango
    if (N_value2 < 1 || N_value2 > 32 || key == NULL || value1 == NULL) return -1;
    // OBS: ¿Se asume que los demás parámetros dados son correctos o también habría que revisar?
    // Considerando que en todos los casos se indica que "Un error puede ocurrir en este caso por un problema en las comunicaciones."
    // ¿O se refiere a la sincronización?
    pthread_mutex_lock(&mutex_lista);
    Elemento *actual = lista;
    while (actual != NULL) {
        // Comparación de strings
        if (strcmp(actual->key, key) == 0) {
            // Copia de string
            strncpy(actual->value1, value1, 255);
            actual->value1[255] = '\0';
            actual->N_value2 = N_value2;
            for (int i = 0; i < N_value2; i++) {
                actual->V_value2[i] = V_value2[i];
            }
            actual->value3 = value3;
            pthread_mutex_unlock(&mutex_lista);
            return 0;
        }
        actual = actual->next;
    }
    pthread_mutex_unlock(&mutex_lista);
    return -1; // No encontrado
}

int delete_key(char *key) {
    if (key == NULL) return -1;

    pthread_mutex_lock(&mutex_lista);
    Elemento *actual = lista;
    Elemento *anterior = NULL;

    while (actual != NULL) {
        if (strcmp(actual->key, key) == 0) {
            if (anterior == NULL) {
                // Esto significa que es el primer elemento
                lista = actual->next;
            } else {
                // Esto significa que NO es el primer elemento,
                // entonces el next del anterior pasa a ser el next del actual, tal que el actual sea borrado
                anterior->next = actual->next;
            }
            free(actual);
            pthread_mutex_unlock(&mutex_lista);
            return 0;
        }
        anterior = actual;
        actual = actual->next;
    }
    pthread_mutex_unlock(&mutex_lista);
    return -1; // No encontrado
}

int exist(char *key) {
    if (key == NULL) return -1;

    pthread_mutex_lock(&mutex_lista);
    Elemento *actual = lista;
    while (actual != NULL) {
        // En caso coincida, devuelvo 1 porque lo encontró
        if (strcmp(actual->key, key) == 0) {
            pthread_mutex_unlock(&mutex_lista);
            return 1;
        }
        actual = actual->next; // Va recorriendo al siguiente
    }
    pthread_mutex_unlock(&mutex_lista);
    return 0;
}
