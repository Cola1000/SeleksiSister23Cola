import os
from http.server import HTTPServer, SimpleHTTPRequestHandler
os.chdir(os.path.dirname(__file__))
class QuietHandler(SimpleHTTPRequestHandler):
    def log_message(self, *args): pass
if __name__=='__main__':
    HTTPServer(('0.0.0.0',8080), QuietHandler).serve_forever()