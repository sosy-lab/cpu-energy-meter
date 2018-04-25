# This is a Docker image for running the tests.
# It should be pushed to registry.gitlab.com/sosy-lab/software/cpu-energy-meter/test
# and will be used by CI as declared in ../.gitlab-ci.yml.
#
# Commands for updating the image:
# docker build -t registry.gitlab.com/sosy-lab/software/cpu-energy-meter/test - < Dockerfile
# docker push registry.gitlab.com/sosy-lab/software/cpu-energy-meter/test

FROM ubuntu:xenial
RUN apt-get update && apt-get install -y \
  build-essential \
  debhelper \
  devscripts \
  dh-make \
  libcap-dev \
  ruby
