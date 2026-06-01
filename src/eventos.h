#pragma once

// ============================================================
// EVENTOS DEL SISTEMA
// ============================================================
//
// ¿Qué es un enum?
// Un "enum" (abreviatura de "enumeración") es una forma de
// darle nombre a un conjunto de valores enteros.
// En lugar de usar números sueltos (0, 1, 2...) que no dicen
// nada por sí solos, le ponemos nombres con significado.
//
// Ejemplo sin enum (difícil de entender):
//   if (evento == 2) { ... }
//
// Ejemplo con enum (claro y legible):
//   if (evento == EVENTO_FRENAZO_FUERTE) { ... }
//
// El compilador traduce los nombres a números internamente,
// pero nosotros siempre trabajamos con los nombres.
//
// En C++ para Arduino, "enum class" es la forma moderna y segura
// de definir enumeraciones. Evita que los valores de diferentes
// enums se confundan entre sí.
// ============================================================

enum class EventoSistema {
    NORMAL,           // No pasa nada especial
    FRENAZO_SUAVE,    // Frenada leve detectada
    FRENAZO_FUERTE,   // Frenada brusca detectada
    VOLANTAZO,        // Movimiento lateral brusco
    IMPACTO,          // Golpe fuerte detectado
    REPOSO,           // Sin actividad durante un tiempo
    DESCONOCIDO       // Dato recibido que el programa no reconoce
};
