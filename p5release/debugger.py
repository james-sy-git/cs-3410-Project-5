import re 

def move_set(): 
    with open("debugger2.txt") as fr: 
        pattern = "0 (r|w) ([0-9A-Fa-f]*) --> .* ([a-z]*) ==> \[set:[\s]*([0-9]*)\]\[way:([0-1])\]"
        # pattern = ':[\s]*([0-9a-f]*)'
        # pattern = re.escape(pattern)
        reg = re.compile(pattern)

        num_set = set() 

        count = 0
        lines = fr.readlines()
        for line in lines: 
            # print(line)
            count += 1

            line = line.strip()

            match = reg.match(line)
            rw = match.group(1).strip()
            addr = match.group(2) 
            hit_miss = match.group(3)
            set_num = int(match.group(4).strip())
            way = match.group(5).strip()

            mask = (~0) >> 6 << 6
            addr = int(addr, 16) & mask
            
            modifier = 'a'
            if not set_num in num_set: 
                modifier = 'x'
                num_set.add(set_num)
            with open(f'singlesets/{str(set_num)}.txt', modifier) as to: 
                to.write(line+"\n")
            
    # str = re.escape(str)
    # print(reg.match(str))
def check_miss(): 
    pattern = "0 (r|w) ([0-9A-Fa-f]*) --> .* ([a-z]*) ==> \[set:[\s]*([0-9]*)\]\[way:([0-1])\]"
    # pattern = ':[\s]*([0-9a-f]*)'
    # pattern = re.escape(pattern)
    reg = re.compile(pattern)

    set_to_addr = dict()

    with open("debugger2.txt") as file: 
        count = 0
        lines = file.readlines()
        for line in lines: 
            # print(line)
            count += 1

            line = line.strip()

            match = reg.match(line)
            rw = match.group(1).strip()
            addr = match.group(2) 
            hit_miss = match.group(3)
            set_num = int(match.group(4).strip())
            way = match.group(5).strip()

            mask = (~0) >> 6 << 6
            addr = int(addr, 16) & mask

            if hit_miss == 'hit': 
                continue

            lst = set_to_addr.get(set_num, [0,0]) 
            # if(set_num == 28): 
            #     print(f"28 tup: {hex(lst[0])}, {hex(lst[1])}\n")
            
            if lst[0] == addr or lst[1] == addr: 
                print(f"missing when present, set: {str(set_num)}, addr: {hex(addr)}, count: {str(count)}")
                print(f"existing addresses: {hex(lst[0])}, {hex(lst[1])}\n")
            lst[int(way)] = addr
            # if(set_num == 28): 
            #     print(f"tup: {hex(lst[0])}, {hex(lst[1])}\n")

            set_to_addr[set_num] = lst
            # if(set_num == 28): 
            #     print(f"tup: {hex(set_to_addr[set_num][0])}, {hex(set_to_addr[set_num][1])}\n")


    print("ended\n")
            
            
    # str = re.escape(str)
    # print(reg.match(str))

move_set()