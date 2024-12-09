#
# see: https://adventofcode.com/2024/day/9
# 
 
from time import time
from collections import deque

__startTime = time()

def loadInput(isTest = True):
    global __startTime

    if isTest:
        filename = f"exchange/day{DAY}/claudia/input-example.txt"
    else:
        filename = f"exchange/day{DAY}/claudia/input.txt"

    with open(filename) as f:
        content = [l[:-1] if l[-1] == "\n" else l for l in f.readlines()]

    return content


def writeSolutionFile(part, solution):
    filename = f"exchange/day{DAY}/claudia/solution-for-input.txt"
    parameter = "w" if part == 1 else "a"

    with open(filename, parameter) as f:
        f.write(f"Part {part}: {solution}\n")


def printTimeTaken():
    global __startTime
    __endTime = time()
    print("Time: {:.3f}s".format(__endTime-__startTime))

print()

#########################################################################################
# Day 09
#########################################################################################
DAY = "09"


def createDiskMap(data, isTest=1):
    diskMap=[]

    for i, d in enumerate(data):
        if i%2 == 0:   # file
            diskMap += [str(int(i/2))] * int(d)
        else:
            diskMap += ["."] * int(d)

    if isTest:         
        print(diskMap, "\n")

    return diskMap


def createBlockMap(data):
    blockMap=deque([])

    for i, d in enumerate(data):
        if i%2 == 0 and int(d) > 0:   # file
            blockMap += [[str(int(i/2))] * int(d)]
        elif int(d) > 0:
            
            blockMap += [["."] * int(d)]

    return blockMap


def showBlockMap(blockMap, isTest = True):
    tmpMap = [blockMap[r][c] for r in range(len(blockMap)) for c in range(len(blockMap[r]))]
    if isTest:
        print("".join(tmpMap))
    return tmpMap


def findFreeSpan(blocks, ind, lenBlock):
    if ind > 0:
        for i in range(0, ind):
            if len(blocks[i]) >= lenBlock and blocks[i][0] == ".":
                return i
    return -1


    

def doPart1(isTest = True):
    data = loadInput(isTest)[0]

    diskMap=createDiskMap(data, isTest)       
    showBlockMap(diskMap, isTest)
    

    for i in range(len(diskMap)-1, -1, -1):
        changeInd = diskMap.index(".")
        if changeInd < i:
            diskMap[i], diskMap[changeInd] = diskMap[changeInd], diskMap[i]
            if isTest:
                	print("".join(diskMap))
        else:
            break

    if isTest:
        print()
    
    result = 0
    emptyFieldInd = diskMap.index(".")
    for i in range(emptyFieldInd):
        result += i * int(diskMap[i])
    
    if not isTest:
        writeSolutionFile(1, result)

    return result


            
def doPart2(isTest = True):
    data = loadInput(isTest)[0]
    
    blocks=createBlockMap(data)
    showBlockMap(blocks, isTest)
    
    i = len(blocks) - 1
    
    while i > 1:

        if len(blocks[i]) == 0:
            next
        
        elif blocks[i][0] == ".":
            next
        
        else: 
            g=findFreeSpan(blocks, i, len(blocks[i])) 
            if g != -1:
                diff = len(blocks[g]) - len(blocks[i])
                lenI = len(blocks[i])

                if isTest:   # single steps for example and output for every single step 
                    for c in range(lenI):
                        blocks[g][c], blocks[i][c] = blocks[i][c], blocks[g][c]
                        showBlockMap(blocks, isTest)
                    
                    if len(blocks[g]) != len(blocks[i]): 
                        # Split of the block which is now partially empty
                        blocks.insert(g+1, blocks[g][lenI:])
                        blocks[g] = blocks[g][:lenI]
                    
                    i += 1   # increase i due to added block


                else:   # for big file no output and therefore exchange of the whole block instead of single steps
                    
                    if len(blocks[g]) != len(blocks[i]):
                        blocks[g], blocks[i] = blocks[i] + blocks[g][:diff], blocks[g][:lenI]
                        
                        # Split of the block which is now partially empty
                        blocks.insert(g+1,blocks[g][lenI:])
                        blocks[g] = blocks[g][:lenI]
                    
                        i += 1    # increase i due to added block
                    
                    else:
                        blocks[g], blocks[i] = blocks[i], blocks[g]
                
        i -= 1

    # only creation of the final list - no output
    finalList=showBlockMap(blocks, False)

    if isTest:
        print()

    result = 0

    for i in range(len(finalList)):
        if finalList[i] != ".":
            result += i * int(finalList[i])

    if not isTest:
        writeSolutionFile(2, result)

    return result

#########################################################################################

print("--- PART 1 ---")
print(f"Solution Example: {doPart1()}")
print(f"Solution Part 1:  {doPart1(False)}")


print("\n==============\n")
print("--- PART 2 ---")
print(f"Solution Example: {doPart2()}")
print(f"Solution Part 2:  {doPart2(False)}")

#########################################################################################
print()
printTimeTaken()
