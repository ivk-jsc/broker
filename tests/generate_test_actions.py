import subprocess
import os.path
from os import path

yaml = """name: Build C++
on: [ push, pull_request ]
jobs:
  build:
    runs-on: ubuntu-latest
    strategy:
      fail-fast: false
      matrix:
        os: [ debian, centos, ubuntu18 ]
        compiler: [ clang, gcc ]
    container: ${{ format('registry.gitlab.com/s.arlyapov/broker_images/{0}:{1}', matrix.os, matrix.compiler) }}
    steps:
      - uses: actions/checkout@v2
      - name: Cmake
        run: cmake .
      - name: Build
        run: cmake --build .
      - name: Add log settings
        run: echo '<config><broker><log><level>7</level><path windows="./" _nix="./">./</path><interactive>false</interactive></log></broker></config>' > broker.xml
      - name: Start broker
        run: ./bins/broker/broker &@tests@
      - name: Upload log
        uses: actions/upload-artifact@v1
        with:
          name: ${{ format('{0}.{1}.log', matrix.os, matrix.compiler) }}
          path: broker.log@checks@
"""

b_dir = input()
test_file = b_dir + "/tests/brokertest/brokertest"

if path.isfile(test_file):
    p = subprocess.Popen("%s --gtest_list_tests" % test_file, shell=True,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT)
    test_ids = []
    check_ids = []
    id = ''
    for line in iter(p.stdout.readline, ''):
        test = line.strip()
        if '.' in line:
            id = test
        else:
            if not test.startswith("DISABLED_"):
                test_id = id + test
                test_name = test_id.replace('.', '_')
                test_ids.append(
                    "\n      - name: %s"
                    "\n        id: %s"
                    "\n        run: ./tests/brokertest/brokertest --gtest_filter=%s && echo ::set-output name=status::success"
                    "\n        continue-on-error: true" %
                    (test_name, test_name, test_id))
                check_ids.append(
                    "\n      - name: Check on failures %s"
                    "\n        if: steps.%s.outputs.status != 'success'"
                    "\n        run: exit 1" %
                    (test_name, test_name))

    retval = p.wait()

    tests = ''.join(map(str, test_ids))
    checks = ''.join(map(str, check_ids))
    yaml = yaml.replace("@tests@", tests)
    yaml = yaml.replace("@checks@", checks)
    f = open("ccpp.yml", "w")
    f.write(yaml)
    f.close()
