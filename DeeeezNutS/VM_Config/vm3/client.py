#!/usr/bin/env python
import argparse, subprocess, sys, urllib.request, urllib.error

INTF="enp0s3"
DNS="10.0.0.10"

def setup_static(ip):
	subprocess.run(['nmcli','connection','delete','internal'], check=False)
	subprocess.run(['nmcli','connection','add','type','ethernet','con-name','internal','ifname',INTF,'ipv4.method','manual','ipv4.addresses', ip, 'ipv4.dns', DNS], check=True)
	subprocess.run(['nmcli','connection','up','internal'], check=True)

def setup_dhcp():
	subprocess.run(['nmcli','connection','delete','internal'], check=False)
	subprocess.run(['nmcli','connection','add','type','ethernet','con-name','internal','ifname',INTF,'ipv4.method','auto'], check=True)
	subprocess.run(['nmcli','connection','up','internal'], check=True)

def fetch():
	url="http://proxy.example.local"
	try:
		with urllib.request.urlopen(url) as r:
			data = r.read().decode()
			print(data)
	except urllib.error.URLError as e:
		print("Fetch error: ", e, file=sys.stderr)

def main():
	p = argparse.ArgumentParser(description="Client to fetch through reverse proxy")
	p.add_argument(
		'--method', choices=['static','dhcp'], required=True,
		help="Network method"
	)
	p.add_argument(
		'--ip', help="10.0.0.30/24 (required for static mode)"
	)
	args = p.parse_args()

	if args.method == "static":
		if not args.ip:
			p.error("--ip is required")
		setup_static(args.ip)
	else:
		setup_dhcp()

	fetch()

if __name__ == '__main__':
	main()
