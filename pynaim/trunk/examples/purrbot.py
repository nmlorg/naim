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
    conn.commands.ctcp(target, 'ACTION', 'purrs at %s' % victim)
  else:
    conn.commands.ctcp(target, 'ACTION', 'purrs')


naim.hooks.add('recvfrom', 200, PurrRecvfrom)

def __exit__():
  naim.hooks.delete('recvfrom', PurrRecvfrom)
