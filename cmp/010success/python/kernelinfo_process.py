import sys
import re

if __name__ == '__main__':
    name1 = sys.argv[1]
    name2 = sys.argv[2]
    f = open(name1,"r")
    f_write = open(name2,"a")
    f_write.write("[my_kernelinfo]\n")
    datas = f.readlines()
    start = 0
    end = 0
    s = ""
    for data in datas:
	if data.find("---KERNELINFO-END---")>=0:
            end = 1
        #print(data)
        if end == 1:
            break
        if start == 1:
            #pass
            new_data = re.sub(r"\[.*\] ","",data)
            f_write.write(new_data)
        if data.find("--KERNELINFO-BEGIN--")>=0:
            start = 1
        
