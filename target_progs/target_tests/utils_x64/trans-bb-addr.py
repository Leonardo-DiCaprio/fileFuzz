"""
Copyright 2018 iscas
author: @chengliang, @wangwenshuo
"""

import angr
import logging
import sys
import os
import argparse
import array
import networkx as nx
import json
import platform
from collections import defaultdict
from angr.codenode import BlockNode
from copy import deepcopy
from tqdm import tqdm

l = logging.getLogger("cfg_nav")

loc2cmpaddr = defaultdict(list) #exclude cur basic block
loc2addr = defaultdict(list)  # loc -> [(prev_addr, cur_addr), ...]
loc2cmp = defaultdict(set)    # loc -> set["804db6c: cmp 0xfc(%ebx),%edi", ...]
afl_log_addrs = []
bbl2hash = {}                 # bbl_addr -> hash, different bbls may 
                              # have the same hash
bbl2_seen_hash = defaultdict(set)
bbl2_seen_cmp_insn = defaultdict(set)

call_log_addr = []
next_hash = defaultdict(set)

node2cmpaddr = defaultdict(list)
# trace_cmp_insns_addr = defaultdict(list)

# for inter-function loc calculation
hashed_functions = set()
function_hashes = {}          # function -> (entry_hash_node, [exit_hash_node1, ...])
func_prev_nodes = defaultdict(set) #(f, prev_addr) -> [hash_node, ...]
func_succ_nodes = defaultdict(set)
f2addrhash = dict()

func_start_points = {}
func_end_points = {}
ex_end_addr = {}
# TODO: check if taint analysis can identify all of them
cmp_insn = set(['cmp', 'cmpsb', 'test']) 


def find_afl_log_functions(prog):
    fun_list = []

    for k in prog.kb.functions.values():
        if 'afl' in k.name: # and 'log' in k.name:
            # print "%s: %s" % (hex(k.addr), k.name)
            fun_list.append(k.addr)

    return fun_list

# function name = __afl_maybe_log, could be defined at different addresses
# len(__afl_maybe_log) = 26(32bits) / 28(64bits)
def is_afl_instrunode (prog, f, n):

    b = prog.factory.block(n.addr)
    call_addr = b.capstone.insns[-1].insn.operands[0].imm

    if call_addr in afl_log_addrs:
        return True
    return False


def find_cmp(prog, baddr):
    cmp_set = set()
    fb = prog.factory.block(baddr)
    cb = fb.capstone
    
    for ins in cb.insns:
        if ins.insn.insn_name() in cmp_insn:
            cmp_set.add(str(ins))

    return cmp_set

def find_cmp_insn_addr(prog,baddr):
    cmp_addr_list = []
    fb = prog.factory.block(baddr)
    cb = fb.capstone

    for ins in cb.insns:
        if ins.insn.insn_name() in cmp_insn:
            cmp_addr_list.append(hex(ins.insn.address))
    return cmp_addr_list

def get_addr_range(prog, binary):

    for k in prog.loader.shared_objects.keys():
        if  k == os.path.basename(binary):

            min_addr, max_addr = (prog.loader.shared_objects[k].min_addr, prog.loader.shared_objects[k].max_addr) # angr5: get_x_addr(), angr7: x_addr

    return min_addr, max_addr


# AFL instruments the target by adding blocks that calls the "__afl_maybe_log" function.
# The random instrumented number is put in %ecx, by instr such as "mov    $0x346d,%ecx",
# just before the instr "call   <__afl_maybe_log>" 
def find_hashed_blocks(prog, f):

    addrhash = {}

    for n in f.transition_graph.nodes():
        node2cmpaddr[(f,n.addr)] = find_cmp_insn_addr(prog,n.addr)
        if type(n) is angr.codenode.BlockNode:
                b = prog.factory.block(n.addr)
                menm = b.capstone.insns[-1].insn.insn_name()
                if menm == "call" or menm == "jmp":
                    call_addr = b.capstone.insns[-1].insn.operands[0].imm

                    if call_addr in afl_log_addrs:
                        call_addr = b.instruction_addrs[-1]
                        call_log_addr.append(hex(call_addr))

                        hash = b.capstone.insns[b.instructions - 2].insn.operands[1].reg
                        addrhash[n] = hash
                        bbl2hash[hex(n.addr)] = hash
    
    return addrhash
def get_cmp_insn_addr(prog,f,start,end,addrhash,add=[],remove=[]):
    result = []
    if start == end:
        for s in f.transition_graph.predecessors(end):
            result += get_cmp_addr(prog,f,start,s,addrhash)
    else:
        result += get_cmp_addr(prog,f,start,end,addrhash)
        result = list(set(result)-set(node2cmpaddr[(f,end.addr)]))
    add_list = []
    remove_list = []
    for m in add:
        add_list+=node2cmpaddr[(f,m.addr)]
    for n in remove:
        remove_list += node2cmpaddr[(f,n.addr)]
    result = list(set(result+add_list)-set(remove_list))
    return result
def get_cmp_addr(prog,f,start,end,addrhash):
    o = list(set(addrhash) - set([start, end]))
    cmp_addrs = []
    visited = defaultdict(list)
    paths = []
    tmp = []
    # o = list(set(addrhash)-set([h,node]))
    dfs_exclude_hash(f,start,end,tmp,visited,paths,o)
    for path in paths:
        for node in path:
            cmp_addrs += node2cmpaddr[(f,node.addr)]
    return cmp_addrs
def dfs_exclude_hash(f,start,end,tmp,visited,result,o):
    visited[start] = True
    tmp.append(start)
    if start == end:
        result.append(deepcopy(tmp))
        visited[end]=False
        tmp.pop(-1)
        return
    for s in f.transition_graph.successors(start):
        if s in o or type(s) is angr.knowledge_plugins.functions.function.Function:
            continue
        if not visited[s]:
            dfs_exclude_hash(f,s,end,tmp,visited,result,o)
    visited[start] = False
    tmp.pop(-1)

# Calculate all the possible loc appeared in AFL's trace_bits[]
def calculate_locs2(prog, f, entry, addrhash):

    global loc2addr, loc2cmp, loc2cmpaddr

    cnt = 0

    # the instrumented hash numbers and cmp instructions 
    # that could propagate to cur node
    seen_hash = defaultdict(set)  
    seen_cmp = defaultdict(set)
    seen_cmp_insn_addr = defaultdict(list)

    # f.graph does not include function nodes, but f.transition_graph does
    g = f.transition_graph

    # propagate hash numbers to each non-instrumented blocks
    for node in addrhash: 

        node_cmps = find_cmp(prog, node.addr)
        # node_cmps_addr = find_cmp_insn_addr(prog,node.addr)

        if node.addr == entry.addr:

            cur = addrhash[node]
            loc = (0 >> 1) ^ cur
            # if (hex(node.addr), ) in loc2addr:
            #     print "1stop"
            loc2addr[loc].append((hex(node.addr), ))
            loc2cmp[loc] |= node_cmps
            # if len(list(loc2cmp[loc])) != 0:
            #     print "stop"
            #not record cmp insn addr in cur basic block
        seen_hash[node].add(node)
        # seen_cmp_insn_addr[node].add(node)
        bbl2_seen_hash[(f, node.addr)].add(node)

        # print "[1] %s -> seen_hash" % hex(node.addr)
        for c in list(node_cmps):
            seen_cmp[node].add((node, c))


        propagated = set()

        def propagate_hash(n):

            t = 0
            for s in f.transition_graph.successors(n):
                if type(n) is angr.knowledge_plugins.functions.function.Function:
                    continue
                # not an instrumented node, and never seen before (such as in a loop)
                if not s in addrhash and not s in propagated:

                    t += 1
                    seen_hash[s] |= seen_hash[n]
                    bbl2_seen_hash[(f,s.addr)] |= bbl2_seen_hash[(f,n.addr)]
                    seen_cmp[s] |= seen_cmp[n]
                    node_cmps = find_cmp(prog, n.addr)
                    for c in list(node_cmps):
                        for h in seen_hash[s]:
                            seen_cmp[s].add((h, c))

                    propagated.add(s)
                    t += propagate_hash(s)

            return t

        propagated.add(node)
        cnt += propagate_hash(node)


    # calculate the loc for each edge having hash numbers
    # only happens when cur node is an instrumented node
    for node in addrhash:
        if node != entry:
            for p in g.predecessors(node):

                for h in seen_hash[p]:
                    prev = addrhash[h]
                    cur = addrhash[node]
                    loc = (prev >> 1) ^ cur
                    cnt += 1

                    loc2cmpaddr[loc] += get_cmp_insn_addr(prog,f,h,node,addrhash)
                    loc2addr[loc].append((hex(h.addr), hex(node.addr)))
                    for n, c in seen_cmp[node]:
                        if h == n:
                            loc2cmp[loc].add(c)
                    # if len(list(loc2cmp[loc]))!=0:
                    #     print "stop"


    return cnt


def cfg_navigation(binary):

    global loc2addr, afl_log_addrs
    
    cnt = 0
    hash_cnt = 0
    func_cnt = 0

    load_options = {}
    load_options['auto_load_libs'] = False

    prog = angr.Project(binary, load_options = load_options)

    min_addr, max_addr = get_addr_range(prog, binary)

    # l.info("entry: %x" % p.entry)

    cfg = prog.analyses.CFGFast() # may invoke SE, see if can be changed

    afl_log_addrs = find_afl_log_functions(prog)
    # print "map startpoint to function\n"
    for k in tqdm(prog.kb.functions.keys()):
        f = prog.kb.functions[k]
        f.normalize()

        entry = f.startpoint

        # if (entry.addr < min_addr) or (entry.addr > max_addr):
        #     continue

        func_start_points[f.startpoint]=f
    print "calculte the inter"
    for k in tqdm(prog.kb.functions.keys()):
        f = prog.kb.functions[k]
        f.normalize()
        
        entry = f.startpoint        
        # if (entry.addr < min_addr) or (entry.addr > max_addr):
        #     continue

        addrhash = find_hashed_blocks(prog, f)
        hash_cnt += len(addrhash)

        if len(addrhash) == 0:
            continue

        f2addrhash[f] = addrhash
        hashed_functions.add(f)

        cnt += calculate_locs2(prog, f, entry, addrhash)
        func_cnt += 1

    for k in tqdm(prog.kb.functions.keys()):
        f = prog.kb.functions[k]
        f.normalize()
        entry = f.startpoint
        # if (entry.addr < min_addr) or (entry.addr > max_addr):
        #     continue

        exit_hash_nodes = set()
        exit_funs = set()
        for n in f.transition_graph.nodes:
            if (f.transition_graph.out_degree(n) == 0 and type(
                    n) is not angr.knowledge_plugins.functions.function.Function):
                for h in bbl2_seen_hash[(f,n.addr)]:
                    exit_hash_nodes.add(h)
            if f.transition_graph.out_degree(n) == 1:
                e = list(f.transition_graph.successors(n))[0]
                if type(e) is angr.knowledge_plugins.functions.function.Function:
                    if e.addr in afl_log_addrs or e not in hashed_functions:
                        for h in bbl2_seen_hash[(f, n.addr)]:
                            exit_hash_nodes.add(h)
                    else:
                        exit_funs.add(e)
                elif e in func_start_points:
                    exit_funs.add(func_start_points[e])

        function_hashes[f] = (exit_hash_nodes, exit_funs)


    def find_all_call_functions_ends(prog,f,result,avoid_cyc):
        if f in avoid_cyc:
            return
        avoid_cyc.append(f)
        (exit_hash_nodes, exit_funs) = function_hashes[f]
        result+=exit_hash_nodes
        for x in exit_funs:
            find_all_call_functions_ends(prog,x,result,avoid_cyc)
    def find_node_caller_fun(prog,f,n):
        result = []
        for x in f.transition_graph.successors(n):
            if type(x) is angr.knowledge_plugins.functions.function.Function and x.addr not in afl_log_addrs:
                if x in hashed_functions:
                    result.append(x)
        return result
    def find_all_next_nodes(prog,f,n,result,avoid_cyc):
        if (f,n.addr) in avoid_cyc:
            return
        avoid_cyc.append((f,n.addr))
        if hex(n.addr) in bbl2hash:
            result.append(n)
        else:
            son = find_node_caller_fun(prog,f,n)
            if len(son)<1:
                for x in f.transition_graph.successors(n):
                    if x not in hashed_functions:
                        find_all_next_nodes(prog,f,x,result,avoid_cyc)
            else:
                for x in son:
                    result.append(x.startpoint)
    def cal_hash(f,pnodes,cnodes,n,type):
        for p in set(pnodes):
            for c in cnodes:
                # if 135154976==p.addr and 134517560==c.addr:
                #     print "stop"
                # if 134518688==p.addr and 134797456==c.addr:
                #     print "stop"
                pp_addr = hex(p.addr)
                cc_addr = hex(c.addr)

                pp = bbl2hash[pp_addr]
                cc = bbl2hash[cc_addr]

                loc = (pp >> 1) ^ cc
                # if (pp_addr, cc_addr) in loc2addr[loc]:
                #     print "3stop"
                if type == 1:
                    loc2cmpaddr[loc] += get_cmp_insn_addr(prog,f,p,n,f2addrhash[f],[n])
                if type == 2:
                    remove = []
                    if c in f2addrhash[f]:
                        remove.append(c)
                    loc2cmpaddr[loc] += get_cmp_insn_addr(prog,f,n,c,f2addrhash[f],[p],remove)
                loc2addr[loc].append((pp_addr, cc_addr))
    for k in tqdm(prog.kb.functions.keys()):

        f = prog.kb.functions[k]
        f.normalize()

        entry = f.startpoint
        if (entry.addr < min_addr) or (entry.addr > max_addr):
            continue

        for n in f.transition_graph.nodes():
            for sn in f.transition_graph.successors(n):
                if type(sn) is angr.knowledge_plugins.functions.function.Function:
                    if sn.addr not in afl_log_addrs and sn in hashed_functions:
                        prev = bbl2_seen_hash[(f,n.addr)]
                        start = [sn.startpoint]
                        end = []
                        avoid_cyc = []
                        find_all_call_functions_ends(prog,sn,end,avoid_cyc)
                        next = []
                        for x in f.transition_graph.successors(n):
                            if type(x) is not angr.knowledge_plugins.functions.function.Function:
                                avoid_cyc = []
                                find_all_next_nodes(prog,f,x,next,avoid_cyc)

                        cal_hash(f,prev,start,n,1)
                        cal_hash(f,end,next,n,2)


    print("[+] %d hash node found in %d functions, %d loc(s) recorded after %d calculations." % (len(bbl2hash), func_cnt, len(loc2addr), cnt))

    for key in loc2addr:
        loc2addr[key] = list(set(loc2addr[key]))

    o_dir, o_name = os.path.split(binary)
    o_base, _ = os.path.splitext(o_name)
    if not o_dir:
        o_dir = '.'
    
    l2a_outf = os.path.join(o_dir, o_base + '_loc2tuple.json')
    with open(l2a_outf, "w") as ouf:
        json.dump(loc2addr, ouf)

    l2c_outf = os.path.join(o_dir, o_base + '_loc2cmp.json')
    with open(l2c_outf, "w") as ouf:
        json.dump(loc2cmpaddr, ouf)

    h2b_outf = os.path.join(o_dir, o_base + '_hash2bb.json')
    with open(h2b_outf, "w")  as ouf:
        json.dump(bbl2hash, ouf)

    laddr_outf = os.path.join(o_dir, "call_log_addr.txt")
    with open(laddr_outf, "w") as ouf:
        for c in call_log_addr:
            ouf.write(c + '\n')
    num_null = 0
    for key in loc2cmpaddr:
        if len(loc2cmpaddr[key])==0:
            num_null += 1

    print("[+] See %s for the loc2addr and %s for hash2addr, %d call sites." % (l2a_outf, h2b_outf, len(call_log_addr)))
    print("[+] See %s for the loc2cmp_insn_addr.%d/%d is empty and %d/%d in not empty" % (l2c_outf, num_null, len(loc2cmpaddr), len(loc2cmpaddr) - num_null, len(loc2cmpaddr)))



def main(argv):

    os_bits, _ = platform.architecture()
    # assert os_bits == '64bit', 'Angr needs Python 2.x and a 64-bit system, but this is a %s system.' % os_bits

    l.setLevel(logging.INFO)

    l.info("cfg_navigation start:")

    parser = argparse.ArgumentParser()

    parser.add_argument("bin", help = "the AFL\'s target binary")

    args = parser.parse_args(argv[1:])

    cfg_navigation(args.bin)

if __name__ == "__main__":
    main(sys.argv)
