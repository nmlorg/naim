import sys
import traceback
import naim


def _myexcepthook(exctype, value, tb):
  for l in traceback.format_exception(exctype, value, tb):
    naim.echo(l)

sys.excepthook = _myexcepthook
