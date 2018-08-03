
import os
import sys
from subprocess import check_call
import json
import shutil
import time

target_values = ['all', 'common', 'client', 'server']

# TODO Override config with CL arguments

# Get configuration values

with open('build_config.json') as file:
    data = json.load(file)

    build_dir   = data['build_directory']
    config      = 'Debug' if data['debug_build'] else 'Release'
    target      = data['target']
    build_tests = data['build_tests']

    if not target in target_values :
        raise Exception('build_config.json bad "target" value')
    
    cmake_args = [
        '-DVERSION_MAJOR='    + str(data['version_major']),
        '-DVERSION_MINOR='    + str(data['version_minor']),
        '-DCMAKE_BUILD_TYPE=' + config,
        '-DBUILD_CLIENT='     + str(target == 'client' or target == 'all'),
        '-DBUILD_SERVER='     + str(target == 'server' or target == 'all'),
        '-DBUILD_TESTS='      + str(build_tests)
    ]

# Reset the build folder

if os.path.exists(build_dir) and 'clear' in sys.argv :
    shutil.rmtree(build_dir)
    time.sleep(.01)
    os.mkdir(build_dir)

os.chdir(build_dir)

# Build the client and/or server

try :
    check_call(['cmake', '..'] + cmake_args)
    check_call(['cmake', '--build', '.', '--config', config, '--', '/v:m'])
    if build_tests: check_call(['ctest', '-C', config, '-V'])
except : pass
