# Herta's Vibe Checker — Runnable MVP (FastAPI + SQLite)

Implements the endpoints and flows from your spec: OAuth2 Client Credentials + Math Challenge,
`/health`, `/stats`, `/math_challenge`, `/oauth/token`, `/vibe-check/single`, `/vibe-check/batch`, `/vibes`.
Sentiment engine uses VADER (lexicon included in package).

## Quick Start (Python)
```
cd herta-vibe-checker
python -m venv .venv && source .venv/bin/activate   # Windows: .venv\Scripts\activate
pip install -r requirements.txt
export JWT_SECRET="change_me"   # Windows: set JWT_SECRET=change_me
uvicorn app.main:app --reload
```
Open http://127.0.0.1:8000/docs

### Example Flow
```
# 1) Get math challenge
curl -s http://127.0.0.1:8000/math_challenge

# Suppose returns: {"challenge_id":"abcd1234","question":"12+5=?"}
# 2) Request token
curl -s -X POST http://127.0.0.1:8000/oauth/token   -H "Authorization: Basic $(printf 'demo:demo' | base64)"   -H "Content-Type: application/x-www-form-urlencoded"   -d "challenge_id=abcd1234" -d "challenge_answer=17"

# 3) Use token
TOKEN="(paste token)"
curl -s -X POST http://127.0.0.1:8000/vibe-check/single   -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json"   -d '{"text":"I love this stream!"}'
```

## Quick Start (Docker)
```
cd herta-vibe-checker
docker compose up --build
```
Default demo credentials: `client_id=demo`, `client_secret=demo` (seeded at startup).

## Tests
```
pytest -q
```

## Credits / References
- FastAPI — https://fastapi.tiangolo.com/
- SQLAlchemy 2.x — https://docs.sqlalchemy.org/
- VADER Sentiment (Hutto & Gilbert, 2014) — https://ojs.aaai.org/index.php/ICWSM/article/view/14550
- Business & API spec: your uploaded document
