from vaderSentiment.vaderSentiment import SentimentIntensityAnalyzer

_analyzer = SentimentIntensityAnalyzer()

def analyze_text(text: str) -> dict:
    '''
    Return dict with keys: vibe, score, detail{pos,neg,neu}
    vibe is 'positive' | 'negative' | 'neutral'
    score is confidence of chosen vibe (0..1)
    '''
    s = _analyzer.polarity_scores(text)
    compound = s.get('compound', 0.0)
    # label mapping per VADER guidelines
    if compound >= 0.05:
        label = 'positive'
        conf = max(s.get('pos', 0.0), compound)
    elif compound <= -0.05:
        label = 'negative'
        conf = max(s.get('neg', 0.0), -compound)
    else:
        label = 'neutral'
        conf = max(s.get('neu', 0.0), 1.0 - abs(compound))
    return {
        'vibe': label,
        'score': float(round(conf, 4)),
        'detail': {
            'positive_score': float(round(s.get('pos', 0.0), 4)),
            'negative_score': float(round(s.get('neg', 0.0), 4)),
            'neutral_score': float(round(s.get('neu', 0.0), 4)),
        },
    }
