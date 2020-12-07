import os
import glob

def get_result(result):
    with open(result) as f:
        num = f.readlines()[8]
        return int(num)


dirs  = ['jpeg' + str(x) for x in range(10, 50, 5)]
dirs += ['rep_jpeg' + str(x) for x in range(10, 50, 5)]
dirs += ['sleep' + str(x) for x in range(10, 50, 5)]
dirs += ['jpeg50', 'jpeg60', 'jpeg70']
dirs += ['dist_jpeg40']

total_gbps = (16384 + 196608) / 1024.0 / 1024 * 8
nano_to_sec = 1.0e9 

for resultDir in dirs:

    stats = [get_result(f) for f in glob.glob(resultDir + '/*.result')]
    bw = [total_gbps / (t / nano_to_sec) for t in stats]
    stat = sum(bw)

    print("{} {}".format(resultDir, stat))

