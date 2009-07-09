"""pynaim weather bot (responds to !pyweather <zipcode>).

To test from outside naim, just run:
  python weather.py
To run from within naim:
  /modload path/to/pynaim
  /pyload path/to/weather.py
"""

__author__ = 'Andrew Chin (eminence)'

import httplib
import re

try:
  import naim
  ### THESE ARE QUICK HACKS
  FakeNaim = None
except ImportError:
  print 'Not running from within naim!  Using FakeNaim'

  class FakeNaim(object):
    class FakeHooks(object):
      def add(self, name, weight, func):
        print 'Adding fake hook: %s %i %r' % (name, weight, func)

    def __init__(self):
      self.hooks = self.FakeHooks()

    def echo(self, s, *args):
      if args:
        s %= args
      else:
        s = '%s' % (s,)

      print s

    def eval(self, s):
      print s

  naim = FakeNaim()


PYWEATHER_RE = re.compile('^!pyweather\s+(\d{5})$')
YCONDITION_RE = re.compile('<yweather:condition.*?text="(.*?)".*?temp="(\d+)".*?/>')


def WeatherRecvFrom(conn, src, dst, message, flags):
  naim.echo('Message from %s: %s', src, message)

  if not dst or dst[0] != '#':
    return

  m = PYWEATHER_RE.match(message)
  if not m:
    return

  zip = m.group(1)
  conn = httplib.HTTPConnection('weather.yahooapis.com')
  conn.request('GET', '/forecastrss?p=%s' % zip)
  r = conn.getresponse()
  if r.status != 200:
    naim.eval('msg %s %s' % (dst, 'Failed to open weather URL!'))
    return

  data = r.read()
  #naim.echo('I have data!: %r', data)
  # TODO(eminence): it would be nice to actually use an XML parser
  # here, instead of attempting to regex out the info we want
  conditionM = YCONDITION_RE.search(data)
  if conditionM:
    naim.eval('/msg %s %s' % (dst, 'It is currently %s and %s' % conditionM.groups([2, 1])))
  else:
    naim.echo('failed to find condition match')


naim.hooks.add('recvfrom', 200, WeatherRecvFrom)

## manual test:
if FakeNaim:
  WeatherRecvFrom('conn','test', '#em32', '!pyweather 02906', 0)
