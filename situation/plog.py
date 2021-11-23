import sys
logx = list()
logy = list()

msg_cnt = 0
drop_cnt = 0
fname=  sys.argv[1]
limit = int(sys.argv[2])

def add_entry(x, y) :
    logx.append(x)
    logy.append(y)

with open(fname, "r") as file:
    for line in file:
        if msg_cnt >= limit : break
        
        if "received" in line:
            msg_cnt+=1
            add_entry(msg_cnt, drop_cnt)
        elif "dropped" in line:
            drop_cnt += 1
            msg_cnt+=1
            add_entry(msg_cnt, drop_cnt)

print(logx, logy)

print(f'msg count {msg_cnt}')
print(f'drop rate {drop_cnt*1.0/msg_cnt}')

from matplotlib import pyplot
pyplot.plot(logx,logy)
pyplot.show()
