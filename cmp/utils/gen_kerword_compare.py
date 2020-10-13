#!/usr/bin/env python3

import os, argparse, json
from collections import defaultdict
import binascii

ins2offsets = defaultdict(set)  # '0x805f033:cmp ebx, 0x5' --> {'37,38,39,40', ...}

offset2struct = {}  # i.e.: int(offset): chunk[0]:type:ctype(4)

ins2keyoff = defaultdict(list)  # 0x80857a0:cmp eax, 0xffffffff: '73524742':0:3, ...

struct2keywordoff = {}  # 'chunk[1]:type:cname -> '73424954':0:3

tdir_ext, tfile_ext = '_taint_logs', '.taint.log'
sdir_ext, sfile_ext = '_parsed_structs', '_struct'

keywords = defaultdict(list)  # keyword_name -> [keyword1, keyword2, ...]
fixed_length = []


def tup2str(t):
    return '(' + str(t[0]) + ',' + str(t[1]) + ')'


def read_taint_file(taintfile):
    global ins2offsets

    ins2offsets.clear()

    with open(taintfile) as tf:
        for line in tf.readlines():
            line = line.strip()
            # print(line)
            if len(line) > 0 and line[0] == '{':
                pos = line.find('0x')
                offsets = line[:pos - 1]
                ins_line = line[pos:]

                delm = ins_line.find(' ')
                addr = ins_line[:delm]
                rb = ins_line.find('[')
                ins = ins_line[delm + 1: rb]
                ins_key = addr + ':' + ins

                off_set = set()
                for off in offsets.split('}'):
                    bl = []
                    # print('off = ' + off)
                    for sr in off.split(','):
                        if sr:
                            b = sr.split(':')[1]
                        else:
                            continue
                        bl.append(b)
                    if bl:
                        bs = ','.join(bl)
                        off_set.add(bs)

                for off in off_set:
                    ins2offsets[ins_key].add(off)


def read_struct_file(structfile):
    global offset2struct
    offset2struct.clear()

    with open(structfile, 'r') as sf:
        prefix = []
        for line in sf.readlines():
            line = line[:-1]
            beg = len(line) - len(line.lstrip())

            if beg == 0:
                # top level
                continue

            cur, cur_lv = line[beg:], beg / 4
            if len(cur.strip()) == 1 and cur[0] == '}':
                # end line of a struct/union
                prefix.pop()
                # print('---' + ':'.join(prefix))
            else:
                left, _, right = cur.partition('=')
                beg, tail = left.find(' '), right[-1]

                line_offset = int(left[:beg], 16)  # hex string -> dec int

                cur_key = left[beg + 1:].strip()

                if tail == '{':
                    # a start of a struct/union
                    # print('prefix <- ' + cur_key)
                    prefix.append(cur_key)
                    continue
                # elif tail == ']':
                #     # start of an array
                #     # print('prefix[a] <- ' + cur_key)
                #     # prefix.append(cur_key)
                #     line_name = ':'.join(prefix) + ':' + cur_key if prefix else cur_key
                else:
                    # leaf nodes
                    line_name = ':'.join(prefix) + ':' + cur_key if prefix else cur_key

                # # the offset overwriting happens at the start of a structure or an array
                # if line_offset in offset2struct:
                #     print('-----' +  offset2struct[line_offset])
                offset2struct[line_offset] = line_name

    sorted_keys = sorted(offset2struct.keys())
    pred = sorted_keys[0]
    for k in sorted_keys[1:]:
        offset2struct[pred] = offset2struct[pred] + '(' + str(k - pred) + ')'
        pred = k

    # need file size to compute the last field's size
    fname = os.path.splitext(os.path.basename(structfile))[0]
    fsize = int(fname.split('_')[-1])
    offset2struct[pred] = offset2struct[pred] + '(' + str(fsize - pred) + ')'

    # for k in sorted(offset2struct.keys()):
    #     print('%d --> %s' % (k, offset2struct[k]))


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
            else:
                fixed_length.append(line.strip())


def find_keywords(cfpath):
    print('[+] Checking %s...' % cfpath)

    # 12 -> {(chunk:type:cname, '73524742'), ...}
    cur_keywords = {}
    for field_name, kw_list in sorted(keywords.items(), key=lambda x: len(x[1][0]), reverse=True):

        kw_len = int(len(kw_list[0]) / 2)
        # is it a magic number ?
        if len(kw_list) == 1:

            with open(cfpath, 'rb') as cf:
                fpos = 0
                cf.seek(fpos, 0)
                b = cf.read(kw_len)
                while b:
                    # if b.hex() == kw_list[0]: # bytes.hex() only works under python3.5+
                    #     cur_keywords[fpos] = (field_name, b.hex())
                    hb = binascii.hexlify(b)
                    if hb.decode() == kw_list[0]:
                        cur_keywords[fpos] = (field_name, hb.decode())
                        # continue
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
                        # if b.hex() == kw: # bytes.hex() only works under python3.5+
                        #     cur_keywords[fpos] = (field_name, b.hex())
                        hb = binascii.hexlify(b)
                        if hb.decode() == kw:
                            cur_keywords[fpos] = (field_name, hb.decode())
                            break
                    fpos += 1
                    cf.seek(fpos, 0)
                    b = cf.read(kw_len)

    # cur_keywords[os.path.getsize(cfpath)] = ("EOF", "end_file")
    print("[+] Following keywords are found:")
    for k, v in cur_keywords.items():
        tt = '(' + v[0] + ',' + v[1] + ')'
        print('%s -> %s' % (k, tt))

    return cur_keywords


def check_struct_len(struct):
    x = struct.rfind(']')
    if x == -1:
        field = struct.split('(')[0]
    else:
        field, in_bracket = '', False
        for c in struct:
            if c == '[':
                in_bracket = True
            elif c == ']':
                in_bracket = False
            elif not in_bracket:
                field += c
        field = field.split('(')[0]

    if field in fixed_length or field in keywords.keys():
        return True
    else:
        return False


def map_variable_len(struct_buf, t, last_kw, cur_kw):
    global struct2keywordoff

    kw_pos, kw_name = cur_kw
    last_kw_pos, last_kw_name = last_kw

    # for variable structs in struct_buf
    start_pos = struct_buf[0] - last_kw_pos
    # print("size = %d, t + 1 = %d" %(len(struct_buf), t + 1))
    end_pos = struct_buf[t + 1] - 1 - kw_pos

    start_offset = tup2str(last_kw_name) + ':' + str(start_pos)
    end_offset = tup2str(kw_name) + ':' + str(end_pos)
    for vp in struct_buf[:t + 1]:
        vs = offset2struct[vp]
        struct2keywordoff[vs] = '|'.join([start_offset, end_offset])

        # for fixed structs in struct_buf
    for vp in struct_buf[t + 1:]:
        start_pos = vp - kw_pos
        vs = offset2struct[vp]
        l = int(vs.split('(')[1].split(')')[0])
        end_pos = start_pos + l - 1

        start_offset = tup2str(kw_name) + ':' + str(start_pos)
        end_offset = tup2str(kw_name) + ':' + str(end_pos)
        struct2keywordoff[vs] = '|'.join([start_offset, end_offset])


def find_last_variable(is_struct_var):
    t = -1
    for i, v in enumerate(is_struct_var):
        if v:
            t = i

    return t


def gen_keyword_map(cur_keywords):
    global ins2keyoff, struct2keywordoff

    # 'chunk[1]:' -> 12
    kw_heads = {}

    for p in sorted(cur_keywords.keys()):
        struct = offset2struct[p]
        x = struct.rfind(']')
        if x != -1:
            kw_heads[struct[:x + 1]] = p
        else:
            kw_heads[struct.split(':')[0]] = p

    # gen struct2keywordoff
    struct_buf, last_keyword_pos = [], -1
    is_struct_var = []
    is_kw_changed = False
    for p in sorted(offset2struct.keys()):
        s = offset2struct[p]
        # print("s = %s, pos = %s" % (s, p))
        if s.rfind(']') != -1:
            for h in kw_heads.keys():
                if h in s:
                    head = h
                    break
        else:
            head = s.split(':')[0]

        length = int(s.split('(')[1].split(')')[0])
        kw = cur_keywords[kw_heads[head]]
        if kw_heads[head] != last_keyword_pos:
            is_kw_changed = True

        is_fixed_len = check_struct_len(s)
        if not is_fixed_len or (not is_kw_changed and len(struct_buf) != 0):
            if not is_fixed_len:
                is_struct_var.append(True)
            else:
                is_struct_var.append(False)
            struct_buf.append(p)
            continue

        if len(struct_buf) != 0 and last_keyword_pos != -1:
            t = find_last_variable(is_struct_var)
            last_kw = (last_keyword_pos, cur_keywords[last_keyword_pos])
            cur_kw = (kw_heads[head], kw)
            map_variable_len(struct_buf, t, last_kw, cur_kw)

            struct_buf, is_struct_var = [], []

        start_pos = p - kw_heads[head]
        end_pos = start_pos + length - 1

        struct2keywordoff[s] = ':'.join([tup2str(kw), str(start_pos), str(end_pos)])

        last_keyword_pos = kw_heads[head]
        if is_kw_changed:
            is_kw_changed = False

    # if the last struct is variable length
    if len(struct_buf) != 0:
        t = find_last_variable(is_struct_var)
        last_kw = (last_keyword_pos, cur_keywords[last_keyword_pos])
        cur_kw = (p + length, ("EOF", "end_file"))
        map_variable_len(struct_buf, t, last_kw, cur_kw)

    # for k, v in struct2keywordoff.items():
    #     print(k + '->' + v)

    # CMP -> keyword offset
    fields_pos = sorted(offset2struct.keys())
    for ins, offsets in ins2offsets.items():
        # if '0x805b892' in ins:
        #     print(" -- %s --> %s" % (ins, '|'.join(offsets)))
        for off_str in offsets:
            offs = off_str.split(',')
            for idx, off in enumerate(offs):
                i_off = int(off)
                for n, p in enumerate(fields_pos):
                    if p > i_off:
                        pos = fields_pos[n - 1]
                        newkey = struct2keywordoff[offset2struct[pos]]
                        # print("ins = " + ins + ", value =" + newkey)
                        if not ins in ins2keyoff:
                            ins2keyoff[ins] = [newkey]
                        else:
                            ins2keyoff[ins].append(newkey)
                        break
                newkey = struct2keywordoff[offset2struct[p]]
                ins2keyoff[ins].append(newkey)

        ins2keyoff[ins] = list(set(ins2keyoff[ins]))
        # if '0x805b892' in ins:
        #     print("++ %s --> %s" % (ins, "|".join(ins2keyoff[ins])))


def do_mapping(corpus_dir, filename=""):
    taint_dir = corpus_dir + tdir_ext
    print("-------------------")
    print(taint_dir)
    print("-------------------")
    assert os.path.isdir(taint_dir), "%s does not exist" % taint_dir
    struct_dir = corpus_dir + sdir_ext
    assert os.path.isdir(struct_dir), "%s does not exist" % sdir_ext

    passed = 0
    if not filename:
        for cf in os.listdir(corpus_dir):
            t_file = cf + tfile_ext
            cf_size = os.path.getsize(os.path.join(corpus_dir, cf))
            base, ext = os.path.splitext(cf)
            s_file = base + '_' + str(cf_size) + ext + sfile_ext

            t_dir = corpus_dir + tdir_ext
            s_dir = corpus_dir + sdir_ext

            t_fpath = os.path.join(t_dir, t_file)
            s_fpath = os.path.join(s_dir, s_file)

            if not os.path.exists(t_fpath):
                print('  [-] Cannot find taint file %s, ignoring it.' % t_fpath)
                continue
            if not os.path.exists(s_fpath):
                print('  [-] Cannot find struct file %s, ignoring it.' % s_fpath)
                continue

            read_taint_file(t_fpath)  # generate ins2offsets
            read_struct_file(s_fpath)  # generate offset2struct

            cfpath = os.path.join(corpus_dir, cf)
            cur_keywords = find_keywords(cfpath)
            gen_keyword_map(cur_keywords)

            passed += 1

        return passed

    else:
        t_file = filename + tfile_ext

        cf_size = os.path.getsize(os.path.join(corpus_dir, filename))
        base, ext = os.path.splitext(filename)
        ext = ".ogg"
        s_file = base + '_' + str(cf_size) + ext + sfile_ext

        t_dir = corpus_dir + tdir_ext
        s_dir = corpus_dir + sdir_ext

        t_fpath = os.path.join(t_dir, t_file)
        s_fpath = os.path.join(s_dir, s_file)

        if not os.path.exists(t_fpath):
            print('  [-] Cannot find taint file %s, ignoring it.' % t_fpath)
            return passed
        if not os.path.exists(s_fpath):
            print('  [-] Cannot find struct file %s, ignoring it.' % s_fpath)
            return passed

        read_taint_file(t_fpath)  # generate ins2offsets
        read_struct_file(s_fpath)  # generate offset2struct

        cfpath = os.path.join(corpus_dir, filename)
        cur_keywords = find_keywords(cfpath)
        gen_keyword_map(cur_keywords)

        passed += 1
        return passed


'''
    Usage:
      ./gen_kw_map.py --keyword_file KW_MAP.TXT --corpus_dir CORPUS_DIR
      ./gen_kw_map.py --keyword_file KW_MAP.TXT --file FILE

    Directory structure:
      -+- CORPUS_DIR
       |- CORPUS_DIR_taint_logs
       |- CORPUS_DIR_parsed_structs

    Output: 
      CMP -> keywordN:offset_start:offset_end
             keywordN:offset_start | keywordN+1:(offset_start-1) 
'''


def main():
    global ins2keyoff

    parser = argparse.ArgumentParser()
    parser.add_argument('--keyword_file', default='./kewords_png.txt', action='store', type=str,
                        help='specify the keyword file.')
    parser.add_argument('--corpus_dir', action='store', type=str, help='specify the corpus\' directory.')
    parser.add_argument('--file', action='store', type=str, help='specify a single file to parse.')

    args = parser.parse_args()

    assert os.path.isfile(args.keyword_file), "%s does not exist" % args.keyword_file
    read_keyword_file(args.keyword_file)

    if args.corpus_dir:
        assert os.path.isdir(args.corpus_dir), "%s does not exist" % args.corpus_dir

        corpus_dir = args.corpus_dir.rstrip(os.sep)
        do_mapping(corpus_dir)

        f_dir, c_name = os.path.split(corpus_dir)

    if args.file:
        assert os.path.isfile(args.file), "%s does not exist" % args.file

        corpus_dir, fn = os.path.split(args.file)
        corpus_dir = corpus_dir.rstrip(os.sep)
        # print("-----------------------")
        # print(args.file)
        # print(fn)
        # print(corpus_dir)
        # print("-----------------------")
        f_dir, c_name = os.path.split(corpus_dir)
        i2k_fpath = os.sep.join([f_dir, c_name + '_ins2keyoff.json'])
        if os.path.exists(i2k_fpath):
            print("  [+] Reading existed ins2keyoff file...")
            with open(i2k_fpath, 'r') as kf:
                ins2keyoff = json.load(kf)

        passed = do_mapping(corpus_dir, fn)

    # for k, v in ins2keyoff.items():
    #     print("%s --> %s" % (k, ' | '.join(v)))

    i2k_fpath = os.sep.join([f_dir, c_name + '_ins2keyoff.json'])
    with open(i2k_fpath, 'w') as kf:
        json.dump(ins2keyoff, kf)

    if passed:
        print("  [+] The keyword mapping generated/updated.")
    else:
        print("  [-] Nothing new generated.")


if __name__ == '__main__':
    main()

