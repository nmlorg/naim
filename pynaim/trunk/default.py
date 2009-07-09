import naim
import naim.hooks

def Init():
  import sys
  import traceback

  def Excepthook(exctype, value, tb):
    for l in traceback.format_exception(exctype, value, tb):
      naim.echo(l)

  sys.excepthook = Excepthook

  # This is a gross hack. Someone please figure out what's going on here.
  #if not hasattr(naim, 'hooks'):
  #  naim.hooks = sys.modules['naim.hooks']

  rawecho = naim.echo
  def Echo(s, *args):
    if args:
      s %= args
    else:
      s = '%s' % (s,)

    rawecho(s)
  Echo.__doc__ = rawecho.__doc__
  naim.echo = Echo

Init()
del Init
