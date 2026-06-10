# VirtualFriend — Diseño Técnico de Carcasa

## 1. Concepto general

Carcasa en dos piezas para salpicadero de coche. Cuerpo principal con apertura frontal
(pantalla + rejilla altavoz) y tapa trasera desmontable con tornillos M3. El conjunto
está inclinado 8° hacia atrás para que la pantalla y el altavoz apunten directamente
al conductor/copiloto. El zócalo de base sobresale lateralmente para mayor estabilidad
y admite velcro o adhesivo 3M en la cara inferior.

---

## 2. Distribución interna de componentes

```
┌─────────────────────────────────────────────────────────────────────┐
│  FRONTAL (cara visible)                                             │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  VENTANA PANTALLA M5Stack  50.5 × 42 mm                      │  │
│  │                                                               │  │
│  │  Asiento M5Stack CoreS3                56.5 × 56.5 × 32.5    │  │
│  │  (retenido por 3 paredes laterales + base)                    │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  REJILLA ALTAVOZ  72 × 33 mm (ranuras verticales 1.5mm)      │  │
│  │  Asiento altavoz con labio perimetral + espacio acústico      │  │
│  │  Amplificador MAX98357A en repisa lateral derecha             │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │  ZONA CAN/OBD (expansión futura)  40 × 35 mm               │    │
│  │  Delimitada por nervios internos, accesible desde tapa      │    │
│  └─────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. Medidas exteriores

| Dimensión             | Valor      |
|-----------------------|------------|
| Ancho                 | 112 mm     |
| Alto                  | 88 mm      |
| Profundidad           | 52 mm      |
| Radio esquinas        | 4 mm       |
| Inclinación           | 8°         |
| Zócalo (sobresale)    | +8 mm c/lado |

---

## 4. Medidas internas (espacio útil)

| Zona                  | Dimensión              |
|-----------------------|------------------------|
| Interior general      | 106.4 × 82.4 × 45.2 mm |
| Hueco M5Stack         | 56.5 × 56.5 × 32.5 mm  |
| Hueco altavoz         | 72 × 33 × 8 mm         |
| Zona amplificador     | 25 × 18 × 10 mm        |
| Zona CAN/OBD futura   | 40 × 35 × 20 mm        |
| Paredes               | 2.8 mm general / 3.0 mm base |

---

## 5. Ubicación del M5Stack CoreS3

- Posición: zona superior del cuerpo, centrado horizontalmente.
- Retenido por 3 paredes interiores (izquierda, derecha, superior) de 1.5 mm de grosor.
- La cara frontal (pantalla) queda alineada con la apertura de la carcasa.
- Margen superior: 4 mm (para fácil extracción).
- Cables salen por la parte inferior hacia el compartimento del amplificador y CAN.

---

## 6. Ubicación del altavoz

- Posición: zona inferior frontal, debajo del M5Stack.
- Separación entre M5Stack y altavoz: 4 mm (para paso de cables y disipación).
- Orientación: cono mirando hacia frontal (hacia el conductor).
- Asiento con labio perimetral de 1.5 mm para sellar con junta de espuma/goma.
- Cámara acústica detrás: ~8 mm de profundidad antes de la zona de amplificador.
- Rejilla frontal: ranuras verticales de 1.5 mm con separación de 2.5 mm, margen perimetral 4 mm.
- El cono del altavoz no toca plástico ni cables (mínimo 2 mm libre).

---

## 7. Ubicación del MAX98357A

- Posición: repisa interior lateral derecha, entre el asiento del altavoz y la zona CAN.
- Altura: a 9 mm del suelo interior (encima del asiento del altavoz).
- Accesible sin extraer el M5Stack (abriendo la tapa trasera).
- Cables que salen:
  - 5V / GND desde M5Stack
  - BCLK, LRC/WS, DIN desde pines I2S del M5Stack
  - + / - hacia altavoz (corto recorrido, altavoz justo encima)

---

## 8. Hueco futuro Jack 3.5mm

- Posición: lateral izquierdo del cuerpo, a media altura de la zona M5Stack.
- El agujero está **marcado pero cerrado** en el diseño actual (pared sólida).
- Basta con taladrar Ø7 mm para instalar el conector jack.
- Espacio interior reservado para DAC PCM5102A (~20×15×5 mm) cerca de ese lateral.
- El jack **no** vendrá de la salida amplificada; vendrá del DAC I2S futuro.

---

## 9. Hueco futuro Módulo CAN/OBD

- Posición: zona inferior del compartimento interior.
- Dimensiones reservadas: 40 × 35 × 20 mm.
- Delimitada por dos nervios verticales de 1 mm de alto (marcadores visuales, no estructurales).
- Accesible directamente al abrir la tapa trasera.
- Salida de cable: ranura inferior trasera de 20×4 mm ya perforada para paso de cable OBD2.
- No interfiere con el prototipo actual.

---

## 10. Sistema de cierre con tornillos

- 4 columnas M3 en las esquinas interiores del cuerpo.
  - Diámetro exterior columna: 6.5 mm.
  - Agujero interior: 2.5 mm (para roscar con macho M3 o insertar inserto de calor M3).
  - Altura: 12 mm.
- Tapa trasera con 4 agujeros pasantes M3 + avellanado para cabeza.
- Tornillos recomendados: **M3×12 cabeza avellanada** (ISO 10642 / DIN 7991).
- El labio de encaje (2 mm de profundidad, tolerancia 0.25 mm) centra la tapa antes de atornillar.

---

## 11. Sistema de fijación al salpicadero

El zócalo sobresale 8 mm por cada lado respecto al cuerpo. La cara inferior es plana.
Opciones de fijación:

| Opción              | Descripción                                       |
|---------------------|---------------------------------------------------|
| Velcro industrial   | Pegar en la base, pareja en el salpicadero        |
| Adhesivo 3M VHB     | Cinta doble cara de alta resistencia              |
| Base antideslizante | Lámina de silicona pegada en la base (temporal)   |
| Soporte articulado  | Adaptar brazo de ventosa/rejilla con tornillo M4  |

Para el soporte articulado: el zócalo tiene espacio suficiente para añadir un agujero
M4 centrado (modificar el parámetro y regenerar el STL).

---

## 12. Recomendaciones de impresión 3D

| Parámetro          | Cuerpo principal     | Tapa trasera        |
|--------------------|----------------------|---------------------|
| Material           | PETG / ASA           | PETG / ASA          |
| Orientación        | Cara frontal abajo   | Cara exterior abajo |
| Capas              | 0.2 mm               | 0.2 mm              |
| Relleno            | 25–30%               | 20%                 |
| Perímetros         | 3–4                  | 3                   |
| Soportes           | No necesarios        | No necesarios       |
| Temperatura cama   | 70°C (PETG) / 100°C (ASA) | igual          |

- **No se necesitan soportes** con la orientación indicada.
- La rejilla del altavoz y la ventana de pantalla se imprimen sin soportes al orientar cara frontal abajo.
- Las columnas de tornillo se imprimen en vertical → buen acabado de agujero.
- Si se usa ASA: imprimir en recinto cerrado para evitar warping.

---

## 13. Lista de piezas necesarias

### Piezas impresas
| Pieza                    | Cantidad | Archivo                         |
|--------------------------|----------|---------------------------------|
| Cuerpo principal         | 1        | virtualfriend_carcasa.scad (PIEZA="cuerpo") |
| Tapa trasera             | 1        | virtualfriend_carcasa.scad (PIEZA="tapa")   |

### Ferretería
| Elemento                 | Cantidad | Especificación                  |
|--------------------------|----------|---------------------------------|
| Tornillo M3×12 avellanado| 4        | ISO 10642 acero/inox            |
| Inserto de calor M3      | 4        | Ø4.5mm×5.7mm (opcional, mejor rosca) |
| Junta espuma altavoz     | 1        | Cortar a medida 72×33mm, Ø3mm grosor |
| Velcro industrial        | 1 par    | 50×25mm mínimo                  |

### Electrónica interna
| Componente               | Cantidad | Notas                           |
|--------------------------|----------|---------------------------------|
| M5Stack CoreS3 K128      | 1        | Ya disponible                   |
| Amplificador MAX98357A   | 1        | Módulo breakout                 |
| Altavoz 4Ω/3W 70×31mm   | 1        | CQRobot Ocean o equivalente     |
| Cable Dupont hembra 6pin | 1        | Para conexión amp-M5Stack       |

### Futuras ampliaciones (no necesarias ahora)
| Componente               | Cantidad | Notas                           |
|--------------------------|----------|---------------------------------|
| Conector Jack 3.5mm PJ-3 | 1        | Montaje en panel, Ø7mm          |
| DAC PCM5102A             | 1        | Módulo breakout                 |
| Módulo CAN SN65HVD230    | 1        | Ya previsto en el proyecto      |

---

## 14. Posibles problemas del diseño

| Problema                              | Causa probable                    | Solución                              |
|---------------------------------------|-----------------------------------|---------------------------------------|
| M5Stack no encaja / muy ajustado      | Tolerancia de impresión variable  | Ajustar m5_w/m5_h ±0.3 mm en el .scad |
| Tapa trasera no cierra bien           | Shrinkage del material            | Reducir tapa_lip_gap de 0.25 a 0.15  |
| Resonancias del altavoz              | Cámara acústica pequeña           | Añadir espuma acústica en la cámara   |
| Columnas de tornillo se rompen        | Impresión a baja densidad         | Subir perímetros a 4, relleno 40%     |
| Calor en coche (verano, ASA)         | PLA o PETG a >60°C                | Usar ASA, evitar exposición solar directa |
| USB-C inaccesible                     | Posición errónea según orientación| Verificar antes de imprimir con mockup de papel |

---

## 15. Mejoras para una versión 2

1. **Inclinación variable**: añadir cuña de base intercambiable (5°, 10°, 15°) para distintas posiciones de salpicadero.
2. **Soporte articulado integrado**: brazo tipo "cuello de cisne" para montaje en rejilla AC o ventosa.
3. **Cámara acústica sellada**: compartimento trasero del altavoz completamente sellado con abertura tipo bass-reflex (tubo de 8mm).
4. **Gestión térmica**: ranuras de ventilación pasiva en laterales superiores.
5. **LED de estado**: agujero lateral pequeño (Ø5mm) para LED indicador de encendido/OBD.
6. **Tapa modular CAN**: cuando llegue el módulo CAN, tapa trasera con conector DB9 o conector OBD integrado.
7. **Canal de cables**: guías internas para organizar el cableado limpiamente.
8. **Etiqueta grabada**: texto "VirtualFriend" en relieve en la tapa trasera (fácil en OpenSCAD).
