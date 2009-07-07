import naim

def Init():
  import sys
  import traceback

  def _myexcepthook(exctype, value, tb):
    for l in traceback.format_exception(exctype, value, tb):
      naim.echo(l)

  sys.excepthook = _myexcepthook

Init()
del Init
