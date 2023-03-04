#通过main.yml中的每层的头的名称找到对应的文件
import yaml
import json
import createflow
import createparser

with open('./main.yml', 'r', encoding = 'utf-8') as f:
    result = yaml.load(f.read(), Loader=yaml.FullLoader)
print(result)

aLayer = ['Layer2', 'Layer3', 'Layer4', 'Layer5']
#判断是否为存在的协议
aExistHeader = ['eth','ipv4','ipv6','tcp','udp','tcmp']

createflow.vFlowInit()
for la in aLayer:
    Layeri = result[la]
    for name in Layeri:
        if(None != name):
            #存在的协议直接copy
            if(name in aExistHeader):
                createflow.vCreateFlow(None,name)
            #新的协议再去生成
            else:
                newname = './header_describe/' + name + '.yml'
                print(newname)
                with open(newname,'r', encoding = 'utf-8') as f:
                    Li_result = yaml.load(f.read(), Loader=yaml.FullLoader)
                    json_str=json.dumps(Li_result)
                    print(Li_result)
                    createflow.vCreateFlow(Li_result,None)
                    #createparser.vCreateParser(Li_result,None)


createflow.vFlowEnd()
 


