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

    #print "�ҵ�",findItem,"��������",rValue
    return rValue

"""
step1 : �ȶ�ȡ��һ�е�������Ϊ��ͷ
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
step2 : �þɵı�ͷͨ���ϲ��ķ�ʽ����һ���µı�ͷ
����������ڶ��ű���title����Щλ�ö�Ӧ�������ڵ�һ�ű���г��ֹ��ģ�
    ����ᱻӦ�����Ժ�step 4��ɸѡdatasheet2�Ľ�������������Զ�����
    �����ڶ��ű�������һ�ű�����ظ�������
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
            #print titleItem.value, "���Ѿ���ǰһ������г��ֹ�����һ�е������ںϲ�ʱ�ᱻ����"
            filterLinesIdx.append( titleOfDataSheet[1].index( titleItem ) )            

#print "��2�����ͬһ������ݵ�ʱ��datasheet2�е������е����ݻᱻ����"
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
        print "���겻��ѧ,�����Ҹ����bug"
    convertMap.append( pairThisIdex )
#print "����һ��������Ҳ�����Ӧ�е�����ʱ���һ�������µ������ϵ��datasheet2�е����ݺϲ��������"
#print convertMap
print("EndOfStep 2-----------------------------------")

"""
step3 :�ֱ��ȡʵ�ʵ����ݵ��ڴ���
issue log : û�п��ǵ��������ұ�ͷʱȥ���յı������ݣ���˻���Ҫ���ǰ�������������
"""
dataSheetIdx = 0
dataSheetColSize = [ len( titleOfDataSheet[0] ),len( titleOfDataSheet[1] ) ]

print "���ݱ��зֱ���",dataSheetColSize, "��"

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
            print "�ڴ���",row,"��",col,"��","ʱ���ָ��е����ݸ���������еĸ�������,���Ժϲ������ɴ���"
    """print listOfLines"""
    dataSheetDatas.append( listOfLines )
    print "done for : ", dataSheet.name

    dataSheetIdx = dataSheetIdx + 1
    
print("EndOfStep 3-----------------------------------")
"""
step4 :�ϲ�ʵ�ʵ���������
�ںϲ�֮ǰ���ȼ��һ��ʵ�ʵ�title������ÿ�����ݵĸ����ܲ����Ǻ�
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
        print "�����д�",lineIndex
    elif -1 != indexDataSheet1 and -1 != indexDataSheet2:
        #print "������������(�Ե�һ���������Ϊ׼)��", lineIndex
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
        #print "ֻ�б��1������:", lineIndex
        lineA = dataSheetDatas[0][indexDataSheet1]
        for item in lineA:
            targetLine.append( item )
        for item in ( dataSheetDatas[1][0] ):
            targetLine.append( nullItem )
            
    elif -1 != indexDataSheet2 and -1 == indexDataSheet1:
        #print "ֻ�б��2������:", lineIndex
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
        print "���ܹ��ɣ���ϵ�Ҹ�bug"
            
    targetData.append( targetLine )
print("EndOfStep 4-----------------------------------")
"""
step5 :���ϲ�������д�ᵽһ���µ��ļ���
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
        #print"����д��",row_idx+1,col_idx,"ֵ=",item
        resultSheet.write( row_idx+1,col_idx, item )

mergedResult.save("�ϲ����.xls")


