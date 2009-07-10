#!/usr/bin/python

"""Test for examples.weather."""

__author__ = 'nmlorg (Daniel Reed)'

import naimtest
import weather


class WeatherTest(naimtest.TestCase):
  def testWeatherToChan(self):
    weather.WeatherRecvFrom('conn','test', '#em32', '!pyweather 02906', 0)
    self.assertEvaluated('/msg #em32 Currently in Providence')
  def testWeatherToBot(self):
    weather.WeatherRecvFrom('conn','test', 'pybot', '!pyweather 94035', 0)
    self.assertEvaluated('/msg test Currently in Mount')


if __name__ == '__main__':
  naimtest.main()
