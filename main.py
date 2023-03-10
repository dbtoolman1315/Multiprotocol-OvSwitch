#通过main.yml中的每层的头的名称找到对应的文件
import yaml
import json
import createflow
import createparser
import createmfext

with open('./main.yml', 'r', encoding = 'utf-8') as f:
    result = yaml.load(f.read(), Loader=yaml.FullLoader)
print(result)

aLayer = ['Layer2', 'Layer3', 'Layer4', 'Layer5']
#判断是否为存在的协议
aExistHeader = ['eth','ipv4','ipv6','tcp','udp','tcmp']
Ttype = {}
Ttype_size = {}

createflow.vFlowInit()
createparser.vParserInit()
padsize = 0
for la in aLayer:
    Layeri = result[la]
    for name in Layeri:
        if(None != name):
            #存在的协议直接copy
            if(name in aExistHeader):
                padsize = createflow.vCreateFlow(None,name)
            #新的协议再去生成
            else:
                newname = './header_describe/' + name + '.yml'
                print(newname)
                with open(newname,'r', encoding = 'utf-8') as f:
                    Li_result = yaml.load(f.read(), Loader=yaml.FullLoader)
                    json_str=json.dumps(Li_result)
                    print(Li_result)
                    padsize = createflow.vCreateFlow(Li_result,None)
                    createparser.vCreateParsera(Li_result,None,padsize)            
            with open('./header_describe/'+name+'.yml','r', encoding = 'utf-8') as f:
                    Li_result = yaml.load(f.read(), Loader=yaml.FullLoader)
                    #print(Li_result)
                    createmfext.Ttype_get(Li_result,name,Ttype,Ttype_size)
                    
createmfext.create_extract(Ttype,Ttype_size)

createflow.vFlowEnd()
 


