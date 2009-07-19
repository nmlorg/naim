#!/usr/bin/python

"""Test for examples.weather."""

__author__ = 'nmlorg (Daniel Reed)'

import naimtest
import re
import weather


class WeatherTest(naimtest.TestCase):
  def testWeatherToChan(self):
    conn = naimtest._MockNaim.connections['dummy']
    weather.WeatherRecvFrom(conn, 'test', '#em32', '!pyweather 02906', 0)
    self.assertCommand('msg', '#em32', re.compile('^Currently in Providence'))

  def testWeatherToBot(self):
    conn = naimtest._MockNaim.connections['dummy']
    weather.WeatherRecvFrom(conn, 'test', 'pybot', '!pyweather 94035', 0)
    self.assertCommand('msg', 'test', re.compile('^Currently in Mount'))


if __name__ == '__main__':
  naimtest.main()
