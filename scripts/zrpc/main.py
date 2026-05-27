# Copyright (c) Vilhelm Engström
#
# SPDX-License-Identifier: Apache-2.0

"""zRPC main entrypoint"""

import argparse

from zrpc import generate  # noqa: F401


def main() -> None:
    """Main entrypoint"""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "config", metavar="CONFIG", help="Path to zRPC core configuraiton file"
    )
    subparsers = parser.add_subparsers(dest="_command")
    subparsers.required = True
    generate_parser = subparsers.add_parser("generate")
    generate_parser.add_argument(
        "-o",
        "--outdir",
        metavar="DIR",
        default=None,
        help="Directory to write the genrated files to. If none is provided, the output is written to stdout",
    )
    generate_parser.add_argument(
        "--cmake-out",
        metavar="FILE",
        default=None,
        help="When passed, the script generates a cmake file adding all generated sources to ZEPHYR_CURRENT_LIBRARY and writes it to this path",
    )

    args = parser.parse_args()

    globals()[getattr(args, subparsers.dest)].run(
        **{k: v for k, v in vars(args).items() if k != subparsers.dest}
    )
