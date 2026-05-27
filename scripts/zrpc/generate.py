# Copyright (c) Vilhelm Engström
#
# SPDX-License-Identifier: Apache-2.0

"""zRPC generation implementation"""

import inspect
import pathlib
import warnings

from typing import Dict

import jinja2

import zrpc.rpc

from zrpc.error import GenerationError
from zrpc.rpc import Specification


def _render_each_channel(
    spec: Specification,
    dirs: Dict[str, str],
    env: jinja2.Environment,
    outdir: pathlib.Path | None,
) -> None:
    """Render and emit channels from zrpc-channel-x.{h,c}.jinja2 templates.

    :param spec: RPC specification.
    :type spec: :class:`zrpc.rpc.Specification`
    :param dirs: Dict mapping file extensions to subdirectires.
    :type dirs: :class:`typing.Dict[str, str]`
    :param env: Jinja environment.
    :type env: :class:`jinja2.Environment`
    :param outdir: Optional directory to write to.
    :type outdir: :class:`typing.Optional[pathlib.Path]`
    """
    for channel in spec.channels:
        if not channel.rpcs:
            warnings.warn(f"RPC channel {channel.name} names no RPCs, skipping")
            continue

        for ext in ("h", "c"):
            base = "zrpc-channel"
            filename = f"{base}-{channel.name}.{ext}"
            template = env.get_template(f"{base}-x.{ext}.jinja2")
            rendered = template.render(channel=channel)

            if outdir is not None:
                with open(
                    str(outdir / dirs[ext] / filename), "w", encoding="utf-8"
                ) as handle:
                    handle.write(rendered)
            else:
                print(rendered)


def _render_channel_gathering(
    spec: Specification,
    dirs: Dict[str, str],
    env: jinja2.Environment,
    outdir: pathlib.Path | None,
) -> None:
    """Render and emit the channel gathering files based on zrpc-channels.{h,c}.jinja2

    :param spec: RPC specification.
    :type spec: :class:`zrpc.rpc.Specification`
    :param dirs: Dict mapping file extensions to subdirectires.
    :type dirs: :class:`typing.Dict[str, str]`
    :param env: Jinja environment.
    :type env: :class:`jinja2.Environment`
    :param outdir: Optional directory to write to.
    :type outdir: :class:`typing.Optional[pathlib.Path]`
    """
    for ext in ("h", "c"):
        base = "zrpc-channels"
        filename = f"{base}.{ext}"
        template = env.get_template(f"{filename}.jinja2")
        rendered = template.render(spec=spec)

        if outdir is not None:
            with open(
                str(outdir / dirs[ext] / filename), "w", encoding="utf-8"
            ) as handle:
                handle.write(rendered)
        else:
            print(rendered)


def _render_cmake_file(
    spec: Specification,
    env: jinja2.Environment,
    outdir: pathlib.Path | None,
    cmake_out: pathlib.Path,
) -> None:
    """Render and emit the cmake file listing generated files.

    :param spec: RPC specification.
    :type spec: :class:`zrpc.rpc.Specification`
    :param env: Jinja environment.
    :type env: :class:`jinja2.Environment`
    :param outdir: Directory containing the generated files.
    :type outdir: :class:`typing.Optional[pathlib.Path]`
    :param cmake_out: Path to write the generated cmake file to.
    :type cmake_out: :class:`pathlib.Path`
    """
    if outdir is None:
        raise GenerationError("CMake generation requires an outdir parameter")

    template = env.get_template("zrpc-lib.cmake.jinja2")
    rendered = template.render(spec=spec, outdir=outdir)

    with open(str(cmake_out), "w", encoding="utf-8") as handle:
        handle.write(rendered)


def run(
    config: str | pathlib.Path,
    outdir: str | pathlib.Path | None,
    cmake_out: str | pathlib.Path | None,
) -> None:
    """Run zRPC generation.

    :param config: Path to the core config yaml file.
    :type config: :class:`typing.Union[str, pathlib.Path]`
    :param outdir: Optional path to write the files to
    :type outdir: :class:`typing.Optional[typing.Union[str, pathlib.Path]]`
    :param cmake_out: Optional path to write generated cmake file to.
    :type cmake_out: :class:`typing.Optional[typing.Union[str, pathlib.Path]]`
    """
    frame = inspect.currentframe()
    assert frame is not None
    p = (pathlib.Path(inspect.getfile(frame)) / "..").resolve() / "templates"
    if not p.is_dir():
        raise FileNotFoundError(f"Found no templates directory at {p}")

    if outdir is not None:
        if not isinstance(outdir, pathlib.Path):
            outdir = pathlib.Path(outdir)
        outdir.mkdir(parents=True, exist_ok=True)

    if cmake_out is not None:
        if not isinstance(cmake_out, pathlib.Path):
            cmake_out = pathlib.Path(cmake_out)
        cmake_out.parent.mkdir(parents=True, exist_ok=True)

    spec = zrpc.rpc.Specification.from_file(config)
    env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(str(p)), trim_blocks=True, lstrip_blocks=True
    )

    dirs = {"h": "include/zephyr/rpc", "c": "src"}

    if outdir is not None:
        for d in dirs.values():
            (outdir / d).mkdir(parents=True, exist_ok=True)

    _render_each_channel(spec, dirs, env, outdir)
    _render_channel_gathering(spec, dirs, env, outdir)

    if cmake_out is not None:
        _render_cmake_file(spec, env, outdir, cmake_out)
