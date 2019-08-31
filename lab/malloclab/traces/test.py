import os
import sys


def parse(filename):
    with open(filename, "r") as f:
        lines = f.readlines()
    alloc_num = int(lines[1].split('\n')[0])
    opts = []
    lines = lines[4:]
    lines = [line.split(' ') for line in lines]
    opts = [(line[0], int(line[1]), int(line[2]) if (len(line) > 2) else None)
            for line in lines]
    return alloc_num, opts


def truncate(opts: list, num: int):
    return [(a, b, c) for a, b, c in opts if b < num]


def write_to_file(filename, opts):
    alloc_num = sum(1 for opt in opts if opt[0] == 'a')
    alloc_size = sum(int(opt[2]) for opt in opts if opt[2] != None)
    lines = [str(alloc_size)+'\n', str(alloc_num) +
             '\n', str(len(opts))+'\n', "1\n"]
    for opt in opts:
        opt_str = opt[0] + " " + \
            str(opt[1]) + ((" " + str(opt[2]))
                           if opt[2] != None else "") + "\n"
        lines.append(opt_str)
    with open(filename, "w") as f:
        f.writelines(lines)


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Error: argv")
        exit(-1)
    print(sys.argv)
    file_name = sys.argv[1]
    num = int(sys.argv[2])
    _, opts = parse(file_name)
    opts = truncate(opts, num)
    write_to_file(str(num)+file_name, opts)
