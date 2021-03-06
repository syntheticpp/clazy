#!/usr/bin/python2

import sys, os

#-------------------------------------------------------------------------------
# Change here, if needed

QT_FLAGS = "-I /usr/include/qt/ -fPIC"
#-------------------------------------------------------------------------------
# Global variables

_compiler_comand = "clang++ -std=c++11 -Wno-unused-value -Qunused-arguments -Xclang -load -Xclang ClangLazy.so -Xclang -add-plugin -Xclang clang-lazy -Xclang -plugin-arg-clang-lazy -Xclang no-inplace-fixits -c " + QT_FLAGS
_dump_ast_command = "clang++ -std=c++11 -fsyntax-only -Xclang -ast-dump -fno-color-diagnostics -c *.cpp " + QT_FLAGS
_help_command = "clang++ -Xclang -load -Xclang ClangLazy.so -Xclang -add-plugin -Xclang clang-lazy -Xclang -plugin-arg-clang-lazy -Xclang help -c empty.cpp"
_dump_ast = "--dump-ast" in sys.argv
_verbose = "--verbose" in sys.argv
_help = "--help" in sys.argv

#-------------------------------------------------------------------------------
# utility functions

def run_command(cmd):
    if os.system(cmd) != 0:
        return False
    return True

def print_usage():
    print "Usage for " + sys.argv[0].strip("./") + ":\n"
    print "    " + sys.argv[0] + " [--help] [--dump-ast] [check1,check2,check3]"
    print
    print "    Without any check supplied, all checks will be run."
    print "    --dump-ast is provided for debugging purposes.\n"
    print "Help for clang plugin:"
    print
    run_command(_help_command)

def files_are_equal(file1, file2):
    try:
        f = open(file1, 'r')
        lines1 = f.readlines()
        f.close()

        f = open(file2, 'r')
        lines2 = f.readlines()
        f.close()

        return lines1 == lines2
    except:
        return False

def get_check_list():
    return filter(lambda entry: os.path.isdir(entry), os.listdir("."))

# Returns all files with .cpp_fixed extension. These were rewritten by clang.
def get_fixed_files():
    return filter(lambda entry: entry.endswith('.cpp_fixed.cpp'), os.listdir("."))

def print_differences(file1, file2):
    return run_command("diff -Naur test.expected test.output")

def extract_word(word, in_file, out_file):
    in_f = open(in_file, 'r')
    out_f = open(out_file, 'w')
    for line in in_f:
        if word in line:
            out_f.write(line)
    in_f.close()
    out_f.close()

def cleanup_fixed_files():
    fixed_files = get_fixed_files()
    for fixed_file in fixed_files:
        os.remove(fixed_file)

def run_check_unit_tests(check):
    cmd = _compiler_comand + " -Xclang -plugin-arg-clang-lazy -Xclang " + check + " *.cpp "

    if _verbose:
        print "Running: " + cmd

    cleanup_fixed_files()

    if not run_command(cmd + " &> compile.output"):
        print "[FAIL] " + check + " (Failed to build test. Check " + check + "/compile.output for details)"
        print
        return False

    extract_word("warning:", "compile.output", "test.output")

    if files_are_equal("test.expected", "test.output"):
        print "[OK]   " + check
    else:
        print "[FAIL] " + check
        if not print_differences("test.expected", "test.output"):
            return False

    # If fixits were applied, test they were correctly applied
    fixed_files = get_fixed_files()
    for fixed_file in fixed_files:
        if run_command(_compiler_comand + " " + fixed_file + " &> compile_fixed.output"):
            print "   [OK]   " + fixed_file
        else:
            print "   [FAIL] " + check + " (Failed to build test. Check " + check + "/compile_fixed.output for details)"
            print
            return False

    return True

def dump_ast(check):
    run_command(_dump_ast_command + " > ast_dump.output")
#-------------------------------------------------------------------------------
# main

if _help:
    print_usage()
    sys.exit(0)

args = sys.argv[1:]

switches = ["--verbose", "--dump-ast", "--help"]

if _dump_ast:
    del(args[args.index("--dump-ast")])


all_checks = get_check_list()
requested_checks = filter(lambda x: x not in switches, args)
requested_checks = map(lambda x: x.strip("/"), args)

for check in requested_checks:
    if check not in all_checks:
        print "Unknown check: " + check
        print
        sys.exit(-1)

if not requested_checks:
    requested_checks = all_checks

for check in requested_checks:
    os.chdir(check)
    if _dump_ast:
        dump_ast(check)
    else:
        run_check_unit_tests(check)

    os.chdir("..")
