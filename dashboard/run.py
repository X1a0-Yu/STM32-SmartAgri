from __future__ import annotations
import argparse, threading, time, webbrowser
from smartagri.flask_app import create_app

def main():
    p=argparse.ArgumentParser(description="SmartAgri V1.1 Flask Control Platform")
    p.add_argument("--port",help="CH340 serial port, e.g. COM6")
    p.add_argument("--baud",type=int,default=19200)
    p.add_argument("--host",default="127.0.0.1")
    p.add_argument("--web-port",type=int,default=8080)
    p.add_argument("--demo",action="store_true")
    p.add_argument("--no-browser",action="store_true")
    p.add_argument("--ingest-token",default="")
    args=p.parse_args()
    app=create_app(port=args.port,baud=args.baud,demo=args.demo or not args.port,ingest_token=args.ingest_token)
    url=f"http://{args.host}:{args.web_port}"
    if not args.no_browser:threading.Thread(target=lambda:(time.sleep(1),webbrowser.open(url)),daemon=True).start()
    print(f"SmartAgri V1.1 Flask: {url}")
    print(f"Mode: {'DEMO' if args.demo or not args.port else args.port+' @ '+str(args.baud)}")
    app.run(host=args.host,port=args.web_port,threaded=True,debug=False,use_reloader=False)
if __name__=="__main__":main()
