#!/usr/bin/python

"""Test for examples.purrbot."""

__author__ = 'nmlorg (Daniel Reed)'

import naimtest
import purrbot


class PurrbotTest(naimtest.TestCase):
  def testPurr(self):
    conn = naimtest._MockNaim.connections['dummy']
    purrbot.PurrRecvfrom(conn, 'actor', '#naim', '!purr', 0)
    self.assertCommand('ctcp', '#naim', 'ACTION', 'purrs')

  def testPurrToVictim(self):
    conn = naimtest._MockNaim.connections['dummy']
    purrbot.PurrRecvfrom(conn, 'actor', '#naim', '!purr n_', 0)
    self.assertCommand('ctcp', '#naim', 'ACTION', 'purrs at n_')

  def testPrivatePurr(self):
    conn = naimtest._MockNaim.connections['dummy']
    purrbot.PurrRecvfrom(conn, 'actor', None, '!purr', 0)
    self.assertCommand('ctcp', 'actor', 'ACTION', 'purrs')


if __name__ == '__main__':
  naimtest.main()
