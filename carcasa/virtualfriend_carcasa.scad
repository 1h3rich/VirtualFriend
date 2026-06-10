// ============================================================
//  VirtualFriend — Carcasa M5Stack CoreS3 K128
//  Convención de ejes: X=ancho, Y=profundidad(frente→atrás), Z=alto
//  La cara frontal (pantalla+altavoz) está en Y=0
// ============================================================
//
//  PIEZA = "cuerpo"  →  parte frontal
//  PIEZA = "tapa"    →  tapa trasera desmontable
//  PIEZA = "ambas"   →  cuerpo + tapa separada (montaje)
//  PIEZA = "seccion" →  sección en X para inspección interna

PIEZA = "ambas"; // ["cuerpo","tapa","ambas","seccion"]

$fn = 48;

// ── Exterior ──────────────────────────────────────────────
ext_w  = 112;   // Ancho  (X)
ext_d  =  52;   // Profundidad frente-atrás (Y)
ext_h  = 106;   // Alto   (Z) — 106mm para que quepan M5Stack+altavoz
r_ext  =   4;   // Radio esquinas exteriores

// ── Paredes ───────────────────────────────────────────────
pared      = 2.8;
pared_base = 3.0;   // suelo (Z=0)
tapa_d     = 4.0;   // grosor tapa trasera
tapa_lip   = 2.0;   // labio de encaje
tapa_gap   = 0.25;  // tolerancia labio

// ── Espacio interior útil ─────────────────────────────────
// int_w = ext_w - 2*pared   = 106.4 mm
// int_h = ext_h - 2*pared   = 100.4 mm
// int_d = ext_d - pared_base - tapa_d - tapa_lip = 43 mm

// ── M5Stack CoreS3 (medidas físicas +tolerancia) ──────────
m5_w = 56.5;   // X
m5_h = 56.5;   // Z (alto)
m5_d = 32.5;   // Y (profundidad)

// ── Ventana de pantalla ───────────────────────────────────
pant_w = 50.5;
pant_h = 42.0;
pant_r =  2.0;

// ── Altavoz (70×31mm, 4Ω/3W, +tolerancia) ────────────────
sp_w     = 72;   // X
sp_h     = 33;   // Z (alto del hueco)
sp_d     =  8;   // Y (profundidad asiento)
sp_margen = 4;   // espacio entre altavoz y M5Stack

// Rejilla: ranuras verticales
gr_slot  = 1.5;
gr_gap   = 2.5;
gr_marg  = 4.0;

// ── Amplificador MAX98357A ────────────────────────────────
amp_w = 25; amp_h = 18; amp_d = 10;

// ── Tornillos M3 ─────────────────────────────────────────
col_d_ext = 6.5;
col_d_int = 2.5;   // agujero roscado en cuerpo
tap_d_int = 2.8;   // agujero pasante en tapa
col_h     = 12;
col_marg  = 6;     // distancia desde esquina interior

// ── Zócalo base ───────────────────────────────────────────
base_ext  = 8;     // cuánto sobresale por cada lado
base_h    = 5;
base_r    = 3;

// ── USB-C (lateral derecho del CoreS3) ────────────────────
usb_w = 10.0;
usb_h =  5.0;

// ── Jack 3.5mm futuro (lateral izquierdo) ─────────────────
jack_d = 7.0;   // diámetro del taladro (marcado, no perforado)

// ============================================================
//  AUXILIARES
// ============================================================
module rcube(w, d, h, r) {
    r2 = min(r, d/2 - 0.01, w/2 - 0.01);  // evita colapso cuando r >= d/2
    linear_extrude(h)
        offset(r=r2, $fn=32) offset(r=-r2)
            square([w, d]);
}

module ventana(w, h, d, r) {
    linear_extrude(d + 1)
        offset(r=r, $fn=24) offset(r=-r)
            square([w, h]);
}

// Ranuras verticales de la rejilla del altavoz
module rejilla(w, h, grosor) {
    n = floor((w - gr_marg*2) / (gr_slot + gr_gap));
    for (i = [0:n-1]) {
        x = gr_marg + i*(gr_slot + gr_gap);
        translate([x, -0.5, gr_marg])
            cube([gr_slot, grosor + 1, h - gr_marg*2]);
    }
}

// Posiciones de las 4 columnas (coordenadas dentro del interior)
int_w_v = ext_w - pared*2;
int_h_v = ext_h - pared*2;

function cols() = [
    [col_marg,              col_marg],
    [int_w_v - col_marg*2,  col_marg],
    [col_marg,              int_h_v - col_marg*2],
    [int_w_v - col_marg*2,  int_h_v - col_marg*2],
];

module columna_cuerpo() {
    difference() {
        cylinder(h=col_h, d=col_d_ext);
        cylinder(h=col_h+1, d=col_d_int);
    }
}

module agujero_tapa(h) {
    cylinder(h=h+1, d=tap_d_int);
    translate([0,0,h-1.5]) cylinder(h=3, d=6.2); // avellanado
}

// ============================================================
//  POSICIONES CALCULADAS (centradas en el interior)
// ============================================================
// int_w = 106.4, int_h = 100.4
// M5Stack: centrado en X, pegado arriba con 4mm de margen
//   z_m5 = int_h - m5_h - 4 = 100.4 - 56.5 - 4 = 39.9
// Altavoz: centrado en X, justo debajo del M5Stack
//   z_sp = z_m5 - sp_h - sp_margen = 39.9 - 33 - 4 = 2.9  ← positivo ✓

int_w  = ext_w - pared*2;   // 106.4
int_h  = ext_h - pared*2;   // 100.4
int_d  = ext_d - pared_base - tapa_d - tapa_lip; // 43

x_m5   = (int_w - m5_w) / 2;               // 24.95
z_m5   = int_h - m5_h - 4;                 // 39.9
x_sp   = (int_w - sp_w) / 2;               // 17.2
z_sp   = z_m5 - sp_h - sp_margen;          // 2.9

x_pant = pared + x_m5 + (m5_w - pant_w)/2;  // 30.75
z_pant = pared + z_m5 + (m5_h - pant_h)/2;  // 50.15

x_amp  = pared + int_w - amp_w - 3;
z_amp  = pared + z_sp + sp_h + 2;

// ============================================================
//  CUERPO PRINCIPAL
// ============================================================
module cuerpo() {
    difference() {
        union() {
            // Exterior
            rcube(ext_w, ext_d, ext_h, r_ext);

            // Zócalo de base
            translate([-base_ext, -base_ext, 0])
                rcube(ext_w+base_ext*2, ext_d+base_ext*2, base_h, r_ext+base_ext);

            // Labio para encaje de tapa (cara trasera Y=ext_d)
            translate([pared-tapa_lip, ext_d-0.01, pared-tapa_lip])
                difference() {
                    rcube(int_w+tapa_lip*2, tapa_lip+0.01, int_h+tapa_lip*2, r_ext-pared+tapa_lip);
                    translate([tapa_lip, -0.5, tapa_lip])
                        rcube(int_w, tapa_lip+1.5, int_h, r_ext-pared);
                }
        }

        // ── Vaciado interior ──────────────────────────────
        translate([pared, pared_base, pared])
            rcube(int_w, int_d+tapa_d+tapa_lip+1, int_h, r_ext-pared);

        // ── Ventana pantalla (cara Y=0) ───────────────────
        translate([x_pant, -0.5, z_pant])
            ventana(pant_w, pant_h, pared+1, pant_r);

        // ── Rejilla altavoz (cara Y=0) ────────────────────
        translate([pared+x_sp, 0, pared+z_sp])
            rejilla(sp_w, sp_h, pared);

        // ── Acceso USB-C (lateral derecho X=ext_w) ────────
        translate([ext_w-0.5, pared_base+5, pared+z_m5+(m5_h-usb_h)/2])
            cube([pared+1, usb_w, usb_h]);

        // ── Hueco futuro Jack 3.5mm (lateral izq, cerrado) ─
        // Posición: lateral X=0, altura media del M5Stack
        // Para instalar: taladrar Ø7mm desde fuera
        // translate([-0.5, pared_base+8, pared+z_m5+m5_h/2])
        //     rotate([0,90,0]) cylinder(h=pared+1, d=jack_d);

        // ── Ranura paso cable OBD (base trasera) ──────────
        translate([ext_w/2-10, ext_d-pared-0.5, base_h+2])
            cube([20, pared+1, 4]);
    }

    // ── Columnas de tornillo ──────────────────────────────
    translate([pared, pared_base, pared]) {
        for (p = cols()) {
            translate([p[0], 0, p[1]])
                rotate([-90,0,0])
                    columna_cuerpo();
        }
    }

    // ── Asiento M5Stack (3 paredes laterales) ─────────────
    translate([pared+x_m5-1.5, pared_base, pared+z_m5-1.5]) {
        difference() {
            cube([m5_w+3, m5_d+1.5, m5_h+3]);
            // hueco para el M5Stack
            translate([1.5, 0, 1.5])
                cube([m5_w, m5_d+1, m5_h]);
            // apertura frontal (pantalla y botones)
            translate([-0.5, -0.5, -0.5])
                cube([m5_w+4, 4, m5_h+4]);
        }
    }

    // ── Asiento altavoz (labio perimetral) ────────────────
    translate([pared+x_sp-1.5, pared_base, pared+z_sp-1.5]) {
        difference() {
            cube([sp_w+3, sp_d+1.5, sp_h+3]);
            translate([1.5, 1.5, 1.5])
                cube([sp_w, sp_d+1, sp_h]);
            translate([-0.5, -0.5, -0.5])
                cube([sp_w+4, 3, sp_h+4]);
        }
    }

    // ── Repisa amplificador ───────────────────────────────
    translate([x_amp, pared_base+sp_d+2, z_amp])
        cube([amp_w+2, 1.5, amp_h+2]);

    // ── Nervios zona CAN/OBD (marcadores visuales) ────────
    can_x0 = pared + (int_w-40)/2;
    can_z0 = pared + 1;
    translate([can_x0-1, pared_base, can_z0]) cube([1, 25, 30]);
    translate([can_x0+40, pared_base, can_z0]) cube([1, 25, 30]);
}

// ============================================================
//  TAPA TRASERA
// ============================================================
module tapa() {
    difference() {
        union() {
            rcube(ext_w, tapa_d, ext_h, r_ext);

            // Labio de encaje
            translate([pared-tapa_lip+tapa_gap,
                       tapa_d-0.01,
                       pared-tapa_lip+tapa_gap])
                rcube(int_w+tapa_lip*2-tapa_gap*2,
                      tapa_lip+0.01,
                      int_h+tapa_lip*2-tapa_gap*2,
                      r_ext-pared+tapa_lip-tapa_gap);
        }

        // Agujeros pasantes tornillo M3
        translate([pared, 0, pared]) {
            for (p = cols()) {
                translate([p[0], 0, p[1]])
                    rotate([-90,0,0])
                        agujero_tapa(tapa_d+tapa_lip);
            }
        }

        // Ranuras de ventilación
        for (i=[0:4]) {
            translate([ext_w/2-26+i*13, -0.5, ext_h*0.35])
                cube([6, tapa_d+1, 18]);
        }
    }
}

// ============================================================
//  RENDER
// ============================================================
if (PIEZA == "cuerpo") {
    cuerpo();
} else if (PIEZA == "tapa") {
    tapa();
} else if (PIEZA == "ambas") {
    cuerpo();
    translate([0, ext_d+15, 0]) tapa();
} else if (PIEZA == "seccion") {
    intersection() {
        cuerpo();
        translate([ext_w/2, -1, -1]) cube([ext_w, ext_d+2, ext_h+2]);
    }
}
