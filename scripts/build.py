import subprocess
import logging
import sys
import argparse
from pathlib import Path
import os

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

PROJECT_DIR = Path(__file__).resolve().parent.parent

def setup_parser(root_parser):
    root_parser.add_argument(
        "--build-directory", type=Path, default=Path(PROJECT_DIR / "build"), help="build directory"
    )
    root_parser.add_argument("--jobs", type=int, help="number of parallel jobs to run")
    root_parser.add_argument("--target", help="target to build")
    root_parser.add_argument("--build-type", help="CMake build type to use")

def process_command_line():
    parser = argparse.ArgumentParser(prog="build")
    setup_parser(parser)
    return parser.parse_args()

def build_project(build_directory, jobs=None, target=None, build_type=None):
    os.chdir(PROJECT_DIR)
    cmd = f"cmake -B {build_directory} -S ."
    logger.info("CMake command line: %s", cmd)
    process = subprocess.run(
        cmd, shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
    )
    output = process.stdout
    logger.info("CMake output:\n%s", output)
    process.check_returncode()

    cmd = f"cmake --build {build_directory} --parallel"
    if jobs:
        cmd += f" {jobs}"
    if target:
        cmd += f" --target {target}"
    if build_type:
        cmd += f" --config {build_type}"
    logger.info("CMake command line: %s", cmd)
    process = subprocess.run(
        cmd, shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
    )
    output = process.stdout
    logger.info("CMake output:\n%s", output)
    process.check_returncode()

def main(args=None):
    if not args:
        args = process_command_line()
    logger.debug("Arguments: %s", args)

    try:
        build_project(args.build_directory, args.jobs, args.target, args.build_type)
    except:
        logger.exception("Fatal error")
        return -1

    return 0

if __name__ == "__main__":
    sys.exit(main())