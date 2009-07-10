"""pynaim weather bot (responds to !pyweather <zipcode>)."""

__author__ = 'eminence (Andrew Chin)'

import httplib
import re
import xml.parsers.expat
import naim


PYWEATHER_RE = re.compile('^!pyweather\s+(\d{5})$')

class weatherFetcher:
  def __init__(self,src,dst):
    if dst and dst[0] == '#':
      self.target = dst
    ## a privmsg will cause dst to be None
    else:
      self.target = src

  def fetch(self,zip):
    conn = httplib.HTTPConnection('weather.yahooapis.com')
    conn.request('GET', '/forecastrss?p=%s' % zip)
    r = conn.getresponse()
    if r.status != 200:
      naim.eval('msg %s %s' % (self.target, 'Failed to open weather URL!'))
      return

    data = r.read()
    p = xml.parsers.expat.ParserCreate()
    p.StartElementHandler = self.start_element

    p.Parse(data)
    naim.eval("/msg %s %s" % (self.target,
      "Currently in %s: %s temp=%s%s windspeed=%s%s windchill=%s%s humidity=%d%% visibility=%s%s pressure=%s%s" %
      (self.city, self.condition, self.temp, self.tempUnit, self.speed, self.speedUnit,
       self.windchill, self.tempUnit, self.humidity, self.vis, self.distanceUnit, 
       self.pressure, self.pressureUnit)
     ))

  def start_element(self,name, attrs):
    if name == "yweather:condition":
      self.condition    = attrs['text']
      self.temp         = attrs['temp']
    if name == "yweather:location":
      self.city         = attrs['city']
    if name == "yweather:units":
      self.tempUnit     = attrs['temperature']
      self.speedUnit    = attrs['speed']
      self.distanceUnit = attrs['distance']
      self.pressureUnit = attrs['pressure']
    if name == "yweather:wind":
      self.windchill    = int(attrs['chill'])
      self.speed        = int(attrs['speed'])
    if name == "yweather:atmosphere":
      self.humidity     = int(attrs['humidity'])
      self.vis = int(attrs['visibility'])
      self.pressure     = float(attrs['pressure'])


def WeatherRecvFrom(conn, src, dst, message, flags):
  naim.echo('Message from %s to %s: %s', src, dst, message)


  m = PYWEATHER_RE.match(message)
  if not m:
    return

  zip = m.group(1)
  fetcher = weatherFetcher(src,dst)
  fetcher.fetch(zip)


if __name__ == '__main__':
  naim.hooks.add('recvfrom', 200, WeatherRecvFrom)
