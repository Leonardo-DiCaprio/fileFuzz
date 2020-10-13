#!/usr/bin/env python
#-*- coding:utf-8 -*-  
from __future__ import print_function
import  os, time, argparse
import pfp


'''
    Work:
        1. PNG.bt

    Does not work:
        1. PDF.bt
'''
def mk_struct_dir(target_data_dir):
    td_dir_l = target_data_dir.split(os.sep)
    if not td_dir_l[:-1]:
        tlog_dir = '.' + os.sep + td_dir_l[-1] + '_parsed_structs'
    else:
        tlog_dir = os.sep.join(td_dir_l[:-1]) + os.sep + td_dir_l[-1] + '_parsed_structs'
    
    if not os.path.exists(tlog_dir):
        print('[+] Creating %s to store parsed struct files.' % tlog_dir)    
        os.mkdir(tlog_dir)

    return tlog_dir

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--file', type=str, help='specify the input file', action='store')
    parser.add_argument('--dir', type=str, help='specify the input directory', action='store')

    parser.add_argument('-t', '--template_file', type=str, help='the template file to parse the files', action='store')

    args = parser.parse_args()

    assert os.path.isfile(args.template_file), "%s does not exist" % args.template_file
    ftype = os.path.splitext(os.path.basename(args.template_file))[0]

    failed = False

    if args.file:
        assert os.path.isfile(args.file)," %s does not exist" % args.file

        try:
            dom = pfp.parse(data_file=args.file, template_file=args.template_file)
        except (pfp.errors.PrematureEOF, pfp.errors.PfpError):
            print("[-] Err: %s cannot be parsed, ignoring it..." % args.file)
            failed = True
            exit(-1)

        dom_stream = dom._pfp__show(include_offset=True)
        print(type(dom_stream), len(dom_stream))
        ctype_dic = dict()
        for item in dom_stream.split('\n'):
            if 'ctype' in item:
                ctype_name = '(chunk:type:cname,' + item.split('[')[-1].split(']')[0] + ')'
                loc = int(item.split('ctype')[0].strip(), 16)
                ctype_dic[ctype_name] = loc
        print(ctype_dic)
        fsize = os.path.getsize(args.file)
        fdir, fn = os.path.split(args.file)

        outdir = fdir + '_parsed_structs'
        ofn = os.path.splitext(fn)[0] + '_' + str(fsize) + '.' + ftype.lower() + '_struct'
        outfile = os.path.join(outdir, ofn)

        if not os.path.exists(outdir):
            print('[+] Creating %s to store struct files.' % outdir)    
            os.mkdir(outdir)

        # ofn = 'output_content'
        # ofn = args.file + '_struct'
        # fn = ofn + '_' + str(fsize)
        # ext = '.txt'
        # outfile = fn + ext
        with open(outfile, 'w') as outf:
            outf.write(dom_stream)

        print("  [+] Structure parsing completed.")
    
    if args.dir:
        assert os.path.isdir(args.dir)," %s does not exist" % args.dir
        
        data_dir, _ = os.path.split(os.path.join(args.dir, 'parsed_structs'))
        outdir = mk_struct_dir(data_dir)

        for root, dirs, files in os.walk(args.dir):
            if root == outdir:
                continue
            
            for fn in files:
                fpath = os.path.join(root, fn)
                print('  [+] Parsing ' + fpath + '...')
                if fn.endswith(ftype.upper()) or fn.endswith(ftype.lower()):
                    fpath = os.path.join(root, fn)

                    try:
                        dom = pfp.parse(data_file=fpath, template_file=args.template_file)
                    except (pfp.errors.PrematureEOF, pfp.errors.PfpError):
                        print("\t [-] Err: %s cannot be parsed, ignoring it..." % args.file)
                        continue
                    else:
                        dom_stream = dom._pfp__show(include_offset=True)
                        fsize = os.path.getsize(fpath)

                        ofn = os.path.splitext(fn)[0] + '_' + str(fsize) + '.' + ftype.lower() + '_struct'
                        outfile = os.path.join(outdir, ofn)
                        with open(outfile, 'w') as outf:
                            outf.write(dom_stream)
                else:
                    print('[-] %s is not a %s file, ignoring it...' % (fpath, ftype))
                    failed = True
                    continue
        print("  [+] Structure parsing completed", end="")
        if failed:
            print(", with some failure.")
        else:
            print(".")



if __name__ == '__main__':
    main()
