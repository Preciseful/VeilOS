from os import system
import multiprocessing
import time
import yaml

print("Running OXAMINER.")
with open('oxaminer.yml', 'r') as file:
    oxaminer = yaml.safe_load(file)

emulator = oxaminer['emulator']

for job, children in oxaminer['jobs'].items():
    print(f'Current job running: {job};')

    timeout = children.get('timeout') or 10
    run = children.get('run')
    expect = children.get('expect')
    expect_file = children.get('expect_file') or 'out'
    emulator_args = children.get('emulator_args') or ''
    system(run)
    print(f'Uploading data to: {expect_file};')
    
    process = multiprocessing.Process(target=system, args=(f'{emulator} {emulator_args}',))
    process.start()

    process.join(timeout)

    if process.is_alive():
        process.terminate()
        process.join()

    with open(expect_file, 'r') as file:
        print(f'Reading contents;')
        if file.read() != expect:
            print(f'Contents don\'t match:')
            print(file.read())
            exit(2)