# This file is part of CPU Energy Meter,
# a tool for measuring energy consumption of Intel CPUs:
# https://github.com/sosy-lab/cpu-energy-meter
#
# SPDX-FileCopyrightText: 2007-2014 Mike Karlesky, Mark VanderVoord, Greg Williams
# SPDX-FileCopyrightText: 2018-2021 Dirk Beyer <https://www.sosy-lab.org>
#
# SPDX-License-Identifier: MIT

# Taken from CMock

suppress_error = !ARGV.nil? && !ARGV.empty? && (ARGV[0].upcase == "--SILENT")

begin
  require "./scripts/unity_test_summary.rb"

  build_dir = ENV.fetch('BUILD_DIR', './build')
  test_build_dir = ENV.fetch('TEST_BUILD_DIR', File.join(build_dir, 'test'))

  results = Dir["#{test_build_dir}/*.testresult"]
  parser = UnityTestSummary.new
  parser.targets = results
  parser.run
  puts parser.report
rescue StandardError => e
  raise e unless suppress_error
end

exit(parser.failures) unless suppress_error
