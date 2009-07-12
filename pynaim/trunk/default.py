import sys
import traceback
import naim


class DummyStdout(object):
  def __init__(self):
    self._remainder = ''

  def write(self, s):
    s = self._remainder + s
    self._remainder = ''

    for s in s.splitlines(True):
      if s[-1] != '\n':
        self._remainder = s
        return

      naim.echo(s)

sys.stdout = DummyStdout()
sys.stderr = DummyStdout()


rawecho = naim.echo

def Echo(s, *args):
  if args:
    s %= args
  else:
    s = '%s' % (s,)

  rawecho(s)

Echo.__doc__ = rawecho.__doc__
naim.echo = Echo
