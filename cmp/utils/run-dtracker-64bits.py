#!/usr/bin/env python3
import shlex, subprocess
import platform
import os, shutil, argparse

'''
    TODO: use abspath
'''
taint_file_name = ""
temp_output = "temp_output"
def do_dtracker(command_line, cmd_args, taint_log_dir):
    # do the magic
    global taint_file_name
    args = shlex.split(command_line)
    global temp_output
    dtracker_output = os.path.join(os.getcwd(), temp_output)
    taint_log_name = os.path.basename(taint_file_name) + '.taint.log'
    taint_log = os.path.join(taint_log_dir, taint_log_name)

    if os.path.exists(taint_log):
        print("  [+] Found %s in log directory, move to the next file..." % taint_log)
        return
    else:
        # print("[+] Enter %s ..." % os.getcwd())
        print('**********'+command_line)
        print("  [+] Taint analyzing %s..." % taint_file_name)
        print(args)
        # args.append("out.mp3")
        try:
            p = subprocess.check_output(args)
        except subprocess.CalledProcessError:
            # print("[-] Analysis failed!")
            if not os.path.exists(dtracker_output):
                print('  [-] Cannot generate the taint trace file %s, Taint analysis failed.' % dtracker_output)
                return

        # if not os.path.exists(dtracker_output):
        #     print('[-] Cannot generate the taint trace file %s' % dtracker_output)
        #     return

        # mv pintool.log -> filename.taint.log
        shutil.move(dtracker_output, taint_log) 
        print("[+] move from %s to %s,Taint analysis completed."%(dtracker_output, taint_log))

        return

def mk_tlog_dir(target_data_dir):
    td_dir_l = target_data_dir.split(os.sep)
    if not td_dir_l[:-1]:
        tlog_dir = '.' + os.sep + td_dir_l[-1] + '_taint_logs'
    else:
        tlog_dir = os.sep.join(td_dir_l[:-1]) + os.sep + td_dir_l[-1] + '_taint_logs'
    
    if not os.path.exists(tlog_dir):
        print('[+] Creating %s to store taint trace files.' % tlog_dir)    
        os.mkdir(tlog_dir)

    return tlog_dir

def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('--dtracker_root', help='specify the location of the DTracker. Pintool must be in <dtracker_root>/pin/, and dtracker.so in <dtracker_root>/obj-ia32/.', type=str, action='store')
    parser.add_argument('--target_prog', help='specify the target program to undergo taint analysis', type=str, action='store')
    parser.add_argument('--target_data', help='specify the data to feed target program,could either be a file or a directory', type=str, action='store')
    parser.add_argument("--others",help='specify other parameters', type=str, action='store')
    parser.add_argument("--parameters1",help='specify other parameters1', type=str, action='store')
    parser.add_argument("--parameters2",help='specify other parameters2', type=str, action='store')
    parser.add_argument("--fuzz_out",help='specify fuzz_out path', type=str, action='store')

    args = parser.parse_args()

    os_bits, _ = platform.architecture()
    assert os_bits == '64bit', 'DTracker now used is a 64-bit program, and cannot run on %s systems.' % os_bits 
    
    pin_exec = os.path.join(args.dtracker_root, 'pin' + os.sep + 'pin')
    assert os.path.exists(pin_exec), 'Cannot find the Pintool script %s.' % pin_exec

    dtracker_exec = os.path.join(args.dtracker_root, 'obj-intel64' + os.sep + 'libdft-dta.so')
    assert os.path.exists(dtracker_exec), 'Cannot find the taint analysis module %s.' % dtracker_exec

    target_prog = args.target_prog
    assert os.path.exists(target_prog), 'Cannot find the target program %s.' % target_prog

    if args.others!=None:
        others = args.others
    else:
        others = ""
    
    if args.parameters1!=None:
        parameters1 = "-"+args.parameters1
    else:
        parameters1 = ""
    if args.parameters2!=None:
        parameters2 = "-"+args.parameters2
    else:
        parameters2 = ""

    fuzz_out_path = args.fuzz_out

    #for wavtool-pl
    # if args.parameters1!=None:
    #     parameters1 = args.parameters1
    # else:
    #     parameters1 = ""
    # if args.parameters2!=None:
    #     parameters2 = args.parameters2
    # else:
    #     parameters2 = ""

    target_data = args.target_data
    global taint_file_name
    taint_file_name = target_data

    global temp_output
    temp_output = os.path.join(fuzz_out_path,temp_output)
    cmds = pin_exec + ' -injection child -follow_execv -t ' + dtracker_exec \
        +' -filename ' + target_data + ' -o ' + temp_output + ' -- ' + target_prog + ' ' 

    if os.path.isfile(target_data):
        # file_or_dir = 0
        td_dir, _ = os.path.split(target_data)

        tlog_dir = mk_tlog_dir(td_dir)
        
        cmd_line = cmds + parameters1 + " " + target_data + " " + parameters2 + " " + others
        do_dtracker(cmd_line, args, tlog_dir)
    
    elif os.path.isdir(target_data):
        if target_data.endswith(os.sep):
            td_dir = target_data.strip(os.sep)
        else:
            td_dir = target_data

        tlog_dir = mk_tlog_dir(td_dir)

        for fn in os.listdir(td_dir):
            fpath = os.path.join(td_dir, fn)
            
            # ignore all the sub-directories, including invalid_files/
            if os.path.isfile(fpath) and not fn.startswith('.'):
                # cmd_line = cmds + fpath + " "+ others
                cmd_line = cmds + parameters1 + " " + fpath + " " + parameters2 + " " + others
                do_dtracker(cmd_line, args, tlog_dir)


    else:
        print('E: %s is not a file nor a directory. We dont know what to do.' % target_data)
        exit(-1)




if __name__ == '__main__':
    main()
