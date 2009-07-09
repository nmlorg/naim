#!/usr/bin/python

"""Test for examples.weather."""

__author__ = 'nmlorg (Daniel Reed)'

import naimtest
import weather


class WeatherTest(naimtest.TestCase):
  def testWeatherRecvFrom(self):
    weather.WeatherRecvFrom('conn','test', '#em32', '!pyweather 02906', 0)
    self.assertEvaluated('/msg #em32 It is currently ')


if __name__ == '__main__':
  naimtest.main()
