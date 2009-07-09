"""pynaim weather bot (responds to !pyweather <zipcode>)."""

__author__ = 'eminence (Andrew Chin)'

import httplib
import re
import naim


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


if __name__ == '__main__':
  naim.hooks.add('recvfrom', 200, WeatherRecvFrom)
