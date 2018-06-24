
import os
import sys
from subprocess import check_call
import json
import shutil

# Get configuration values

with open('config.json') as file:
    data = json.load(file)['pre_build']

    build_dir  = data['build_directory']
    config     = "Debug" if data['debug_build'] else "Release"
    merging    = "ON" if data['merge_client_server'] else "OFF"
    client_log = data['client_logger_verbosity']
    server_log = data['server_logger_verbosity']

# Reset the build folder

if os.path.exists(build_dir):
    shutil.rmtree(build_dir)
os.mkdir(build_dir)
os.chdir(build_dir)

# Build the client & server

cmake_args = ['cmake', '..',
    '-DMERGE_CLIENT_SERVER=' + merging,
    '-DCLIENT_LOGGER_VERBOSITY=' + client_log,
    '-DSERVER_LOGGER_VERBOSITY=' + server_log]

check_call(cmake_args)
check_call(['cmake', '--build', '.', '--config', config, '--', '/v:m'])
check_call(['ctest', '-C', config, '-V'])
