import sys
import traceback
import naim


def Excepthook(exctype, value, tb):
  for l in traceback.format_exception(exctype, value, tb):
    naim.echo(l)

sys.excepthook = Excepthook


def Displayhook(obj):
  if obj is not None:
    naim.echo('%s', obj)

sys.displayhook = Displayhook


rawecho = naim.echo

def Echo(s, *args):
  if args:
    s %= args
  else:
    s = '%s' % (s,)

  rawecho(s)

Echo.__doc__ = rawecho.__doc__
naim.echo = Echo
