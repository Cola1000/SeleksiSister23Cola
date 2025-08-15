import base64
from fastapi.testclient import TestClient
from app.main import app

client = TestClient(app)

def get_token():
    r = client.get("/math_challenge")
    assert r.status_code == 200
    ch = r.json()
    q = ch["question"]
    a, b = q.split("+")[0], q.split("+")[1].split("=?")[0]
    answer = int(a) + int(b)
    basic = base64.b64encode(b"demo:demo").decode()
    r2 = client.post(
        "/oauth/token",
        headers={"Authorization": f"Basic {basic}"},
        data={"challenge_id": ch["challenge_id"], "challenge_answer": answer},
    )
    assert r2.status_code == 200, r2.text
    return r2.json()["access_token"]

def test_health():
    r = client.get("/health")
    assert r.status_code == 200
    assert r.json()["status"] == "OK"

def test_vibe_flow():
    token = get_token()
    r = client.post(
        "/vibe-check/single",
        headers={"Authorization": f"Bearer {token}"},
        json={"text": "I absolutely love this!"},
    )
    assert r.status_code == 200, r.text
    data = r.json()
    assert data["vibe"] in {"positive", "neutral", "negative"}

    r2 = client.post(
        "/vibe-check/batch",
        headers={"Authorization": f"Bearer {token}"},
        json={"texts": ["This is great", "This is awful"]},
    )
    assert r2.status_code == 200, r2.text
    out = r2.json()
    assert len(out["results"]) == 2

    r3 = client.get("/vibes?limit=5", headers={"Authorization": f"Bearer {token}"})
    assert r3.status_code == 200, r3.text
    hist = r3.json()["history"]
    assert len(hist) >= 3
