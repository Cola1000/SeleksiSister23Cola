from pydantic import BaseModel, Field
from typing import List, Optional

class HealthOut(BaseModel):
    status: str = "OK"
    version: str = "1.0.0"

class StatsOut(BaseModel):
    total_texts_analyzed: int
    positive_percentage: float
    neutral_percentage: float
    negative_percentage: float
    avg_response_time_ms: int = 0
    uptime: str = "n/a"

class MathChallengeOut(BaseModel):
    challenge_id: str
    question: str

class TokenOut(BaseModel):
    access_token: str
    token_type: str = "Bearer"
    expires_in: int = 3600
    scope: str = "vibe.read vibe.write"

class SingleIn(BaseModel):
    text: str = Field(..., min_length=1, max_length=500)

class DetailScores(BaseModel):
    positive_score: float
    negative_score: float
    neutral_score: float

class SingleOut(BaseModel):
    id: str
    text: Optional[str] = None
    vibe: str
    score: float
    detail: Optional[DetailScores] = None

class BatchIn(BaseModel):
    texts: List[str]

class BatchItemOut(BaseModel):
    id: str
    text: Optional[str] = None
    vibe: str
    score: float

class BatchOut(BaseModel):
    results: List[BatchItemOut]

class HistoryItem(BaseModel):
    id: str
    text: str
    vibe: str
    score: float
    timestamp: str

class HistoryOut(BaseModel):
    history: List[HistoryItem]









class RegisterIn(BaseModel):
    name: str
    email: Optional[str] = None
    uri: Optional[str] = None

class RegisterOut(BaseModel):
    message: str = "Application registered successfully"
    client_id: str
    client_secret: str

class TokenForm(BaseModel):
    grant_type: str = "client_credentials"

class DetectIn(BaseModel):
    text: str = Field(..., min_length=1)

class DetectOut(BaseModel):
    isProfane: bool
    detected_words: Optional[List[str]] = None
    message: Optional[str] = None

class CustomWordsIn(BaseModel):
    action: str  # "add" | "remove"
    category: str  # "blacklist" | "whitelist"
    words: List[str]

class CustomWordsOut(BaseModel):
    success: bool
    message: str
