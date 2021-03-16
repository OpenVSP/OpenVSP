# Copyright (c) 2020 Uber Technologies, Inc.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


import logging


class StyleAdapter(logging.LoggerAdapter):
    """
    Logging a adapter that supports using new format syntax for messages
    """
    def __init__(self, logger, default_style="%", extra=None):
        super(StyleAdapter, self).__init__(logger, extra)
        self.default_style = default_style

    def process(self, msg, kwargs):
        style = kwargs.pop('style', self.default_style)
        if style == "{":  # optional
            msg = _FormatString(msg)
        return msg, kwargs


class _FormatString(str):
    def __mod__(self, other):
        return self.format(*other)

    def __str__(self):
        return self
