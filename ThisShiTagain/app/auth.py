import os, base64, uuid, random
from datetime import datetime, timedelta, timezone
from typing import Optional

import jwt
from passlib.hash import bcrypt
from fastapi import Depends, HTTPException, status, Header, Form
from sqlalchemy.orm import Session

from .db import get_db
from .models import ClientApp, MathChallenge

JWT_SECRET = os.getenv("JWT_SECRET", "dev_super_secret_change_me")
JWT_ALG = "HS256"
TOKEN_TTL_SECS = 3600

def _now() -> datetime:
    return datetime.now(timezone.utc)

def create_math_challenge(db: Session) -> dict:
    a, b = random.randint(6, 50), random.randint(1, 25)
    question = f"{a}+{b}=?"
    answer = a + b
    challenge_id = uuid.uuid4().hex[:16]
    mc = MathChallenge(challenge_id=challenge_id, question=question, answer=answer, expires_at=_now() + timedelta(minutes=5))
    db.add(mc)
    db.commit()
    return {"challenge_id": challenge_id, "question": question}

def verify_basic_auth(authorization: Optional[str]) -> tuple[str, str]:
    if not authorization or not authorization.startswith("Basic "):
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Missing Basic auth")
    try:
        decoded = base64.b64decode(authorization.split(" ", 1)[1]).decode()
        cid, csec = decoded.split(":", 1)
        return cid, csec
    except Exception:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Invalid Basic header")

def issue_token(db: Session, client_id: str, client_secret: str, challenge_id: str, challenge_answer: int) -> dict:
    app = db.query(ClientApp).filter_by(client_id=client_id).first()
    if not app or not bcrypt.verify(client_secret, app.client_secret_hash):
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Invalid client credentials")

    mc = db.query(MathChallenge).filter_by(challenge_id=challenge_id).first()
    if not mc or mc.expires_at < _now() or mc.answer != challenge_answer:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Invalid or expired challenge")

    db.delete(mc)
    db.commit()

    payload = {"cid": client_id, "exp": _now() + timedelta(seconds=TOKEN_TTL_SECS)}
    token = jwt.encode(payload, JWT_SECRET, algorithm=JWT_ALG)
    return {"access_token": token, "token_type": "Bearer", "expires_in": TOKEN_TTL_SECS, "scope": "vibe.read vibe.write"}

def get_current_client(authorization: Optional[str] = Header(None), db: Session = Depends(get_db)) -> ClientApp:
    if not authorization or not authorization.startswith("Bearer "):
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Missing Bearer token")
    token = authorization.split(" ", 1)[1]
    try:
        payload = jwt.decode(token, JWT_SECRET, algorithms=[JWT_ALG])
        cid = payload.get("cid")
    except jwt.PyJWTError:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Invalid token")
    app = db.query(ClientApp).filter_by(client_id=cid).first()
    if not app:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Client not found")
    return app

def ensure_demo_client(db: Session):
    cid = os.getenv("DEMO_CLIENT_ID", "demo")
    csec = os.getenv("DEMO_CLIENT_SECRET", "demo")
    existing = db.query(ClientApp).filter_by(client_id=cid).first()
    if not existing:
        hashed = bcrypt.hash(csec)
        app = ClientApp(client_id=cid, client_secret_hash=hashed, app_name="Demo App", contact_email=None)
        db.add(app)
        db.commit()
