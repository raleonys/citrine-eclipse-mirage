# SPDX-License-Identifier: GPL-3.0-only
"""Scoped PE/CRT checks over the shared immutable image view;not the full Golden chain."""
from pe_image import Image
from golden import binary_acceptance

def inspect(data,*,native=False):
    return binary_acceptance(Image(data),native=native)
