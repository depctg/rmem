from glob import glob

dirs = [',']

startLine = 1024
endLine = 1024*3

for resultDir in dirs:
    results = glob(resultDir + '/*.result')
    avgs = []
    for result in results:
        with open(result) as f:
            lines = f.readlines()
            lines = [int(l) for l in lines[startLine:endLine]]
            
            length = len(lines)
            avg = sum(lines) / length
            avgs.append(avg)

    print("{}: AVG {}".format(resultDir, sum(avgs)/len(avgs)))
