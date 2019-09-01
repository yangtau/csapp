import os
import sys


def parse(filename):
    with open(filename, "r") as f:
        lines = f.readlines()
    num = int(lines[2].split('\n')[0])
    opts = []
    lines = lines[4:]
    lines = [line.split(' ') for line in lines]
    opts = [(line[0], int(line[1]), int(line[2]) if (len(line) > 2) else None)
            for line in lines]
    return num, opts


def truncate(opts: list, num: int):
    return [(a, b, c) for a, b, c in opts if b < num]


def truncate_realloc(opts: list, num: int):
    realloc_cnt = dict()
    res = []
    for op, i, s in opts:
        if op == 'r':
            realloc_cnt.setdefault(i, 0)
            realloc_cnt[i] += 1
            if realloc_cnt[i] < num:
                res.append((op, i, s))
        else:
            res.append((op, i, s))
    return res


def write_to_file(filename, opts):
    alloc_num = sum(1 for opt in opts if opt[0] == 'a')
    alloc_size = sum(opt[2] for opt in opts if opt[0] == 'a')
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
    # filename, block num, realloc cnt
    if len(sys.argv) < 3:
        print("Error: argv")
        exit(-1)
    print(sys.argv)
    file_name = sys.argv[1]
    num = int(sys.argv[2])
    _, opts = parse(file_name)
    opts = truncate(opts, num)
    if len(sys.argv) >= 4:
        realloc_num = int(sys.argv[3])
        opts = truncate_realloc(opts, realloc_num)
    write_to_file(str(num)+file_name, opts)
