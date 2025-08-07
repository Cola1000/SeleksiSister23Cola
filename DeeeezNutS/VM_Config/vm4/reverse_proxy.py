#!/usr/bin/env python3
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.request import urlopen, Request
from urllib.error import URLError, HTTPError

TARGET = 'http://10.0.0.20:8080'

class ProxyHandler(BaseHTTPRequestHandler):
	def do_GET(self):
		try:
			upstream = urlopen(Request(TARGET + self.path))
			self.send_response(upstream.status)
			for k, v in upstream.getheaders():
				if k.lower() in ('transfer-encoding', 'connection', 'content-encoding'):
					continue
				self.send_header(k,v)
			self.end_headers()
			self.wfile.write(upstream.read())
		except HTTPError as e:
			self.send_error(e.code, e.reason)
		except URLError as e:
			self.send_error(502, str(e))

	def log_message(self, fmt, *args):
		pass

if __name__ == '__main__':
	server = HTTPServer(('',80), ProxyHandler)
	print('Reverse proxy running on port 80 ->', TARGET)
	server.serve_forever()
