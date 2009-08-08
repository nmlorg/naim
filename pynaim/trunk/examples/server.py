"""A more interesting threading example.
Runs a webserver, providing a very simple interface to naim

To run, pyload this file, and then browse to http://localhost:8081/

Please note that it will only server two requests before exiting
"""

__author__ = 'eminence (Andrew Chin)'

#import naimtest
import httplib
import naim
from cgi import parse_qsl
import threading


from BaseHTTPServer import BaseHTTPRequestHandler
from BaseHTTPServer import HTTPServer


class naimHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        if self.path == "/":
            naim.echo("got a GET request for %s; trying to send reponse", self.path)
            self.send_response(200, '')
            self.send_header("content-type","text/html")
            self.end_headers()
            self.wfile.write("<br/><form method=\"POST\" action=\"/submit\">")
            self.wfile.write("<select name=\"conn\">")
            for conn in naim.connections:
                self.wfile.write("<option value=\"%s\">%s</option>" % (conn,conn))
            self.wfile.write("</select>")
            self.wfile.write("<input type=\"text\" name=\"dst\" value=\"#naim\" />")
            self.wfile.write("<input type=\"text\" name=\"msg\" value=\"hello, kitties!\" />")
            self.wfile.write("<input type=\"submit\" value=\"send!\" />")
            self.wfile.write("</form>")

        else:
            naim.echo("got a GET request, but i don't handles those %s -->%s<--", self.client_address, self.path)
            self.send_error(404)
    def do_POST(self):
        if self.path == "/submit":
            self.send_response(200,'')
            self.send_header("content-type","text/html")
            self.end_headers()
            content_len = int(self.headers.get("Content-Length"))
            raw_data = self.rfile.read(content_len)

            data = {}
            for arg in parse_qsl(raw_data):
                data[arg[0]] = arg[1]

            naim.connections[data['conn']].commands.msg(data['dst'],data['msg'])
            self.wfile.write("<a href=\"/\">ok.  go back</a>")



def run(server_class=HTTPServer, handler_class=BaseHTTPRequestHandler):
    server_address = ('127.0.0.1', 8081)
    httpd = server_class(server_address, handler_class)
    naim.echo("waiting for request") 
    ## handle only two requests at a time:
    for x in range(2):
        httpd.handle_request() # NOTE this will only only serve one request


def go():
#naim.hooks.add('recvfrom', 200, WeatherRecvFrom)
    naim.echo("Starting up new thread")
    run(handler_class=naimHandler)
    naim.echo("Finishing thread")

t = threading.Thread(target=go)
t.start()
naim.echo("started up new thread")
