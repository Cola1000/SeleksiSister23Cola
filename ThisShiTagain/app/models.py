from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy import String, Integer, Float, DateTime, ForeignKey, Text, func, UniqueConstraint
from .db import Base

class ClientApp(Base):
    __tablename__ = "client_apps"
    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    client_id: Mapped[str] = mapped_column(String(128), unique=True, index=True)
    client_secret_hash: Mapped[str] = mapped_column(String(256))
    app_name: Mapped[str | None] = mapped_column(String(128), nullable=True)
    contact_email: Mapped[str | None] = mapped_column(String(128), nullable=True)
    results: Mapped[list["VibeResult"]] = relationship("VibeResult", back_populates="client", cascade="all, delete-orphan")

class VibeResult(Base):
    __tablename__ = "vibe_results"
    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    client_id: Mapped[str] = mapped_column(String(128), ForeignKey("client_apps.client_id", ondelete="CASCADE"), index=True)
    text: Mapped[str] = mapped_column(Text)
    vibe_label: Mapped[str] = mapped_column(String(16))
    score: Mapped[float] = mapped_column(Float)
    created_at: Mapped["DateTime"] = mapped_column(DateTime(timezone=True), server_default=func.now())
    client: Mapped["ClientApp"] = relationship("ClientApp", back_populates="results")

class MathChallenge(Base):
    __tablename__ = "math_challenges"
    challenge_id: Mapped[str] = mapped_column(String(64), primary_key=True)
    question: Mapped[str] = mapped_column(String(64))
    answer: Mapped[int] = mapped_column(Integer)
    expires_at: Mapped["DateTime"] = mapped_column(DateTime(timezone=True), index=True)



class CustomWord(Base):
    __tablename__ = "custom_words"
    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    client_id: Mapped[str] = mapped_column(String(128), ForeignKey("client_apps.client_id", ondelete="CASCADE"), index=True)
    word: Mapped[str] = mapped_column(String(128))
    category: Mapped[str] = mapped_column(String(16))  # 'blacklist' | 'whitelist'
    __table_args__ = (UniqueConstraint("client_id", "word", "category", name="uq_customword_client_word_cat"),)
