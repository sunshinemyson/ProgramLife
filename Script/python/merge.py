from xlrd import        open_workbook
from tempfile import    TemporaryFile
from xlwt import        Workbook

workBook = open_workbook( "Origin.xls" )

def getIndex( dataSheet, findItem ):
    rValue = -1
    for i,lines in enumerate( dataSheet ):
        if lines[0] == findItem :
            rValue = i
            break;
    if rValue == len( dataSheet ):
        return -1

    #print "找到",findItem,"的索引是",rValue
    return rValue

"""
step1 : 先读取第一行的内容作为表头
"""
titleOfDataSheet = []
for datasheet in workBook.sheets():
    #print "Get title from : ", datasheet.name
    titleOfThis = []
    for eachTitle in datasheet.row(0):
        if eachTitle.value != '':
            titleOfThis.append( eachTitle )
    #print "titleLine: ", titleOfThis
    titleOfDataSheet.append( titleOfThis )
print("EndOfStep 1-----------------------------------")
"""
step2 : 用旧的表头通过合并的方式产生一个新的表头
新增：计算第二张表格的title中那些位置对应的列是在第一张表格中出现过的，
    这个会被应用与稍后step 4中筛选datasheet2的结果，这样做就自动会抛
    弃掉第二张表格中与第一张表格中重复的数据
"""
targetTitle = []
tempRecordTable = dict()

filterLinesIdx = []

for line in titleOfDataSheet:
    for titleItem in line:
        tempRecordTable[titleItem.value] = 0
        
for line in titleOfDataSheet:
    for titleItem in line:
        if tempRecordTable[titleItem.value] == 0:
            tempRecordTable[titleItem.value] = 1
            targetTitle.append( titleItem.value )
        else:
            #print titleItem.value, "列已经在前一个表格中出现过，这一列的数据在合并时会被忽略"
            filterLinesIdx.append( titleOfDataSheet[1].index( titleItem ) )            

#print "当2个表格同一项都有数据的时候，datasheet2中的以下列的数据会被忽略"
#print filterLinesIdx

convertMap = []
for srcIdx,items in enumerate( titleOfDataSheet[1] ):
    pairThisIdex = []
    pairThisIdex.append( srcIdx )
    for destIdx,destItem in enumerate( targetTitle ):
        if destItem == items.value:
            pairThisIdex.append( destIdx )            
            break
    if len( pairThisIdex ) == 1:
        print "尼玛不科学,来找我改这个bug"
    convertMap.append( pairThisIdex )
#print "当第一个表格中找不到对应行的数据时，我会根据以下的隐射关系把datasheet2中的数据合并到结果中"
#print convertMap
print("EndOfStep 2-----------------------------------")

"""
step3 :分别获取实际的数据到内存中
issue log : 没有考虑到与上面找表头时去除空的表项内容，因此还需要结合前面的内容作修正
"""
dataSheetIdx = 0
dataSheetColSize = [ len( titleOfDataSheet[0] ),len( titleOfDataSheet[1] ) ]

print "数据表中分别有",dataSheetColSize, "行"

dataSheetDatas = []
for dataSheet in workBook.sheets():
    print " get data from:", dataSheet.name
    listOfLines = []
    for row in range(1,dataSheet.nrows):
        line = []
        lineItemCnt = 0
        for col in range(dataSheet.ncols):
            if( lineItemCnt < dataSheetColSize[dataSheetIdx] ):
                line.append( dataSheet.cell(row,col).value )
                lineItemCnt = lineItemCnt + 1
        listOfLines.append( line )
        if len( line ) != dataSheetColSize[dataSheetIdx]:
            print "在处理",row,"行",col,"列","时发现该行的数据个数与标题行的个数不符,这会对合并结果造成错误"
    """print listOfLines"""
    dataSheetDatas.append( listOfLines )
    print "done for : ", dataSheet.name

    dataSheetIdx = dataSheetIdx + 1
    
print("EndOfStep 3-----------------------------------")
"""
step4 :合并实际的数据内容
在合并之前，先检查一下实际的title个数和每行数据的个数能不能吻合
"""
targetData = []

idColItem = []
for dataSheet in dataSheetDatas:
    for lines in dataSheet:
        try :
            idColItem.index( lines[0] )
            pass
        except Exception as e:            
            idColItem.append( lines[0] )

print len( idColItem )

nullItem = ""

for lineIndex in idColItem:
    #print "merging for: ", lineIndex
    indexDataSheet1 = getIndex( dataSheetDatas[0], lineIndex )
    indexDataSheet2 = getIndex( dataSheetDatas[1], lineIndex )
    targetLine = []
    if( indexDataSheet1 == -1 and indexDataSheet2 == -1 ):
        print "这行有错：",lineIndex
    elif -1 != indexDataSheet1 and -1 != indexDataSheet2:
        #print "两个表都有数据(以第一个表的数据为准)：", lineIndex
        lineA = dataSheetDatas[0][indexDataSheet1]
        lineB = dataSheetDatas[1][indexDataSheet2]
        #print "before merge",
        #print lineA,"\n",lineB
        for item in lineA:
            targetLine.append( item )
        for item in lineB:
            try:
                filterLinesIdx.index( lineB.index( item ) )
                pass
            except Exception as e:
                targetLine.append( item )
                
    elif -1 != indexDataSheet1 and -1 == indexDataSheet2:
        #print "只有表格1有数据:", lineIndex
        lineA = dataSheetDatas[0][indexDataSheet1]
        for item in lineA:
            targetLine.append( item )
        for item in ( dataSheetDatas[1][0] ):
            targetLine.append( nullItem )
            
    elif -1 != indexDataSheet2 and -1 == indexDataSheet1:
        #print "只有表格2有数据:", lineIndex
        lineB = dataSheetDatas[1][indexDataSheet2]

        for i in range( len( targetTitle ) ):
            targetLine.insert(i,nullItem)
            
        for index,item in enumerate( lineB ):
            actualIndex = -1
            for eachPair in convertMap:
                if eachPair[0] == index:
                    actualIndex = eachPair[1]
                    break
            
            targetLine[ actualIndex ] = item 
        
    else:
        print "不能够吧，联系我改bug"
            
    targetData.append( targetLine )
print("EndOfStep 4-----------------------------------")
"""
step5 :将合并的数据写会到一个新的文件中
"""
mergedResult = Workbook()
resultSheet = mergedResult.add_sheet('merged result datasheet')

col_idx = 0
#print "\t5.1 write title to result file"
for title in targetTitle:
    resultSheet.write(0,col_idx, title)
    #print "write result:",title
    col_idx = col_idx + 1

for row_idx,targetLine in enumerate(targetData):
    for col_idx,item in enumerate( targetLine ):
        #print"正在写入",row_idx+1,col_idx,"值=",item
        resultSheet.write( row_idx+1,col_idx, item )

mergedResult.save("合并结果.xls")


