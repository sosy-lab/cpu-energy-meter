# This file is part of CPU Energy Meter,
# a tool for measuring energy consumption of Intel CPUs:
# https://github.com/sosy-lab/cpu-energy-meter
#
# SPDX-FileCopyrightText: 2018-2026 Dirk Beyer <https://www.sosy-lab.org>
#
# SPDX-License-Identifier: BSD-3-Clause

# This is a container image for running the tests.
# It should be pushed to registry.gitlab.com/sosy-lab/software/cpu-energy-meter/test
# and will be used by CI as declared in ../.gitlab-ci.yml.
#
# Commands for updating the image:
# podman build --pull -t registry.gitlab.com/sosy-lab/software/cpu-energy-meter/test - < test/Dockerfile
# podman push registry.gitlab.com/sosy-lab/software/cpu-energy-meter/test

FROM ubuntu:resolute
RUN apt-get update && apt-get install -y \
  build-essential \
  debhelper \
  devscripts \
  dh-make \
  libcap-dev \
  ruby
