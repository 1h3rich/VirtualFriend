import {
	env,
	createExecutionContext,
	waitOnExecutionContext,
} from "cloudflare:test";
import { describe, it, expect } from "vitest";
import worker from "../src";

const VALID_TOKEN = "test-token-123";

function makeEnv(overrides = {}) {
	return { DEVICE_TOKEN: VALID_TOKEN, GEMINI_API_KEY: "fake-key", ELEVENLABS_API_KEY: "fake-el-key", ...overrides };
}

function makeRequest(body = null, token = VALID_TOKEN, method = "POST") {
	return new Request("http://example.com/", {
		method,
		headers: {
			"Content-Type": "application/json",
			...(token ? { "X-Device-Token": token } : {}),
		},
		body: body ? JSON.stringify(body) : null,
	});
}

describe("VirtualFriend Worker", () => {
	it("rechaza GET con 405", async () => {
		const req = makeRequest(null, VALID_TOKEN, "GET");
		const ctx = createExecutionContext();
		const res = await worker.fetch(req, makeEnv(), ctx);
		await waitOnExecutionContext(ctx);
		expect(res.status).toBe(405);
	});

	it("rechaza petición sin token con 401", async () => {
		const req = makeRequest({ message: "hola" }, null);
		const ctx = createExecutionContext();
		const res = await worker.fetch(req, makeEnv(), ctx);
		await waitOnExecutionContext(ctx);
		expect(res.status).toBe(401);
	});

	it("rechaza token incorrecto con 401", async () => {
		const req = makeRequest({ message: "hola" }, "token-malo");
		const ctx = createExecutionContext();
		const res = await worker.fetch(req, makeEnv(), ctx);
		await waitOnExecutionContext(ctx);
		expect(res.status).toBe(401);
	});

	it("rechaza mensaje vacío con 400", async () => {
		const req = makeRequest({ message: "   " });
		const ctx = createExecutionContext();
		const res = await worker.fetch(req, makeEnv(), ctx);
		await waitOnExecutionContext(ctx);
		expect(res.status).toBe(400);
	});

	it("rechaza body sin campo message con 400", async () => {
		const req = makeRequest({ mood: "sarcastico" });
		const ctx = createExecutionContext();
		const res = await worker.fetch(req, makeEnv(), ctx);
		await waitOnExecutionContext(ctx);
		expect(res.status).toBe(400);
	});

	it("la respuesta siempre tiene Content-Type application/json", async () => {
		const req = makeRequest(null, "token-malo", "GET");
		const ctx = createExecutionContext();
		const res = await worker.fetch(req, makeEnv(), ctx);
		await waitOnExecutionContext(ctx);
		expect(res.headers.get("Content-Type")).toBe("application/json");
	});
});
