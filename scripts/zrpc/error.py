# Copyright (c) Vilhelm Engström
#
# SPDX-License-Identifier: Apache-2.0

"""zRPC errors"""


class ZrpcError(RuntimeError):
    """Generic zRPC error"""


class MissingOriginError(ZrpcError): ...


class MissingSizeFieldError(ZrpcError): ...


class InvalidSizeFieldError(ZrpcError): ...


class UnsizedFieldError(ZrpcError): ...


class InvalidDirectionError(ZrpcError): ...


class InvalidHexDigest(ZrpcError): ...


class SelfRefError(ZrpcError): ...


class DuplicationError(ZrpcError): ...


class GenerationError(ZrpcError): ...


class ChannelIdReuseError(ZrpcError): ...
