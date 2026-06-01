#pragma once
#include <string.h>  // Para poder usar strcmp() (comparar texto)

// ============================================================
// EXPRESIONES DEL COMPAÑERO VIRTUAL
// ============================================================
//
// En el Prototipo Nivel 1, las expresiones se muestran
// por el Monitor Serie como texto ASCII.
//
// Cuando se conecte la pantalla real de la K128, solo
// habrá que cambiar el interior de estas funciones.
// El resto del programa no cambia, porque las funciones
// tienen el mismo nombre y reciben los mismos parámetros.
//
// Esto se llama "abstracción": el programa principal no sabe
// NI LE IMPORTA si la expresión va a una pantalla real o
// al Monitor Serie. Solo llama a la función y la función
// hace lo que toca en cada momento.
//
// ¿Qué es "inline"?
// Cuando una función es muy pequeña y se usa con frecuencia,
// poner "inline" le dice al compilador que puede copiar
// el código de la función directamente donde se llama,
// en lugar de hacer un salto de memoria. En placas pequeñas
// esto puede mejorar la velocidad y el uso de memoria.
//
// En archivos .h se usa inline para evitar errores de
// "función definida múltiples veces" si el archivo se
// incluye desde varios sitios.
// ============================================================

// Muestra la cara del personaje por Serial (simulación)
// Cuando se use pantalla real, se cambia el contenido de
// esta función por el código de la librería de pantalla.
inline void mostrarExpresion(const char* nombre) {
    Serial.println("----------------------------");
    Serial.print("[EXPRESION] ");
    Serial.println(nombre);

    // Caras ASCII para hacer el prototipo más visual
    if (strcmp(nombre, "Normal") == 0) {
        Serial.println("  o   o  ");
        Serial.println("    -    ");
        Serial.println("  \\___/  ");
    } else if (strcmp(nombre, "Feliz") == 0) {
        Serial.println("  ^   ^  ");
        Serial.println("         ");
        Serial.println("  \\___/  ");
    } else if (strcmp(nombre, "Sarcastico") == 0) {
        Serial.println("  -   o  ");
        Serial.println("    -    ");
        Serial.println("  \\___/  ");
    } else if (strcmp(nombre, "Asustado") == 0) {
        Serial.println("  O   O  ");
        Serial.println("    !    ");
        Serial.println("  /___\\  ");
    } else if (strcmp(nombre, "Alerta") == 0) {
        Serial.println("  > . <  ");
        Serial.println("   !!!   ");
        Serial.println("  /___\\  ");
    } else if (strcmp(nombre, "Dormido") == 0) {
        Serial.println("  -   -  ");
        Serial.println("    .    ");
        Serial.println("  \\___/  ");
    } else if (strcmp(nombre, "Error") == 0) {
        Serial.println("  x   x  ");
        Serial.println("    ?    ");
        Serial.println("  /___\\  ");
    }

    Serial.println("----------------------------");
}

// ============================================================
// FUTURO: Cuando se conecte la pantalla real de la K128
// ============================================================
// void mostrarExpresion(const char* nombre) {
//     tft.fillScreen(TFT_BLACK);  // Borrar pantalla
//     tft.drawBitmap(...);        // Dibujar cara desde imagen
//     // etc.
// }
// ============================================================
