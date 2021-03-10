#!/usr/bin/env python
#############################################################################
# Copyright (c) 2021 Balabit
# Copyright (c) 2021 Kokan
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as published
# by the Free Software Foundation, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# As an additional exemption you are allowed to compile & link against the
# OpenSSL libraries as published by the OpenSSL project. See the file
# COPYING for details.
#
#############################################################################
import pytest

test_parameters_raw = [
    (r"""python1""", "$FOOBAR", "python1"),
    (r"""python1""", "$MSG", "removed"),
]


@pytest.mark.parametrize(
    "input_message, template, expected_value", test_parameters_raw,
    ids=list(map(str, range(len(test_parameters_raw)))),
)
def test_python_parser(config, syslog_ng, input_message, template, expected_value):
    config.add_include("scl.conf")
    config.add_python_inline_codes(r"""
class MyParser(object):
  def init(self, options):
      return True

  def deinit(self):
      return True

  def parse(self, msg):
      msg['FOOBAR'] = msg['MSG']
      msg['MSG'] = 'removed'
      return True
""")

    generator_source = config.create_example_msg_generator_source(num=1, template=config.stringify(input_message))
    python_parser = config.create_python_parser(clazz="MyParser")

    file_destination = config.create_file_destination(file_name="output.log", template=config.stringify(template + "\n"))
    config.create_logpath(statements=[generator_source, python_parser, file_destination])

    syslog_ng.start(config)
    assert file_destination.read_log().strip() == expected_value
