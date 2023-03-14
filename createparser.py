import createflow
import math

sparsername = 'size_t parser_name(void **datap, uint32_t *sizep, struct mf_ctx *mfp)\n\
{ \n'

sfunname = '    miniflow_push_stype((*mfp), filedname, *(ltype *)data_pull(datap, sizep, sizeof(ltype)));\n\n'

sfunnamestruct = '    miniflow_push_uint8((*mfp), fieldname.eb[cnt], *(uint8_t *)data_pull(datap, sizep, sizeof(uint8_t)));\n'

stypename = '    size_t rtype = *(size_t *)data_pull(datap, sizep, sizeof(size_t));\n\
    miniflow_push_size_s((*mfp), fieldname, rtype);\n\n'

spadname = '    miniflow_pad_to_64((*mfp), fieldname);\n\n'

slast = '    return rtype;\n\
}\n'
def sParserName(fname):
    global sparsername
    sWholeStr = sparsername.replace('name', fname)
    return sWholeStr

def sFunName(sWholeStr,size,fname):
    global sfunname
    global sfunnamestruct
    global stypename
    global spadname

    dSizeToTypeS = {8 : 'uint8', 16 :'be16', 32 : 'be32'}  
    dSizeToTypeL = {8 : 'uint8_t', 16 :'ovs_be16', 32 : 'ovs_be32'}   

    if 'type' in fname:
        stype = stypename.replace('size_t', dSizeToTypeL[size])
        stype = stype.replace('size_s',dSizeToTypeS[size])
        stype = stype.replace('fieldname',fname)
        sWholeStr = sWholeStr + stype
    
    
    #elif 'pad' in fname:
    #    spad = spadname.replace('fieldname',lastfieldname) 
    #    sWholeStr = sWholeStr + spad
    

    else :
        if(size in dSizeToTypeS.keys()):
            
            saftername = sfunname.replace('stype',dSizeToTypeS[size])
            slongtype = saftername.replace('ltype',dSizeToTypeL[size])
            sfiledname = slongtype.replace('filedname', fname)

            sWholeStr = sWholeStr + sfiledname

        else :

            sstructname = sfunnamestruct.replace('fieldname',fname)
            stemp = sstructname
            cnt = size /8
            cnt = int(cnt)
            for i in range(cnt):
                si = str(i)
                sstructname = stemp.replace('cnt',si)
                sWholeStr = sWholeStr + sstructname

            
            #sWholeStr = sWholeStr + sstructname

    return sWholeStr

def sPad(sWholeStr,padsize,lastfieldname):
    if padsize != 0:
        spad = spadname.replace('fieldname',lastfieldname) 
        sWholeStr = sWholeStr + spad
    return sWholeStr

def vCreateParsera(dHead,name,padsize):
    file_path = 'parser.c'
    if(name != None):
        print(1)
    else:
        for fname,val in dHead.items():
            sWholeStr = sParserName(fname)
            dHeadVal = val
        if('Type' in val.keys()):
            val.pop('Type')
        for fieldname, val in dHeadVal.items():
            lastfieldname = fieldname
            for key1,size in val.items():
                sWholeStr = sFunName(sWholeStr, size, fieldname)
                print (sWholeStr)
        sWholeStr = sPad(sWholeStr,padsize,lastfieldname)
        sWholeStr = sWholeStr + slast
        
        with open(file_path, mode='a', encoding='utf-8') as file_obj:
            file_obj.write(sWholeStr)

def vParserInit():
    #先打开parser.c
    file_path = 'parser.c'
    with open(file_path, mode='w', encoding='utf-8') as file_obj:
            file_obj.write('include "parser.h\n\n\n"')