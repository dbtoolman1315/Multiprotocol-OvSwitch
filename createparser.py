import createflow
import math

sfparser = 'datasize_t vpMyselfParser    (const void **datap, size_t *sizep)\n\
{\n\
    miniflow_push_myselfs     (mf, filedname,     data);\n\
    data_pull(datap, sizep, fieldsizeone    );\n\
    data_pull(datap, sizep, fieldsizetwo    );\n\
\n\
    indicatetype = *(datasize_t  *)data_pull(datap,sizep,1);\n\
    miniflow_push_be16(mf, yang, yang);\n\
\n\
    //pad\n\
    miniflow_pad_to_64(mf, dl_type);\n\
    return indicatetype\n\
}\n'

sfminiflow = '#define miniflow_push_myself_(MF, OFS, VALUEP)                    \ \n\
{                                                               \ \n\
    miniflow_set_maps(MF, (OFS) / 8, num64);                        \ \n \
    memcpy(MF.data, (VALUEP), bitosize);                \ \n\
    MF.data += 1;                   /* First word only. */      \ \n\
} \n\
#define miniflow_push_myselfs(MF, FIELD, VALUEP)                       \  \n\
    miniflow_push_myself_(MF, offsetof(struct flow, FIELD), VALUEP)\n\n\n'

def sModMiniflowNum(sWholeMiniflow, allsize):
    num64 = allsize / 64
    num64 = math.ceil(num64)
    snum64 = str(num64)
    sWholeMiniflow = sWholeMiniflow.replace('num64',snum64)
    num8 = allsize / 8
    num8 = int(num8)
    num8 = str(num8)
    sWholeMiniflow = sWholeMiniflow.replace('bitosize',num8)
    print(sWholeMiniflow)
    return sWholeMiniflow

def ReplaceStr(sReplaceName,sWholeStr,sYamlName,sAfterYamlName):
    fnameLocation =  sfparser.find(sReplaceName)  
    sWholeStr = list(sWholeStr)

    sname = sYamlName + sAfterYamlName
    size = len(sname)
    for i in range(size):
        sWholeStr[i + fnameLocation] = sname[i]
    sWholeStr = ''.join(sWholeStr)
    return sWholeStr


def sModVariableName(sWholeStr,sFieldName, xFieldSize, cnt, xHeaderSize):
    if(cnt == 1):
        sWholeStr = sWholeStr.replace('filedname', sFieldName)
    s = xFieldSize / 8
    s = int(s)
    s = str(s)
    dSizeToType = {8 : 'uint8_t', 16 :'ovs_be16', 32 : 'ovs_be32'}     
    if(cnt == xHeaderSize):
        sWholeStr = sWholeStr.replace('indicatetype',sFieldName)
        sWholeStr = sWholeStr.replace('datasize_t',dSizeToType[xFieldSize])

    elif(cnt == 1):
        sWholeStr = sWholeStr.replace('fieldsizeone',s)

    elif(cnt == 2):
        sWholeStr = sWholeStr.replace('fieldsizetwo',s)

    print(sWholeStr)
    return sWholeStr


def sModFunName(name):
    global sfparser
    global sfminiflow
    sWholeStr = ReplaceStr('MyselfParser', sfparser, name, 'Parser')

    name = name.lower()
    sWholeStr = ReplaceStr('myselfs', sWholeStr, name, 's')

    sWholeMiniflow = sfminiflow.replace('myself', name)
    print(sWholeMiniflow)
    file_path = 'parser.c'
    #with open(file_path, mode='w', encoding='utf-8') as file_obj:
        #file_obj.write(sfparser)
    return sWholeStr, sWholeMiniflow

def vCreateParser(dHead,name):
    global sfparser
    if(name != None):
        print(1)
    else:
        for fname,val in dHead.items():
            xHeaderSize = len(val)
            dHeadVal = val
            sWholeStr, sWholeMiniflow= sModFunName(fname)
        cnt = 0
        xAllSize = 0
        for fieldname, val in dHeadVal.items():
            cnt = cnt + 1
            for key1,size in val.items():
                xAllSize = xAllSize + size
                sWholeStr = sModVariableName(sWholeStr,fieldname, size, cnt, xHeaderSize)
                if(cnt == xHeaderSize - 1):
                    sWholeMiniflow = sModMiniflowNum(sWholeMiniflow,xAllSize)
        file_path = 'parser.c'
        with open(file_path, mode='w', encoding='utf-8') as file_obj:
            file_obj.write(sWholeMiniflow)
            file_obj.write(sWholeStr)
