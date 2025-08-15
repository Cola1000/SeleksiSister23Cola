import os, re
from datetime import datetime, timezone
from typing import Optional

from fastapi import FastAPI, Depends, HTTPException, status, Header, Form, Request, Body
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.orm import Session

from .db import init_db, get_db
from .models import VibeResult, CustomWord
from .schemas import (
    HealthOut, StatsOut, MathChallengeOut, TokenOut,
    SingleIn, SingleOut, BatchIn, BatchOut, BatchItemOut,
    HistoryOut, HistoryItem, DetailScores, 
    RegisterIn, RegisterOut, DetectIn, DetectOut, CustomWordsIn, CustomWordsOut
)
from .auth import create_math_challenge, verify_basic_auth, issue_token, get_current_client, ensure_demo_client, create_client
from .nlp_engine import analyze_text

app = FastAPI(title="Herta's Vibe Checker", version="1.0.0") 
app = FastAPI(title="Herta's Vibe Checker", version="1.0.0", root_path=os.getenv("ROOT_PATH",""))  #Change this to change root_path

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.on_event("startup")
def on_startup():
    init_db()
    from .db import SessionLocal
    with SessionLocal() as db:
        ensure_demo_client(db)

@app.get("/health", response_model=HealthOut)
def health():
    return HealthOut()

@app.get("/stats", response_model=StatsOut)
def stats(db: Session = Depends(get_db)):
    total = db.query(VibeResult).count()
    if total == 0:
        return StatsOut(
            total_texts_analyzed=0,
            positive_percentage=0.0,
            neutral_percentage=0.0,
            negative_percentage=0.0,
            avg_response_time_ms=0,
            uptime="n/a",
        )
    pos = db.query(VibeResult).filter_by(vibe_label="positive").count()
    neu = db.query(VibeResult).filter_by(vibe_label="neutral").count()
    neg = db.query(VibeResult).filter_by(vibe_label="negative").count()
    return StatsOut(
        total_texts_analyzed=total,
        positive_percentage=round(pos/total, 4),
        neutral_percentage=round(neu/total, 4),
        negative_percentage=round(neg/total, 4),
        avg_response_time_ms=0,
        uptime="n/a",
    )

@app.get("/math_challenge", response_model=MathChallengeOut)
def get_math_challenge(db: Session = Depends(get_db)):
    return create_math_challenge(db)

def oauth_token_compatible(
    grant_type: str = Form("client_credentials"),
    authorization: Optional[str] = Header(None),
    challenge_id: Optional[str] = Form(None),
    challenge_answer: Optional[int] = Form(None),
    db: Session = Depends(get_db),
):
    if grant_type != "client_credentials":
        raise HTTPException(status_code=400, detail="unsupported grant_type")
    client_id, client_secret = verify_basic_auth(authorization)
    result = issue_token(db, client_id, client_secret, challenge_id, challenge_answer)

    return result

@app.post("/vibe-check/single", response_model=SingleOut)
def vibe_single(
    payload: SingleIn,
    db: Session = Depends(get_db),
    client = Depends(get_current_client),
):
    res = analyze_text(payload.text)
    vr = VibeResult(
        client_id=client.client_id,
        text=payload.text,
        vibe_label=res["vibe"],
        score=res["score"],
    )
    db.add(vr)
    db.commit()
    db.refresh(vr)
    return SingleOut(
        id=f"res_{vr.id:010d}",
        text=payload.text,
        vibe=res["vibe"],
        score=res["score"],
        detail=DetailScores(**res["detail"]),
    )

@app.post("/vibe-check/batch", response_model=BatchOut)
def vibe_batch(
    payload: BatchIn,
    db: Session = Depends(get_db),
    client = Depends(get_current_client),
):
    if not isinstance(payload.texts, list) or len(payload.texts) == 0:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Field 'texts' must be a non-empty list")
    if len(payload.texts) > 100:
        raise HTTPException(status_code=status.HTTP_413_REQUEST_ENTITY_TOO_LARGE, detail="Too many texts (max 100)")
    results = []
    for t in payload.texts:
        analysis = analyze_text(t)
        vr = VibeResult(
            client_id=client.client_id,
            text=t,
            vibe_label=analysis["vibe"],
            score=analysis["score"],
        )
        db.add(vr)
        db.flush()
        results.append(BatchItemOut(
            id=f"res_{vr.id:010d}",
            text=t,
            vibe=analysis["vibe"],
            score=analysis["score"],
        ))
    db.commit()
    return BatchOut(results=results)

@app.get("/vibes", response_model=HistoryOut)
def list_vibes(
    limit: int = 50,
    offset: int = 0,
    db: Session = Depends(get_db),
    client = Depends(get_current_client),
):
    q = db.query(VibeResult).filter_by(client_id=client.client_id).order_by(VibeResult.id.desc()).offset(offset).limit(limit)
    items = []
    for vr in q:
        ts = vr.created_at
        if ts and ts.tzinfo is None:
            from datetime import timezone
            ts = ts.replace(tzinfo=timezone.utc)
        iso = ts.isoformat() if ts else datetime.now(timezone.utc).isoformat()
        items.append(HistoryItem(
            id=f"res_{vr.id:010d}",
            text=vr.text,
            vibe=vr.vibe_label,
            score=vr.score,
            timestamp=iso,
        ))
    return HistoryOut(history=items)

# ============== This is so ass ================

@app.post("/register", response_model=RegisterOut)
def register(payload: RegisterIn, db: Session = Depends(get_db)):
    cid, csec = create_client(db, name=payload.name, email=payload.email, uri=payload.uri)
    return RegisterOut(client_id=cid, client_secret=csec)

@app.post("/custom-words", response_model=CustomWordsOut)
def custom_words(
    payload: CustomWordsIn,
    db: Session = Depends(get_db),
    client = Depends(get_current_client),
):
    action = payload.action.lower()
    category = payload.category.lower()
    if action not in {"add", "remove"} or category not in {"blacklist", "whitelist"}:
        raise HTTPException(status_code=400, detail="Invalid action or category")

    words = {w.strip().lower() for w in payload.words if w.strip()}
    if not words:
        raise HTTPException(status_code=400, detail="No words provided")

    if action == "add":
        for w in words:
            cw = CustomWord(client_id=client.client_id, word=w, category=category)
            # avoid duplicates thanks to unique constraint
            try:
                db.add(cw)
                db.flush()
            except Exception:
                db.rollback()
        db.commit()
        return CustomWordsOut(success=True, message=f"Words successfully added to {category}")
    else:  # remove
        (db.query(CustomWord)
           .filter(CustomWord.client_id == client.client_id,
                   CustomWord.category == category,
                   CustomWord.word.in_(list(words)))
           .delete(synchronize_session=False))
        db.commit()
        return CustomWordsOut(success=True, message=f"Words successfully removed from {category}")

def _tokenize(text: str) -> list[str]:
    # simple word tokenizer (unicode letters, numbers); tune as needed
    return [t.lower() for t in re.findall(r"[0-9A-Za-z_]+", text, flags=re.UNICODE)]

@app.post("/detect", response_model=DetectOut)
def detect(payload: DetectIn, db: Session = Depends(get_db), client = Depends(get_current_client)):
    tokens = set(_tokenize(payload.text))
    # fetch per-client lists
    rows = db.query(CustomWord).filter(CustomWord.client_id == client.client_id).all()
    bl = {r.word for r in rows if r.category == "blacklist"}
    wl = {r.word for r in rows if r.category == "whitelist"}
    hits = sorted((tokens & bl) - wl)
    if hits:
        return DetectOut(isProfane=True, detected_words=hits)
    else:
        return DetectOut(isProfane=False, message="No sensitive words detected")
