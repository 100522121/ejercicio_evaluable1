#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
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

// Nombre de la cola del servidor (debe coincidir con el servidor)
const char *SERVER_QUEUE = "/servidor_mq_ariana";

// Función auxiliar para gestionar la comunicación con el servidor
static int realizar_peticion(struct Peticion *pet, struct Respuesta *res) {
    mqd_t q_servidor, q_cliente;
    struct mq_attr attr;
    char nombre_cola_cliente[256];

    // Crear nombre de cola único para este proceso/hilo
    sprintf(nombre_cola_cliente, "/client_%d_%ld", getpid(), (long)res); // Usamos la dirección de res para unicidad en hilos
    strcpy(pet->q_nombre, nombre_cola_cliente);

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct Respuesta);

    // 1. Crear cola de respuesta
    mq_unlink(nombre_cola_cliente);
    q_cliente = mq_open(nombre_cola_cliente, O_CREAT | O_RDONLY, 0666, &attr);
    if (q_cliente == (mqd_t)-1) return -2;

    // 2. Abrir cola del servidor
    q_servidor = mq_open(SERVER_QUEUE, O_WRONLY);
    if (q_servidor == (mqd_t)-1) {
        mq_close(q_cliente);
        mq_unlink(nombre_cola_cliente);
        return -2;
    }

    // 3. Enviar petición
    if (mq_send(q_servidor, (const char *)pet, sizeof(struct Peticion), 0) == -1) {
        mq_close(q_servidor);
        mq_close(q_cliente);
        mq_unlink(nombre_cola_cliente);
        return -2;
    }

    // 4. Recibir respuesta
    if (mq_receive(q_cliente, (char *)res, sizeof(struct Respuesta), NULL) == -1) {
        mq_close(q_servidor);
        mq_close(q_cliente);
        mq_unlink(nombre_cola_cliente);
        return -2;
    }

    // 5. Limpieza
    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(nombre_cola_cliente);

    return res->resultado;
}

int destroy(void) {
    struct Peticion pet;
    struct Respuesta res;
    memset(&pet, 0, sizeof(pet));
    pet.op = OP_DESTROY;
    return realizar_peticion(&pet, &res);
}

int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    if (key == NULL || value1 == NULL || V_value2 == NULL) return -1;
    if (strlen(value1) > 255 || N_value2 > 32 || N_value2 < 1) return -1;

    struct Peticion pet;
    struct Respuesta res;
    memset(&pet, 0, sizeof(pet));
    
    pet.op = OP_SET_VALUE;
    strncpy(pet.key, key, MAX_KEY - 1);
    strncpy(pet.value1, value1, MAX_VAL1 - 1);
    pet.N_value2 = N_value2;
    for (int i = 0; i < N_value2; i++) pet.V_value2[i] = V_value2[i];
    pet.value3 = value3;

    return realizar_peticion(&pet, &res);
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    if (key == NULL || value1 == NULL || N_value2 == NULL || V_value2 == NULL || value3 == NULL) return -1;

    struct Peticion pet;
    struct Respuesta res;
    memset(&pet, 0, sizeof(pet));

    pet.op = OP_GET_VALUE;
    strncpy(pet.key, key, MAX_KEY - 1);

    int ret = realizar_peticion(&pet, &res);
    if (ret == 0) {
        strcpy(value1, res.value1);
        *N_value2 = res.N_value2;
        for (int i = 0; i < res.N_value2; i++) V_value2[i] = res.V_value2[i];
        *value3 = res.value3;
    }
    return ret;
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    if (key == NULL || value1 == NULL || V_value2 == NULL) return -1;
    if (strlen(value1) > 255 || N_value2 > 32 || N_value2 < 1) return -1;

    struct Peticion pet;
    struct Respuesta res;
    memset(&pet, 0, sizeof(pet));

    pet.op = OP_MODIFY_VALUE;
    strncpy(pet.key, key, MAX_KEY - 1);
    strncpy(pet.value1, value1, MAX_VAL1 - 1);
    pet.N_value2 = N_value2;
    for (int i = 0; i < N_value2; i++) pet.V_value2[i] = V_value2[i];
    pet.value3 = value3;

    return realizar_peticion(&pet, &res);
}

int delete_key(char *key) {
    if (key == NULL) return -1;

    struct Peticion pet;
    struct Respuesta res;
    memset(&pet, 0, sizeof(pet));

    pet.op = OP_DELETE_KEY;
    strncpy(pet.key, key, MAX_KEY - 1);

    return realizar_peticion(&pet, &res);
}

int exist(char *key) {
    if (key == NULL) return -1;

    struct Peticion pet;
    struct Respuesta res;
    memset(&pet, 0, sizeof(pet));

    pet.op = OP_EXIST;
    strncpy(pet.key, key, MAX_KEY - 1);

    return realizar_peticion(&pet, &res);
}
