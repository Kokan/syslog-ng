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
    (r"""python1""", "bzorp python1"),
]


@pytest.mark.parametrize(
    "input_message, expected_value", test_parameters_raw,
    ids=list(map(str, range(len(test_parameters_raw)))),
)
def test_python_destination_parser(config, syslog_ng, input_message, expected_value):
    generator_source = config.create_example_msg_generator_source(num=1, template=config.stringify(input_message))
    python_sngtestmod = config.create_python_sngtestmod_destination(value_pairs="key('MSG') pair('HOST', 'bzorp')")
    config.add_python_inline_codes(python_sngtestmod.python_code)
    config.create_logpath(statements=[generator_source, python_sngtestmod])

    syslog_ng.start(config)
    assert python_sngtestmod.wait_file_content(expected_value)
