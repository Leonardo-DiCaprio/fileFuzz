x = raw_input();
s = ""
for i in x:
   s = s+hex(ord(i)).replace("0x","")
print s

