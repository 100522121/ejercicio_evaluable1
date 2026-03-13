#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mqueue.h>
#include <string.h>
#include "claves.h"

#define MAX_KEY 256
#define MAX_VAL1 256

// Códigos de operación
enum {
    OP_DESTROY = 0,
    OP_SET_VALUE,
    OP_GET_VALUE,
    OP_MODIFY_VALUE,
    OP_DELETE_KEY,
    OP_EXIST
};

// Estructura de la petición (Cliente -> Servidor)
struct Peticion {
    int op;
    char q_nombre[256];
    char key[MAX_KEY];
    char value1[MAX_VAL1];
    int N_value2;
    float V_value2[32];
    struct Paquete value3;
};

// Estructura de la respuesta (Servidor -> Cliente)
struct Respuesta {
    int resultado;
    char value1[MAX_VAL1];
    int N_value2;
    float V_value2[32];
    struct Paquete value3;
};

// Función que ejecutará cada hilo para atender una petición
void *atender_peticion(void *arg) {
    struct Peticion pet = *(struct Peticion *)arg;
    free(arg);
    struct Respuesta res;
    mqd_t q_cliente;

    memset(&res, 0, sizeof(struct Respuesta));

    switch (pet.op) {
        case OP_DESTROY:      res.resultado = destroy(); break;
        case OP_SET_VALUE:    res.resultado = set_value(pet.key, pet.value1, pet.N_value2, pet.V_value2, pet.value3); break;
        case OP_GET_VALUE:    res.resultado = get_value(pet.key, res.value1, &res.N_value2, res.V_value2, &res.value3); break;
        case OP_MODIFY_VALUE: res.resultado = modify_value(pet.key, pet.value1, pet.N_value2, pet.V_value2, pet.value3); break;
        case OP_DELETE_KEY:   res.resultado = delete_key(pet.key); break;
        case OP_EXIST:        res.resultado = exist(pet.key); break;
        default: res.resultado = -1;
    }

    q_cliente = mq_open(pet.q_nombre, O_WRONLY);
    if (q_cliente != (mqd_t)-1) {
        mq_send(q_cliente, (const char *)&res, sizeof(res), 0);
        mq_close(q_cliente);
    }
    pthread_exit(NULL);
}

int main() {
    mqd_t q_servidor;
    struct Peticion *pet;
    struct mq_attr attr;

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct Peticion);

    // Usar un nombre de cola único para evitar conflictos
    const char *nombre_cola_servidor = "/servidor_mq_ariana";

    mq_unlink(nombre_cola_servidor); 
    q_servidor = mq_open(nombre_cola_servidor, O_CREAT | O_RDONLY, 0666, &attr);
    
    if (q_servidor == (mqd_t)-1) {
        perror("Error al crear cola del servidor");
        return -1;
    }

    printf("Servidor de Tuplas (POSIX MQ) iniciado y escuchando en %s...\n", nombre_cola_servidor);

    while (1) {
        pet = malloc(sizeof(struct Peticion));
        if (mq_receive(q_servidor, (char *)pet, sizeof(struct Peticion), NULL) != -1) {
            pthread_t hilo;
            if (pthread_create(&hilo, NULL, atender_peticion, (void *)pet) != 0) {
                perror("Error al crear hilo");
                free(pet);
            } else {
                pthread_detach(hilo); 
            }
        } else {
            perror("Error en mq_receive");
            free(pet);
        }
    }

    mq_close(q_servidor);
    return 0;
}
