import os
import glob

def get_result(result):
    with open(result) as f:
        num = f.readlines()[8]
        return int(num)


dirs  = ['jpeg' + str(x) for x in range(10, 45, 5)]
dirs += ['rep_jpeg' + str(x) for x in range(10, 50, 5)]

for resultDir in dirs:
    stats = [get_result(f) for f in glob.glob(resultDir + '/*.result')]
    stat = sum(stats) / len(stats)
    print("{} {}".format(resultDir, stat))

