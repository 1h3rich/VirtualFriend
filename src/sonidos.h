#pragma once

// ============================================================
// SONIDOS DEL COMPAÑERO VIRTUAL
// ============================================================
//
// En el Prototipo Nivel 1, los sonidos se simulan imprimiendo
// un mensaje por el Monitor Serie.
//
// La ventaja de separar los sonidos en su propio archivo es
// que cuando se conecte el altavoz real de la K128, solo
// hay que cambiar el interior de estas funciones.
// El resto del programa sigue igual.
//
// Esto se llama "encapsulamiento": escondemos los detalles
// de implementación dentro de una función con nombre claro.
// Quien llame a "reproducirSonido()" no necesita saber si
// el sonido va por Serial, por un buzzer, por un altavoz
// DFPlayer Mini, o por cualquier otro sistema.
// ============================================================

// Reproduce (o simula) un sonido según su tipo
inline void reproducirSonido(const char* tipoSonido) {
    Serial.print("[SONIDO] ");
    Serial.println(tipoSonido);

    // --------------------------------------------------------
    // FUTURO: Aquí irá el código real para reproducir sonido
    // en la K128. Por ejemplo, si tiene un buzzer conectado:
    //
    //   tone(PIN_BUZZER, 1000, 200);  // 1000Hz durante 200ms
    //
    // O si tiene un módulo DFPlayer Mini:
    //
    //   dfPlayer.play(1);  // Reproducir archivo 1
    //
    // El número de archivo o la frecuencia dependerá del
    // hardware que lleve la K128 definitiva.
    // --------------------------------------------------------
}

// ============================================================
// Funciones de sonido específicas
// Llamar a estas funciones es más cómodo que recordar
// los nombres de los sonidos en cada parte del código.
// ============================================================

inline void sonidoInicio() {
    reproducirSonido("Inicio - bienvenida");
}

inline void sonidoConfirmacion() {
    reproducirSonido("Confirmacion");
}

inline void sonidoAlerta() {
    reproducirSonido("Alerta - beep urgente");
}

inline void sonidoError() {
    reproducirSonido("Error - tono raro");
}

inline void sonidoReposo() {
    reproducirSonido("Reposo - tono suave");
}
