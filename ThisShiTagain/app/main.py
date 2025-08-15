import os
from datetime import datetime, timezone
from typing import Optional

from fastapi import FastAPI, Depends, HTTPException, status, Header, Form
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.orm import Session

from .db import init_db, get_db
from .models import VibeResult
from .schemas import (
    HealthOut, StatsOut, MathChallengeOut, TokenOut,
    SingleIn, SingleOut, BatchIn, BatchOut, BatchItemOut,
    HistoryOut, HistoryItem, DetailScores
)
from .auth import create_math_challenge, verify_basic_auth, issue_token, get_current_client, ensure_demo_client
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

@app.post("/oauth/token", response_model=TokenOut)
def oauth_token(
    challenge_id: str = Form(...),
    challenge_answer: int = Form(...),
    authorization: Optional[str] = Header(None),
    db: Session = Depends(get_db),
):
    client_id, client_secret = verify_basic_auth(authorization)
    return issue_token(db, client_id, client_secret, challenge_id, challenge_answer)

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
