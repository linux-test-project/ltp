# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2023 SUSE LLC

ARG PREFIX=docker.io/
ARG DISTRO_NAME=alpine
ARG DISTRO_RELEASE=3.18

FROM $PREFIX$DISTRO_NAME:$DISTRO_RELEASE AS build
ARG LTPROOT=/opt/ltp
ARG DISTRO_NAME=alpine
ARG DISTRO_RELEASE=3.18

RUN mkdir /build
WORKDIR /build
COPY . /build
RUN ./ci/${DISTRO_NAME}.sh
RUN git clean -fdX
RUN ./build.sh -p $LTPROOT -i

FROM $PREFIX$DISTRO_NAME:$DISTRO_RELEASE
ARG LTPROOT=/opt/ltp
ARG KIRKROOT=/opt/kirk
ARG DISTRO_NAME=alpine

COPY --from=build /build/ci/${DISTRO_NAME}-runtime.sh $LTPROOT/runtime-deps.sh
RUN $LTPROOT/runtime-deps.sh

COPY --from=build $LTPROOT $LTPROOT
ENV LTPROOT=$LTPROOT
ENV PATH=$LTPROOT/testcases/bin:$LTPROOT/bin:$PATH

RUN mkdir -p $KIRKROOT
COPY --from=build /build/tools/kirk $KIRKROOT

USER ltp
