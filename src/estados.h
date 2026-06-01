#pragma once

// ============================================================
// ESTADOS DEL COMPAÑERO VIRTUAL
// ============================================================
//
// ¿Qué es un estado?
// Un estado representa "cómo se encuentra" el personaje en
// un momento concreto. Es como el humor o la situación
// emocional del compañero virtual.
//
// El personaje solo puede estar en UN estado a la vez.
// Cuando ocurre un evento, el estado puede cambiar.
//
// Ejemplo:
//   El personaje está en NORMAL.
//   Llega un EVENTO_FRENAZO_FUERTE.
//   El personaje pasa a ASUSTADO.
//   Se muestra la expresión de asustado.
//   Se dice una frase seria.
//
// Los estados controlan qué expresión se muestra,
// qué frases se dicen y qué sonidos se emiten.
// ============================================================

enum class EstadoCompanero {
    INICIO,      // El sistema acaba de encenderse
    NORMAL,      // Todo va bien, estado tranquilo
    SARCASTICO,  // Algo leve ha pasado, modo bromista
    ASUSTADO,    // Algo fuerte ha pasado, modo serio-humorístico
    ALERTA,      // Situación grave, sin bromas
    REPOSO,      // Sin actividad, modo descanso
    ERROR_       // Evento desconocido o problema interno
    // Nota: se usa ERROR_ con guión bajo porque ERROR es una
    // palabra reservada en algunos compiladores de Arduino.
};
