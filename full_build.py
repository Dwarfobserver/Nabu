
import os
import sys
from subprocess import check_call
import json
import shutil

# Handle Debug/Release argument

argc = len(sys.argv)

if argc > 2:
    print('Wrong number of arguments. Usage : python full_build.py [Opt: Debug | Release]')
    sys.exit(1)

config = sys.argv[1] if argc == 2 else "Debug"

if config != "Debug" and config != "Release":
    print('Wrong argument value. Usage : python full_build.py [Opt: Debug | Release]')

# Get 'merge_client_server' value from config.json

with open('config.json') as file:
    data = json.load(file)
    merging = data['merge_client_server']

# Reset the build folder

shutil.rmtree('build')
os.mkdir('build')
os.chdir('build')

# Build the client & server

cmake_args = ['cmake', '..']
if merging: cmake_args.append('-DMERGE_CLIENT_SERVER=ON')
    
check_call(cmake_args)
check_call(['cmake', '--build', '.', '--config', config, '--', '/v:m'])
check_call(['ctest', '-C', config, '-V'])
