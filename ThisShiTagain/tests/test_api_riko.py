import os
import base64
import json
from fastapi.testclient import TestClient

# Use a separate DB so tests don't touch your dev data
os.environ.setdefault("DATABASE_URL", "sqlite:///./test_niggas_api.db")
# Force friend-spec token flow by default; flip to "true" to re-test your math-challenge flow
os.environ.setdefault("REQUIRE_MATH_CHALLENGE", "false")
# If your app uses ROOT_PATH behind a reverse proxy, set it here; otherwise leave empty.
os.environ.setdefault("ROOT_PATH", "")

from app.main import app  # noqa: E402  (import after env setup)
client = TestClient(app)

def _basic(cid: str, csec: str) -> str:
    return base64.b64encode(f"{cid}:{csec}".encode()).decode()

def test_flow_register_token_customwords_detect_and_original_vibe():
    # 1) Register → get client_id + client_secret (friend's /register)
    r = client.post("/register", json={
        "name": "CompatTestApp",
        "email": "dev@example.com",
        "uri": "https://example.com"
    })
    assert r.status_code == 200, r.text
    reg = r.json()
    assert "client_id" in reg and "client_secret" in reg, reg
    cid, csec = reg["client_id"], reg["client_secret"]

    # 2) Token via client_credentials (friend's /oauth/token)
    hdr = {"Authorization": f"Basic {_basic(cid, csec)}"}
    r = client.post("/oauth/token",
                    headers=hdr,
                    data={"grant_type": "client_credentials"})
    assert r.status_code == 200, r.text
    tok = r.json()
    assert "access_token" in tok and tok["token_type"] == "Bearer", tok
    bearer = {"Authorization": f"Bearer {tok['access_token']}"}

    # 3) Add blacklist words
    r = client.post("/custom-words", headers=bearer, json={
        "action": "add",
        "category": "blacklist",
        "words": ["badword", "heck"]
    })
    assert r.status_code == 200, r.text
    assert r.json()["success"] is True

    # 4) Detect should hit "badword"
    r = client.post("/detect", headers=bearer, json={"text": "this is a badword indeed"})
    assert r.status_code == 200, r.text
    out = r.json()
    assert out["isProfane"] is True
    assert "badword" in out.get("detected_words", [])

    # 5) Whitelist overrides blacklist
    r = client.post("/custom-words", headers=bearer, json={
        "action": "add",
        "category": "whitelist",
        "words": ["badword"]
    })
    assert r.status_code == 200, r.text

    r = client.post("/detect", headers=bearer, json={"text": "badword should now be ok"})
    assert r.status_code == 200, r.text
    out = r.json()
    assert out["isProfane"] is False
    assert out.get("detected_words") in (None, [])

    # 6) Remove "heck" from blacklist and confirm it disappears
    r = client.post("/custom-words", headers=bearer, json={
        "action": "remove",
        "category": "blacklist",
        "words": ["heck"]
    })
    assert r.status_code == 200, r.text

    r = client.post("/detect", headers=bearer, json={"text": "heck"})
    assert r.status_code == 200, r.text
    out = r.json()
    # it might still be a hit if also blacklisted elsewhere; but our test removes it:
    assert out["isProfane"] is False

    # 7) Original endpoint still works with same token
    r = client.post("/vibe-check/single", headers=bearer, json={"text": "I absolutely love this!"})
    assert r.status_code == 200, r.text
    data = r.json()
    assert data["vibe"] in {"positive", "neutral", "negative"}
