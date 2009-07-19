"""Simple naim-specific extension of the Python unittest module.

To use, create a modulename_test.py that imports this module (naimtest) before 
importing modulename (and all other naim modules), then subclass 
naimtest.TestCase and run naimtest.main (just as you would do with the unittest 
module). naimtest preinserts a mock of pynaim's naim module into sys.modules so 
other modules can "import naim".
"""

__author__ = 'Daniel Reed (nmlorg)'

import sys
import unittest


HOOKS = []
ECHOES = []
EVALS = []


class _MockCommands(object):
  def __init__(self, commandlog):
    self._commandlog = commandlog

  def __getattr__(self, key):
    def f(*args):
      self._commandlog.append((key,) + args)

    return f


class _MockConnection(object):
  def __init__(self, winname):
    self.winname = winname
    self._commandlog = []
    self.commands = _MockCommands(self._commandlog)


class _MockNaim(object):
  connections = {'dummy': _MockConnection('dummy')}

  class hooks(object):
    @staticmethod
    def add(name, weight, func):
      HOOKS.append((name, weight, func))

  @staticmethod
  def echo(s, *args):
    if args:
      s %= args
    else:
      s = '%s' % (s,)

    ECHOES.append(s)

  @staticmethod
  def eval(s):
    EVALS.append(s)

sys.modules['naim'] = _MockNaim


class TestCase(unittest.TestCase):
  def assertHooked(self, name, weight, func):
    k = (name, weight, func)
    if k not in HOOKS:
      raise self.failureException('%s not hooked' % (k,))

  def assertEchoed(self, s):
    for e in ECHOES:
      if s in e:
        return

    raise self.failureException('%r was not echoed' % s)

  def assertEvaluated(self, s):
    for e in EVALS:
      if e.startswith(s):
        return

    raise self.failureException('%r was not evaluated' % s)

  def assertCommand(self, *args):
    for command in _MockNaim.connections['dummy']._commandlog:
      if len(command) != len(args):
        continue

      for arg1, arg2 in zip(args, command):
        if isinstance(arg1, str):
          if arg1 != arg2:
            break
        else:
          if not arg1.match(arg2):
            break
      else:
        return

    raise self.failureException('%s not called' % (args,))


main = unittest.main
