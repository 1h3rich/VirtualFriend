# Cloudflare Worker — VirtualFriend

## Datos del despliegue

| Campo | Valor |
|-------|-------|
| Nombre del Worker | `virtualfriend-worker` |
| URL pública | `https://virtualfriend-worker.rykhjob.workers.dev` |
| Subdominio Cloudflare | `rykhjob.workers.dev` |
| Account ID | `5ec1e21526952b525dd63b8e66571d3b` |
| Email cuenta | `rykhjob@gmail.com` |

---

## Secrets guardados en Cloudflare

| Secret | Descripción |
|--------|-------------|
| `GEMINI_API_KEY` | API key de Google Gemini (guardada en Escritorio/Proyecto/Api/Gemini.txt) |
| `DEVICE_TOKEN` | `(ver src/Secrets.h — no publicar)` |

> Los secrets NO están en el código. Están en Cloudflare de forma segura.

---

## Carpeta del proyecto

```
~/Proyectos/virtualfriend-worker/
  src/
    index.js          ← lógica principal del Worker
  test/
    index.spec.js     ← tests de validación
  wrangler.jsonc      ← configuración de Wrangler
  .dev.vars           ← secrets locales para desarrollo (no en git)
  package.json
```

---

## Cómo usar el Worker desde el M5Stack

### Petición HTTP

```
Método:  POST
URL:     https://virtualfriend-worker.rykhjob.workers.dev
```

### Cabeceras obligatorias

```
Content-Type: application/json
X-Device-Token: (ver src/Secrets.h — no publicar)
```

### Body (JSON)

```json
{
  "message": "Haz un comentario sarcástico sobre este viaje",
  "mood": "sarcastico",
  "movement": "normal"
}
```

### Respuesta del Worker

```json
{
  "text": "Ah, sí, otro viaje para añadir a la colección.",
  "expression": "sarcastic",
  "sound": "none"
}
```

### Valores válidos

| Campo | Valores posibles |
|-------|-----------------|
| `expression` | `happy`, `sarcastic`, `serious`, `surprised`, `sleepy` |
| `sound` | `none`, `laugh`, `beep`, `warning` |

---

## Códigos de error

| HTTP | Causa |
|------|-------|
| `405` | Método no es POST |
| `401` | Token ausente o incorrecto |
| `400` | Campo `message` vacío o ausente |
| `500` | Error interno — el Worker devuelve respuesta de fallback |

---

## Comandos útiles

```bash
cd ~/Proyectos/virtualfriend-worker

# Servidor local de desarrollo
npx wrangler dev

# Ver logs en tiempo real del Worker desplegado
npx wrangler tail

# Desplegar nueva versión
npx wrangler deploy

# Ver secrets guardados
npx wrangler secret list

# Ejecutar tests
npm test
```

---

## Notas técnicas importantes

- **Gemini 2.5 Flash** es un modelo "thinking" — consume tokens pensando antes de responder.
  Se desactivó el thinking (`thinkingBudget: 0`) para respuestas rápidas y predecibles en el coche.
- **TLS en este equipo**: hay un problema con TLS 1.3 + ECH en CachyOS. Para probar con curl usar `--tlsv1.2 --tls-max 1.2`. El M5Stack no tiene este problema, usa TLS estándar.
- **Arquitectura futura**: la carpeta `src/providers/` está prevista para añadir OpenAI, Claude, Mistral, Groq, xAI sin tocar `index.js`.

---

*Creado: 2026-06-03 — Sesión 4*
