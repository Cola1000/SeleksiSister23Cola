Here’s a **drop-in README** you can paste over your current one. It adds clear run modes (your API only vs. compatibility (compat)), and shows exactly how to “compile/build” and **run the tests**.

---

# Herta's Vibe Checker — Runnable MVP (FastAPI + SQLite)

Implements the endpoints and flows from your spec:

* Core (your API): `/health`, `/stats`, `/math_challenge`, `/oauth/token`, `/vibe-check/single`, `/vibe-check/batch`, `/vibes`
* Kalau Integrasi Temen: `/register`, `/oauth/token` (client\_credentials), `/custom-words`, `/detect`
* Sentiment engine: **VADER** (lexicon bundled)

## Requirements

* Python 3.11+ (or use Docker)
* (Optional) `make`, `curl`, `pytest`

---

## Run Modes

You can switch behavior with one environment flag:

* **Your API only** (math challenge enforced):
  `REQUIRE_MATH_CHALLENGE=true`
* **compat mode** (pure client\_credentials; math challenge optional):
  `REQUIRE_MATH_CHALLENGE=false`

> If you don’t set it, default is compat OFF (`false` -> other API flow allowed).
> All endpoints exist in both modes; this flag only changes the token issuance rules.

---

## Quick Start (Python)

```bash
cd ThisShiTagain
python -m venv .venv && source .venv/bin/activate   # Windows: .venv\Scripts\activate
pip install -r requirements.txt

# Choose ONE mode:

# 1) Run YOUR API ONLY (math challenge required)
export JWT_SECRET="change_me"
export REQUIRE_MATH_CHALLENGE=true
uvicorn app.main:app --host 0.0.0.0 --port 8000 --proxy-headers

# 2) Run COMPAT MODE (client_credentials)
# export JWT_SECRET="change_me"
# export REQUIRE_MATH_CHALLENGE=false
# uvicorn app.main:app --host 0.0.0.0 --port 8000 --proxy-headers
```

Open: [http://127.0.0.1:8000/docs](http://127.0.0.1:8000/docs)

### Bare API Example Flow

```bash
# 1) Get math challenge
curl -s http://127.0.0.1:8000/math_challenge

# Suppose: {"challenge_id":"abcd1234","question":"12+5=?"}
# 2) Request token (Basic = base64(client_id:client_secret); demo:demo is pre-seeded)
curl -s -X POST http://127.0.0.1:8000/oauth/token \
  -H "Authorization: Basic $(printf 'demo:demo' | base64)" \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "challenge_id=abcd1234" -d "challenge_answer=17"

# 3) Use token
TOKEN="(paste access_token)"
curl -s -X POST http://127.0.0.1:8000/vibe-check/single \
  -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" \
  -d '{"text":"I love this stream!"}'
```

### Testing Compatibility | Example Flow

```bash
# Start with REQUIRE_MATH_CHALLENGE=false

# Register new client
curl -s -X POST http://127.0.0.1:8000/register \
  -H 'Content-Type: application/json' \
  -d '{"name":"MyApp","email":"dev@example.com"}'

# Token via client_credentials (no challenge)
CID="(client_id)"; CSEC="(client_secret)"; BASIC=$(printf "%s:%s" "$CID" "$CSEC" | base64)
curl -s -X POST http://127.0.0.1:8000/oauth/token \
  -H "Authorization: Basic $BASIC" \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "grant_type=client_credentials"
```

---

## Quick Start (Docker)

```bash
cd ThisShiTagain
# YOUR API ONLY
docker compose up --build \
  -e JWT_SECRET=change_me \
  -e REQUIRE_MATH_CHALLENGE=true

# compat mode (uncomment to use)
# docker compose up --build \
#   -e JWT_SECRET=change_me \
#   -e REQUIRE_MATH_CHALLENGE=false
```

API: [http://127.0.0.1:8000](http://127.0.0.1:8000)

---

## Running Tests

### Run the default suite

```bash
pytest -q
```

### Add & run the compat integration test (register -> token -> custom-words -> detect -> original vibe)

Create `tests/test_api_{WHOMST}.py`:

```python
import os, base64
from fastapi.testclient import TestClient

# Use a separate DB for tests
os.environ.setdefault("DATABASE_URL", "sqlite:///./test_niggas_api.db")
# Flip to "true" to re-run with math challenge enforced (token step will then fail by design)
os.environ.setdefault("REQUIRE_MATH_CHALLENGE", "false")

from app.main import app
c = TestClient(app)

def b64(cid, csec): return base64.b64encode(f"{cid}:{csec}".encode()).decode()

def test_flow_register_token_customwords_detect_and_original_vibe():
    r = c.post("/register", json={"name":"CompatTest","email":"dev@example.com"})
    assert r.status_code == 200
    cid, csec = r.json()["client_id"], r.json()["client_secret"]

    r = c.post("/oauth/token",
               headers={"Authorization": f"Basic {b64(cid,csec)}"},
               data={"grant_type":"client_credentials"})
    assert r.status_code == 200
    tok = r.json()["access_token"]
    bear = {"Authorization": f"Bearer {tok}"}

    # add blacklist
    r = c.post("/custom-words", headers=bear,
               json={"action":"add","category":"blacklist","words":["badword","heck"]})
    assert r.status_code == 200

    # detect hits
    r = c.post("/detect", headers=bear, json={"text":"this is a badword indeed"})
    assert r.status_code == 200 and r.json()["isProfane"] is True

    # whitelist override
    c.post("/custom-words", headers=bear,
           json={"action":"add","category":"whitelist","words":["badword"]}).raise_for_status()
    r = c.post("/detect", headers=bear, json={"text":"badword should now be fine"})
    assert r.status_code == 200 and r.json()["isProfane"] is False

    # original endpoint still works
    r = c.post("/vibe-check/single", headers=bear, json={"text":"I absolutely love this!"})
    assert r.status_code == 200
```

Run it:

```bash
pytest -q tests/test_api_{WHOMST}.py
```

---

## “Run my API solely” (no other API flow)

If you want to **enforce your original token flow** and treat compat as off:

```bash
export REQUIRE_MATH_CHALLENGE=true
uvicorn app.main:app --host 0.0.0.0 --port 8000 --proxy-headers
# Now tokens require /math_challenge + correct answer; other API's style /oauth/token without challenge will fail.
```

---

## Environment Summary

| Variable                 | Purpose                                   | Typical values                                   |
| ------------------------ | ----------------------------------------- | ------------------------------------------------ |
| `JWT_SECRET`             | Sign JWT access tokens                    | `change_me`                                      |
| `DATABASE_URL`           | SQLAlchemy URL                            | `sqlite:///./data.db`                            |
| `REQUIRE_MATH_CHALLENGE` | Toggle auth style                         | `true` (your API only) / `false` (compat) |
| `ROOT_PATH`              | If served under a sub-path behind a proxy | `/vibe` or empty                                 |

---

## Credits / References

* FastAPI — [https://fastapi.tiangolo.com/](https://fastapi.tiangolo.com/)
* SQLAlchemy 2.x — [https://docs.sqlalchemy.org/](https://docs.sqlalchemy.org/)
* VADER Sentiment — Hutto & Gilbert (2014) — [https://ojs.aaai.org/index.php/ICWSM/article/view/14550](https://ojs.aaai.org/index.php/ICWSM/article/view/14550)
