import os, base64
from fastapi.testclient import TestClient

os.environ.setdefault("DATABASE_URL", "sqlite:///./test_niggas_api.db")
os.environ.setdefault("REQUIRE_MATH_CHALLENGE", "false")
from app.main import app  # import after env set

c = TestClient(app)

def b64(cid, csec): return base64.b64encode(f"{cid}:{csec}".encode()).decode()

def main():
    print("== register")
    r = c.post("/register", json={"name":"CompatScript","email":"dev@example.com","uri":"https://example.com"})
    r.raise_for_status()
    reg = r.json(); cid, csec = reg["client_id"], reg["client_secret"]

    print("== token")
    r = c.post("/oauth/token",
               headers={"Authorization": f"Basic {b64(cid,csec)}"},
               data={"grant_type":"client_credentials"})
    r.raise_for_status()
    tok = r.json(); bearer = {"Authorization": f"Bearer {tok['access_token']}"}

    print("== add blacklist [badword, heck]")
    r = c.post("/custom-words", headers=bearer, json={"action":"add","category":"blacklist","words":["badword","heck"]})
    r.raise_for_status(); print(r.json())

    print("== detect badword")
    r = c.post("/detect", headers=bearer, json={"text":"this is a badword indeed"})
    r.raise_for_status(); print(r.json())

    print("== whitelist badword, detect again")
    c.post("/custom-words", headers=bearer, json={"action":"add","category":"whitelist","words":["badword"]}).raise_for_status()
    r = c.post("/detect", headers=bearer, json={"text":"badword should now be ok"})
    r.raise_for_status(); print(r.json())

    print("== original vibe-check/single")
    r = c.post("/vibe-check/single", headers=bearer, json={"text":"I absolutely love this!"})
    r.raise_for_status(); print(r.json())

    print("All good ✔")

if __name__ == "__main__":
    main()
