#pragma once
#include "eventos.h"

// ============================================================
// SIMULADOR DE EVENTOS
// ============================================================
//
// ¿Qué es el Monitor Serie?
// El Monitor Serie es una herramienta del Arduino IDE y
// PlatformIO que permite enviar y recibir texto entre
// el ordenador y la placa a través del cable USB.
//
// Funciona como una "ventana de texto":
//   - Lo que escribe el programa (Serial.println) aparece ahí.
//   - Lo que escribimos nosotros lo recibe el programa.
//
// En el Prototipo Nivel 1, usamos el Monitor Serie para
// simular eventos: escribimos un número y el programa
// actúa como si hubiera detectado ese evento con sensores reales.
//
// ¿Qué es Serial.available()?
// Serial.available() devuelve cuántos bytes (caracteres) hay
// esperando para ser leídos. Si devuelve 0, no hay nada.
// Si devuelve algo mayor que 0, hay texto para leer.
//
// ¿Qué es Serial.parseInt()?
// Lee un número entero desde el Monitor Serie.
// Espera hasta que llega un número y lo devuelve como int.
//
// ¿Qué es Serial.read()?
// Lee UN byte del buffer del Monitor Serie y lo descarta.
// Se usa aquí para limpiar el salto de línea '\n' que queda
// después de leer el número con parseInt().
// ============================================================

// ¿Está activo el modo simulación?
// Si cambiamos esto a "false", el programa intentará leer
// los sensores reales de la K128.
#define MODO_SIMULACION true

// Lee el evento desde el Monitor Serie (modo simulación)
// Devuelve el evento que corresponde al número escrito.
// Si no hay nada escrito, devuelve -1 como señal de "nada nuevo".
//
// Nota: devolvemos int en lugar de EventoSistema porque
// necesitamos poder indicar "sin datos" con -1, que no
// existe en el enum. La conversión se hace en leerEvento().
inline int leerEventoSimuladoRaw() {
    if (Serial.available() > 0) {
        int numero = Serial.parseInt();

        // Limpiar el buffer de cualquier carácter sobrante
        // (como el '\n' al final de la línea)
        while (Serial.available() > 0) {
            Serial.read();
        }

        return numero;
    }
    return -1;  // Sin datos
}

// Convierte el número recibido en un EventoSistema
// -1 → sin datos (devuelve NORMAL como valor neutro)
//  0 → NORMAL
//  1 → FRENAZO_SUAVE
//  2 → FRENAZO_FUERTE
//  3 → VOLANTAZO
//  4 → IMPACTO
//  5 → REPOSO
// 99 → DESCONOCIDO
// cualquier otro → DESCONOCIDO
inline EventoSistema numeroAEvento(int numero) {
    switch (numero) {
        case  0: return EventoSistema::NORMAL;
        case  1: return EventoSistema::FRENAZO_SUAVE;
        case  2: return EventoSistema::FRENAZO_FUERTE;
        case  3: return EventoSistema::VOLANTAZO;
        case  4: return EventoSistema::IMPACTO;
        case  5: return EventoSistema::REPOSO;
        case 99: return EventoSistema::DESCONOCIDO;
        default: return EventoSistema::DESCONOCIDO;
    }
}

// Imprime el menú de comandos disponibles
inline void mostrarMenuSimulador() {
    Serial.println("========================================");
    Serial.println("  VirtualFriend - Modo Simulacion");
    Serial.println("  Escribe un numero y pulsa Enter:");
    Serial.println("  0  = Normal");
    Serial.println("  1  = Frenazo suave");
    Serial.println("  2  = Frenazo fuerte");
    Serial.println("  3  = Volantazo");
    Serial.println("  4  = Impacto");
    Serial.println("  5  = Reposo");
    Serial.println("  99 = Evento desconocido");
    Serial.println("========================================");
}

// ============================================================
// FUTURO: Lectura de sensores reales de la K128
// ============================================================
// Cuando se conecten los sensores reales, esta función
// leerá el acelerómetro o giroscopio de la K128 y
// devolverá el evento que corresponda.
//
// EventoSistema leerEventoRealK128() {
//     float accel = leerAcelerometro();
//     float giro  = leerGiroscopio();
//
//     if (accel > UMBRAL_IMPACTO)        return EventoSistema::IMPACTO;
//     if (accel > UMBRAL_FRENAZO_FUERTE) return EventoSistema::FRENAZO_FUERTE;
//     if (accel > UMBRAL_FRENAZO_SUAVE)  return EventoSistema::FRENAZO_SUAVE;
//     if (giro  > UMBRAL_VOLANTAZO)      return EventoSistema::VOLANTAZO;
//     if (tiempoSinMovimiento > UMBRAL_REPOSO) return EventoSistema::REPOSO;
//     return EventoSistema::NORMAL;
// }
// ============================================================
