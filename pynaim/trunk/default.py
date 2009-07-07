import naim

def Init():
  import sys
  import traceback

  def Excepthook(exctype, value, tb):
    for l in traceback.format_exception(exctype, value, tb):
      naim.echo(l)

  sys.excepthook = Excepthook

  rawecho = naim.echo
  def Echo(s, *args):
    if args:
      s %= args
    else:
      s = '%s' % s

    rawecho(s)
  Echo.__doc__ = rawecho.__doc__
  naim.echo = Echo

Init()
del Init
