#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include "claves.h"

// Función auxiliar para que el plan de pruebas sea fácil de leer
void print_test_result(const char* test_name, int result) {
    if (result == 0) {
        printf("[OK] %s\n", test_name);
    } else if (result == 1) {
        printf("[OK] %s (Resultado: 1/Existe)\n", test_name);
    } else {
        printf("[ERROR] %s\n", test_name);
    }
}

int main() {
    printf("--- Inicio de Plan de Pruebas (Versión No Distribuida) ---\n\n");

    // Datos de prueba iniciales
    char *key1 = "clave1";
    char *val1 = "valor_string_1";
    int n2 = 3;
    float v2[] = {1.1, 2.2, 3.3};
    struct Paquete p = {10, 20, 30};

    // 1. Función destroy(): Limpiar el servicio
    printf("Prueba 1: destroy()\n");
    print_test_result("destroy", destroy());

    // 2. Función set_value(): Insertar un elemento válido
    printf("\nPrueba 2: set_value()\n");
    print_test_result("set_value clave1", set_value(key1, val1, n2, v2, p));

    // 3. Función exist(): Comprobar si existe
    printf("\nPrueba 3: exist()\n");
    print_test_result("exist clave1", exist(key1));
    if (exist("no_existe") == 0) {
        printf("[OK] exist clave inexistente (Resultado: 0)\n");
    } else {
        printf("[ERROR] exist clave inexistente\n");
    }

    // 4. Función get_value(): Recuperar y mostrar los valores
    printf("\nPrueba 4: get_value()\n");
    char res_val1[256];
    int res_n2;
    float res_v2[32];
    struct Paquete res_p;
    if (get_value(key1, res_val1, &res_n2, res_v2, &res_p) == 0) {
        printf("[OK] get_value clave1\n");
        printf("  - Val1 recuperado: %s\n", res_val1);
        printf("  - N2 recuperado: %d\n", res_n2);
        printf("  - V2[0]: %.1f, V2[1]: %.1f, V2[2]: %.1f\n", res_v2[0], res_v2[1], res_v2[2]);
        printf("  - Paquete: x=%d, y=%d, z=%d\n", res_p.x, res_p.y, res_p.z);
    } else {
        printf("[ERROR] get_value clave1\n");
    }

    // 5. Función modify_value(): Modificar un valor existente
    printf("\nPrueba 5: modify_value()\n");
    char *new_val1 = "valor_modificado";
    v2[0] = 9.9; // Cambiamos el primer valor del vector
    p.x = 100;   // Cambiamos x en el paquete
    print_test_result("modify_value clave1", modify_value(key1, new_val1, n2, v2, p));
    
    // Verificar que se ha modificado de verdad
    get_value(key1, res_val1, &res_n2, res_v2, &res_p);
    if (strcmp(res_val1, new_val1) == 0 && res_v2[0] > 9.8 && res_p.x == 100) {
        printf("[OK] Verificación de modificación exitosa\n");
    } else {
        printf("[ERROR] Verificación de modificación fallida\n");
    }

    // 6. Función set_value(): Pruebas de robustez (Errores controlados)
    printf("\nPrueba 6: Casos de error\n");
    if (set_value(key1, "duplicado", 1, v2, p) == -1) {
        printf("[OK] Error detectado correctamente: No permite claves duplicadas\n");
    }
    if (set_value("clave2", "rango", 33, v2, p) == -1) {
        printf("[OK] Error detectado correctamente: N_value2 fuera de rango (>32)\n");
    }

    // 7. Función delete_key(): Borrar y verificar
    printf("\nPrueba 7: delete_key()\n");
    print_test_result("delete_key clave1", delete_key(key1));
    if (exist(key1) == 0) {
        printf("[OK] Verificación: La clave ya no existe\n");
    }
    if (delete_key("no_existe") == -1) {
        printf("[OK] Error detectado correctamente: No se puede borrar lo que no existe\n");
    }

    printf("\n--- Fin de Pruebas (Versión No Distribuida) ---\n");
    return 0;
}