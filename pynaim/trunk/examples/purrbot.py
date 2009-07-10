"""pynaim purr bot (responds to !purr [<user>])."""

__author__ = 'nmlorg (Daniel Reed)'

import naim


def PurrRecvfrom(conn, src, dst, message, flags):
  if not message.startswith('!purr'):
    return

  target = dst or src

  message = message.strip()
  if ' ' in message:
    victim = message.split()[1]
    naim.eval('/ctcp %s ACTION purrs at %s' % (target, victim))
  else:
    naim.eval('/ctcp %s ACTION purrs' % target)


naim.hooks.add('recvfrom', 200, PurrRecvfrom)

def __exit__():
  naim.hooks.delete('recvfrom', PurrRecvfrom)
