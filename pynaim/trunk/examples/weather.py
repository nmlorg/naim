## Example pynaim script:  weather bot
##   responds to !pyweather <zipcode> messages in irc channels
##
## to test from outside naim, just run:
##  python weather.py
## to run from within naim:
##  /modload path/to/pynaim
##  /pyload path/to/weather.py

try:
    import naim
    ### THESE ARE QUICK HACKS
    FakeNaim = None
except ImportError:
    print "Not running from within naim!  Using FakeNaim"
    class FakeNaim:
        class FakeHooks:
            def add(self,*args,**kwargs):
                print "Adding fake hook: %s %d %r" % (args[0],args[1], args[2])
        def __init__(self):
            self.hooks = self.FakeHooks()
        def echo(self, message, *args):
            if len(args)>0:
                message = message %(args)
            print message
        def eval(self, *args):
            print ",".join(args)
    naim = FakeNaim()
    
import re
import httplib

def weatherRecvFrom(conn, src, dst, message, flags):
    naim.echo("Message from %s: %s", src, message)
    if dst and dst[:1] == "#":
        r = re.compile("^!pyweather\s+(\d{5})$")
        m = r.match(message)
        if m:
            zip = m.groups(1)
            conn = httplib.HTTPConnection("weather.yahooapis.com")
            conn.request("GET","/forecastrss?p=%s"%(zip))
            r = conn.getresponse()
            if r.status == 200:
                data = r.read()
                #naim.echo("I have data!: %r"%data)
                ### TODO it would be nice to actually use an XML parser here, instead of 
                ###      attempting to regex out the info we want
                conditionRE = re.compile("<yweather:condition.*?text=\"(.*?)\".*?temp=\"(\d+)\".*?/>")
                conditionM = conditionRE.search(data)
                if (conditionM):
                    naim.eval("/msg %s %s" % (dst,"It is currently %s and %s" % (conditionM.group(2), conditionM.group(1))))
                else:
                    naim.echo("failed to find condition match")
            else:
                naim.eval("msg %s %s"%(dst,"Failed to open weather URL!"))
    pass
          

naim.hooks.add('recvfrom', 200, weatherRecvFrom)

## manual test:
if FakeNaim:
    weatherRecvFrom("conn","test","#em32","!pyweather 02906",0)

