#!/usr/bin/env python3

# EPOS Bignum tester
# This script reads output generated by src/utility/bignum_test.cc
# You should run this script after running bignum_test to verify the results

test_output = "img/bignum_test.out"
errors = 0
digit_size = 4

# Find a Bignum in EPOS' format in the given line and convert it to a python int
def find_bignum(line):
    global digit_size

    line = line.replace(' ', '')
    line = line[line.find('[')+1 : line.find(']')]
    line = line.split(",")
    line.reverse()

    ret = 0;
    for i in line:
        ret <<= (digit_size * 8)
        ret += int(i)
    return ret

def read_line(f):
    global errors

    line = f.readline()
    line = line.rstrip()
    if line == 'Done!' or line == '':
        print("\nVerification done with", errors, "errors.")
        exit()

    return line


print("Opening file", test_output)
with open(test_output, 'r') as f:
    print("Searching for Digit size...")
    ok = False
    line = ''
    while not ok:
        line = read_line(f)
        ok = line.find("sizeof(Bignum<") != -1 and line.find("::Digit") != -1 and line.find("bytes") != -1

    line = line.replace(' ','')
    line = line[line.find("=")+1 : line.find("bytes")]
    digit_size = int(line)
    print("Digit size found:", digit_size, "bytes.")

    print("Searching for modulo...")
    line = f.readline()
    if line == '':
        exit()
    while line[0:6] != "Modulo":
        line = read_line(f)

    mod_raw = line
    mod = find_bignum(line) + 1
    print("Modulo found:", mod)

    print("Verifying EPOS' output...")
    print("Operations parsed:")
    op_count = 0
    while True:
        a_raw = read_line(f)
        a = find_bignum(a_raw)

        b_raw = read_line(f)
        b = find_bignum(b_raw)

        c_raw = read_line(f)
        c = find_bignum(c_raw)

        op = c_raw.split()[1]
        if op == '+':
            expected = ((a + b) % mod)
        elif op == '-':
            expected = ((a - b) % mod)
        elif op == '*':
            expected = ((a * b) % mod)
        elif op == '/':
            inv = pow(b, mod-2, mod)
            expected = ((a * inv) % mod)
        elif op == 'mod_exp':
            mod_raw = read_line(f)
            mod = find_bignum(mod_raw)
            expected = pow(a, b, mod)

        if expected != c:
            errors += 1
            print("\nOperation FAILED!")
            print("EPOS' output:")
            print(a_raw)
            print(b_raw)
            print(c_raw)
            print(mod_raw)
            print("Interpretation:")
            print("a =",a)
            print("b =",b)
            print("a",op,"b =",c)
            print("mod =",mod)
            print("correct result =",expected)

            expected_raw = '[' + str(expected & (0xffffffff))
            expected >>= (digit_size * 8)
            while expected > 0:
                expected_raw += ', '
                expected_raw += str(expected & (0xffffffff))
                expected >>= (digit_size * 8)
            expected_raw += ']'

            print("correct result in array form =",expected)


            print("Operations parsed:")
            print('', op_count, end='\r')

        op_count += 1
        print('', op_count, end='\r')
