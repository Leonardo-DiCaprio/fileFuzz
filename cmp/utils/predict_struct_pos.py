#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import os
import json
import binascii
from collections import defaultdict

ins2keyoff = defaultdict(list) # 0x80857a0:cmp eax, 0xffffffff: '73524742':0:3, ...
keywords = defaultdict(list) # keyword_name -> [keyword1, keyword2, ...]
ins2pos = defaultdict(list) # 0x80857a0:cmp eax, 0xffffffff -> [12:15, 24:27,..]

def tup2str(t):
    return '(' + str(t[0]) + ',' + str(t[1]) + ')'

def read_keyword_file(kwfile):
    global keywords, fixed_length

    with open(kwfile, 'r') as kwf:
        for line in kwf.readlines():
            if "->" in line:
                head, tail = line.split('->')
                field_name = head.strip()
                kwlist = tail.split(',')
                for kw in kwlist:
                    keywords[field_name].append(kw.strip())
    print(keywords)

def find_keywords(cfpath):
    print('Checking %s...' % cfpath)

    # [('74455874', 12), (),..]
    keyword2pos = []
    for field_name, kw_list in sorted(keywords.items(), key=lambda x: len(x[1][0]), reverse=True):

        kw_len = int(len(kw_list[0]) / 2)
        # is it a magic number ?
        if len(kw_list) == 1:

            with open(cfpath, 'rb') as cf:
                fpos = 0
                cf.seek(fpos, 0)
                b = cf.read(kw_len)
                while b:
                    # add = ""
                    hb = binascii.hexlify(b)
                    if hb.decode() == kw_list[0]:
                        # if field_name == "page:CapturePattern":
                        #     cf.seek(fpos + 5, 0)
                        #     b_ = cf.read(1)
                        #     num = int(binascii.hexlify(b_).decode())
                        #
                        #     if num == 2:
                        #         add = "0"
                        #     elif num == 4:
                        #         add = "2"
                        #     else:
                        #         add = "1"
                        keyword2pos.append((hb.decode(), fpos))

                        # break
                    fpos += 1
                    cf.seek(fpos, 0)
                    b = cf.read(kw_len)
        else:
            with open(cfpath, 'rb') as cf:
                fpos = 0
                cf.seek(fpos, 0)
                b = cf.read(kw_len)
                while b:
                    for kw in kw_list:
                        hb = binascii.hexlify(b)
                        if hb.decode() == kw:
                            keyword2pos.append((hb.decode(), fpos))
                            break
                    fpos += 1
                    cf.seek(fpos, 0)
                    b = cf.read(kw_len)

    # keyword2pos[os.path.getsize(cfpath)] = ("EOF", "end_file")
    print("[+] Following keywords are Located:")
    for kp in keyword2pos:
        print('  %s -> %d' % (kp[0],kp[1]))

    return keyword2pos

def retrieve_keyword(item):
    x = item.rfind(')')
    return item[1:x], item[x + 2:]

# 不同文件里关键字的顺序可否不一样
def do_mapping(kw2pos):
    global ins2pos

    pos_buf = defaultdict(set)
    # for k, vl in ins2keyoff.items():
    #     print("%s -> %s" % (k, ','.join(vl)))

    for ins, koff_list in ins2keyoff.items():


        for koff in koff_list:
            if "0x80893ff" in ins:
                print('--' + koff + '--')
            #variable fields, ie. (chunk:type:cname,49454e44):-8|(chunk:type:cname,49454e44):-5
            if '|' in koff:
                # print("-->" + koff)
                head, tail = koff.split('|')

                start_kw, start_off = retrieve_keyword(head)
                sf_type, sf_name = start_kw.split(',')

                end_kw, end_off = retrieve_keyword(tail)
                ef_type, ef_name = end_kw.split(',')

                buf_key = sf_name + '@' + start_off + ':' + ef_name + '@' + end_off

                start_off = int(start_off)
                end_off = int(end_off)

                if buf_key in pos_buf:
                    for t in list(pos_buf[buf_key]):
                        ins2pos[ins].append(t)
                        if "0x80893ff" in ins:
                            print('@' + str(t[0]) + str(t[1]))
                    continue

                for idx, kw in enumerate(kw2pos):
                    if idx + 1 < len(kw2pos):
                        if kw[0] == sf_name and kw2pos[idx + 1][0] in keywords[ef_type]:
                            start_pos = kw[1] + start_off
                            end_pos = kw2pos[idx + 1][1] + end_off
                            ins2pos[ins].append((start_pos, end_pos))
                            pos_buf[buf_key].add((start_pos, end_pos))
                            if "0x80893ff" in ins:
                                print("|<: %d, %d" %(start_pos, end_pos))
                    if idx == len(kw2pos) - 1:
                        if kw[0] == sf_name:
                            start_pos = kw[1] + start_off
                            end_pos = kw[1] + end_off
                            ins2pos[ins].append((start_pos, end_pos))
                            pos_buf[buf_key].add((start_pos, end_pos))

                            if "0x80893ff" in ins:
                                print("|=: %d, %d" %(start_pos, end_pos))
            # fixed length fields, i.e.:(chunk:type:cname,49484452):4:7
            else:
                # print("-->" + koff)

                kw, offs = retrieve_keyword(koff)
                f_type, f_name = kw.split(',')
                start_off, end_off = offs.split(':')

                buf_key = f_name + '@' + start_off + ':' + f_name + '@' + end_off

                start_off = int(start_off)
                end_off = int(end_off)

                if buf_key in pos_buf:
                    for t in list(pos_buf[buf_key]):
                        ins2pos[ins].append(t)
                        if "0x80893ff" in ins:
                            print("-@" + str(t[0]) + str(t[1]))
                    continue

                for kw in kw2pos:
                    if kw[0] == f_name:
                        start_pos = kw[1] + start_off
                        end_pos = kw[1] + end_off
                        ins2pos[ins].append((start_pos, end_pos))
                        pos_buf[buf_key].add((start_pos, end_pos))
                        if "0x80893ff" in ins:
                            print("-: %d, %d" %(start_pos, end_pos))
        pos_list = ins2pos[ins]
        ins2pos[ins]= list(set(pos_list))


    print(ins2pos['0x80893ff:cmp ebp, edi'])

def main():
    global ins2keyoff

    parser = argparse.ArgumentParser()
    parser.add_argument('--keyword_file', default='./keywords_png.txt', action='store', type=str, help='specify the keyword file.')
    parser.add_argument('--ins2keyoff_map', default='fuzz_out6/queue_ins2keyoff.json', action='store', type=str, help='specify the instruction to keyword offset mapping file.')
    parser.add_argument('--target_file', action='store', type=str, help='specify the target file to predict the ins2pos mapping.')

    args = parser.parse_args()    
    if not os.path.isfile(args.ins2keyoff_map):
        print("%s does not exist,exit..." % args.ins2keyoff_map)
        exit(1)
    assert os.path.isfile(args.keyword_file), "%s does not exist" % args.keyword_file
    assert os.path.isfile(args.ins2keyoff_map), "%s does not exist" % args.ins2keyoff_map
    assert os.path.isfile(args.target_file), "%s does not exist" % args.target_file

    with open(args.ins2keyoff_map, 'r') as kf:
        ins2keyoff = json.load(kf)


    read_keyword_file(args.keyword_file)

    # [('74455874', 12), (),..]
    keyword2pos = find_keywords(args.target_file) 

    do_mapping(keyword2pos)

    # print("----output----")
    # for k, vs in ins2pos.items():
    #     s = []
    #     for v in list(vs):
    #         s.append(tup2str(v))
    #     print("%s -> %s" % (k, s))


    d, b = os.path.split(args.target_file)
    f_dname, dname = os.path.split(d)
    out_dname = dname + '_ins2pos'
    out_path = os.path.join(f_dname, out_dname)

    if not os.path.exists(out_path):
        os.mkdir(out_path)

    i2p_fpath = os.path.join(out_path, b + '_ins2pos.json')
    with open(i2p_fpath, 'w') as kf:
        json.dump(ins2pos, kf)

if __name__ == '__main__':
    main()