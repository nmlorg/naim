#!/usr/bin/python

"""Test for examples.purrbot."""

__author__ = 'nmlorg (Daniel Reed)'

import naimtest
import purrbot


class PurrbotTest(naimtest.TestCase):
  def testPurr(self):
    purrbot.PurrRecvfrom(None, 'actor', '#naim', '!purr', 0)
    self.assertEvaluated('/ctcp #naim ACTION purrs')

  def testPurrToVictim(self):
    purrbot.PurrRecvfrom(None, 'actor', '#naim', '!purr n_', 0)
    self.assertEvaluated('/ctcp #naim ACTION purrs at n_')

  def testPrivatePurr(self):
    purrbot.PurrRecvfrom(None, 'actor', None, '!purr', 0)
    self.assertEvaluated('/ctcp actor ACTION purrs')


if __name__ == '__main__':
  naimtest.main()
