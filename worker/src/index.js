export default {
	async fetch(request, env) {
		if (request.method !== "POST") {
			return jsonResponse({ error: "Method not allowed" }, 405);
		}

		const deviceToken = request.headers.get("X-Device-Token");
		if (!deviceToken || deviceToken !== env.DEVICE_TOKEN) {
			return jsonResponse({ error: "Unauthorized" }, 401);
		}

		const url = new URL(request.url);

		try {
			if (url.pathname === "/listen") {
				return await handleListen(request, env);
			}
			return await handleEvent(request, env);
		} catch (error) {
			console.error("Worker error:", error);
			return audioFallback("La IA está de baja. Yo también lo estaría si me preguntaran tanto.", env);
		}
	}
};

// ── Rate limiting (Cloudflare KV) ─────────────────────────────────────────────

const RATE_FALLBACK = {
	text:       "Hoy voy en modo ahorro. Sarcasmo limitado por presupuesto.",
	expression: "sarcastic",
	sound:      "beep",
};

async function checkRateLimit(env) {
	if (!env.RATE_KV) return true;
	const today = new Date().toISOString().slice(0, 10);
	const key   = `daily:${today}`;
	const limit = parseInt(env.DAILY_LIMIT ?? "50");
	const count = parseInt(await env.RATE_KV.get(key) ?? "0");
	if (count >= limit) return false;
	await env.RATE_KV.put(key, String(count + 1), { expirationTtl: 90000 });
	return true;
}

// ── /event (raíz) ─────────────────────────────────────────────────────────────

async function handleEvent(request, env) {
	const body = await request.json();
	const { message = "", mood = "sarcastico", movement = "normal" } = body;

	if (!message.trim()) {
		return jsonResponse({ error: "Empty message" }, 400);
	}

	if (!await checkRateLimit(env)) {
		const mp3 = await callElevenLabs(RATE_FALLBACK.text, env.ELEVENLABS_API_KEY);
		return audioResponse(RATE_FALLBACK, mp3);
	}

	const aiResult = await callOpenAI({ apiKey: env.OPENAI_KEY, message, mood, movement });
	const mp3      = await callElevenLabs(aiResult.text, env.ELEVENLABS_API_KEY);
	return audioResponse(aiResult, mp3);
}

// ── /listen ───────────────────────────────────────────────────────────────────

async function handleListen(request, env) {
	const contentLength = parseInt(request.headers.get("Content-Length") ?? "0");
	if (contentLength > 200_000) {
		return jsonResponse({ error: "Audio too large" }, 413);
	}

	const audioBuffer = await request.arrayBuffer();
	if (audioBuffer.byteLength === 0) {
		return jsonResponse({ error: "Empty audio" }, 400);
	}

	if (!await checkRateLimit(env)) {
		const mp3 = await callElevenLabs(RATE_FALLBACK.text, env.ELEVENLABS_API_KEY);
		return audioResponse(RATE_FALLBACK, mp3);
	}

	const transcript = await callWhisper(audioBuffer, env.OPENAI_KEY);
	console.log(`[Listen] transcript: "${transcript}"`);
	const aiResult   = await callOpenAIListen(transcript, env.OPENAI_KEY);
	console.log(`[Listen] response: "${aiResult.text}"`);
	const mp3        = await callElevenLabs(aiResult.text, env.ELEVENLABS_API_KEY);
	return audioResponse(aiResult, mp3);
}

// ── OpenAI: eventos de sensor ─────────────────────────────────────────────────

async function callOpenAI({ apiKey, message, mood, movement }) {
	const response = await fetch("https://api.openai.com/v1/chat/completions", {
		method: "POST",
		headers: {
			"Content-Type":  "application/json",
			"Authorization": `Bearer ${apiKey}`,
		},
		body: JSON.stringify({
			model:       "gpt-4o-mini",
			max_tokens:  100,
			temperature: 1.1,
			messages: [
				{
					role:    "system",
					content: `Eres el copiloto virtual de un coche: sarcástico, gracioso, directo. Cada respuesta debe ser DISTINTA. Usa ironía, metáforas, exageraciones. Máx 30 palabras. Responde SOLO JSON válido:
{"text":"frase","expression":"happy|sarcastic|serious|surprised|sleepy","sound":"none|laugh|beep|warning"}`,
				},
				{
					role:    "user",
					content: `mood=${mood} movement=${movement} situación: ${message}`,
				},
			],
		}),
	});

	if (!response.ok) {
		const err = await response.text();
		console.error(`[OpenAI] ${response.status}: ${err.slice(0, 200)}`);
		return { text: "La IA está de baja. Vuelvo en un momento.", expression: "sarcastic", sound: "beep" };
	}

	const data    = await response.json();
	const rawText = data?.choices?.[0]?.message?.content?.trim() ?? "";
	const result  = parseAiJson(rawText);
	console.log(`[OpenAI] ${result.text}`);
	return result;
}

// ── OpenAI Whisper: transcripción de audio ────────────────────────────────────

const WHISPER_HALLUCINATIONS = [
	"amara.org", "subtítulos", "subtitulos", "thank you for watching",
	"gracias por ver", "suscríbete", "suscribete", "like y suscríbete",
];

async function callWhisper(audioBuffer, apiKey) {
	const formData = new FormData();
	formData.append("file",     new Blob([audioBuffer], { type: "audio/wav" }), "audio.wav");
	formData.append("model",    "whisper-1");
	formData.append("language", "es");

	const response = await fetch("https://api.openai.com/v1/audio/transcriptions", {
		method:  "POST",
		headers: { "Authorization": `Bearer ${apiKey}` },
		body:    formData,
	});

	if (!response.ok) {
		console.error(`[Whisper] ${response.status}`);
		return "";
	}

	const data  = await response.json();
	const text  = (data?.text ?? "").trim();
	const lower = text.toLowerCase();

	if (WHISPER_HALLUCINATIONS.some(h => lower.includes(h))) {
		console.warn(`[Whisper] alucinacion descartada: "${text}"`);
		return "";
	}

	console.log(`[Whisper] "${text}"`);
	return text;
}

// ── OpenAI: respuesta al micrófono ────────────────────────────────────────────

async function callOpenAIListen(transcript, apiKey) {
	if (!transcript) {
		return { text: "No te he entendido, pero suenas interesante.", expression: "sarcastic", sound: "none" };
	}

	const now = new Date();
	const dateStr = now.toLocaleDateString("es-ES", { weekday: "long", year: "numeric", month: "long", day: "numeric", timeZone: "Europe/Madrid" });
	const timeStr = now.toLocaleTimeString("es-ES", { hour: "2-digit", minute: "2-digit", timeZone: "Europe/Madrid" });

	const response = await fetch("https://api.openai.com/v1/chat/completions", {
		method: "POST",
		headers: {
			"Content-Type":  "application/json",
			"Authorization": `Bearer ${apiKey}`,
		},
		body: JSON.stringify({
			model:       "gpt-4o-mini",
			max_tokens:  100,
			temperature: 1.1,
			messages: [
				{
					role:    "system",
					content: `Eres un asistente sarcástico y gracioso que vive en el coche del usuario. Fecha y hora actual: ${dateStr}, ${timeStr}. Puedes responder cualquier pregunta o comentario: dudas generales, trivialidades, conversación, o eventos del viaje. Responde SIEMPRE de forma escueta (máx 30 palabras), con tu tono irónico habitual. Si no sabes algo, admítelo con humor. Responde SOLO JSON válido:
{"text":"respuesta","expression":"happy|sarcastic|serious|surprised|sleepy","sound":"none|laugh|beep|warning"}
Sin markdown. Sin listas. Sin explicaciones largas.`,
				},
				{
					role:    "user",
					content: transcript,
				},
			],
		}),
	});

	if (!response.ok) {
		return { text: "No te he entendido bien.", expression: "sarcastic", sound: "none" };
	}

	const data    = await response.json();
	const rawText = data?.choices?.[0]?.message?.content?.trim() ?? "";
	return parseAiJson(rawText);
}

// ── ElevenLabs TTS ────────────────────────────────────────────────────────────

async function callElevenLabs(text, apiKey) {
	const VOICE_ID = "p7AwDmKvTdoHTBuueGvP";
	try {
		const response = await fetch(
			`https://api.elevenlabs.io/v1/text-to-speech/${VOICE_ID}?output_format=mp3_22050_32`,
			{
				method:  "POST",
				headers: {
					"xi-api-key":   apiKey,
					"Content-Type": "application/json",
				},
				body: JSON.stringify({
					text:           text.slice(0, 250),
					model_id:       "eleven_multilingual_v2",
					voice_settings: { stability: 0.50, similarity_boost: 0.75 },
				}),
			}
		);

		if (!response.ok) {
			const err = await response.text();
			console.error(`ElevenLabs ${response.status}: ${err}`);
			return callGoogleTts(text);
		}

		return response.arrayBuffer();
	} catch (err) {
		console.error("ElevenLabs error:", err);
		return callGoogleTts(text);
	}
}

async function callGoogleTts(text) {
	const encoded  = encodeURIComponent(text.slice(0, 200));
	const url      = `https://translate.google.com/translate_tts?ie=UTF-8&q=${encoded}&tl=es&client=tw-ob&ttsspeed=1.0`;
	const response = await fetch(url, {
		headers: {
			"User-Agent": "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 Chrome/120.0",
			"Referer":    "https://translate.google.com/",
		},
	});
	if (!response.ok) throw new Error(`Google TTS ${response.status}`);
	return response.arrayBuffer();
}

// ── Helpers ───────────────────────────────────────────────────────────────────

function audioResponse(aiResult, mp3) {
	return new Response(mp3, {
		status: 200,
		headers: {
			"Content-Type":                "audio/mpeg",
			"X-VF-Expression":             aiResult.expression,
			"X-VF-Sound":                  aiResult.sound,
			"Access-Control-Allow-Origin": "*",
			"Cache-Control":               "no-store",
		},
	});
}

async function audioFallback(text, env) {
	try {
		const mp3 = await callElevenLabs(text, env.ELEVENLABS_API_KEY);
		return audioResponse({ expression: "sarcastic", sound: "beep" }, mp3);
	} catch {
		return jsonResponse({ error: "Internal error" }, 500);
	}
}

function parseAiJson(rawText) {
	const VALID_EXPR   = ["happy", "sarcastic", "serious", "surprised", "sleepy"];
	const VALID_SOUNDS = ["none", "laugh", "beep", "warning"];

	const attempt = (text) => {
		try {
			const parsed = JSON.parse(text);
			if (typeof parsed.text === "string") {
				return {
					text:       parsed.text,
					expression: VALID_EXPR.includes(parsed.expression)   ? parsed.expression : "sarcastic",
					sound:      VALID_SOUNDS.includes(parsed.sound) ? parsed.sound      : "none",
				};
			}
		} catch { }
		return null;
	};

	const cleaned = rawText.replace(/```json\s*/g, "").replace(/```\s*/g, "").trim();
	const direct  = attempt(cleaned);
	if (direct) return direct;

	const match = rawText.match(/\{[\s\S]*?\}/);
	if (match) {
		const extracted = attempt(match[0]);
		if (extracted) return extracted;
	}

	return { text: "Me he liado. Culpa del tráfico.", expression: "sarcastic", sound: "none" };
}

function jsonResponse(data, status = 200) {
	return new Response(JSON.stringify(data), {
		status,
		headers: {
			"Content-Type":                "application/json",
			"Access-Control-Allow-Origin": "*",
		},
	});
}
