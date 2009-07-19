import sys
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


class Commands(object):
  def __init__(self, conn):
    self._conn = conn

naim.types.Commands = Commands

def RegisterCommand(name, cfunc, desc, minarg, maxarg):
  def f(self, *args):
    if not minarg <= len(args) <= maxarg:
      raise TypeError('%s takes between %i and %i arguments (%i given)' % (
          name, minarg, maxarg, len(args)))

    self._conn._conio_stub(cfunc, args)

  f.__name__ = name
  f.__doc__ = desc

  setattr(Commands, name, f)

naim._RegisterCommand = RegisterCommand
