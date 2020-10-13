import os
import subprocess
if __name__ == "__main__":
    g = os.walk("./")  

    prefix1 = "/vagrant/afl-2.52b/afl-g++ " 
    tail1 = " -I .. ../lodepng.cpp -Wall -Wextra -pedantic -ansi -O3 -o "
    prefix2 = "/vagrant/afl-2.52b/afl-gcc " 
    tail2 = " -I .. ../lodepng.c -ansi -pedantic -Wall -Wextra -O3 -o "
    for path,dir_list,file_list in g:  
        for file_name in file_list:  
            # print(file_name)
            print("-------------------------------------")
            if not ".json" in file_name and not ".txt" in file_name and ".py" not in file_name and ".o" not in file_name:
                print(file_name)
                command = "python /home/wws/Music/Fuzz/cmp/utils/trans-bb-addr.py "+file_name
                cmd = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE,
                                                                    stderr=subprocess.STDOUT)
                result = cmd.stdout.read()
                print(result)
            # # print(os.path.join(path, file_name) )
            # if ".cpp" in file_name:
            #     print("------------------------------------------")
            #     print("now compile "+file_name)
            #     # print(type(file_name))
            #     # print(type(file_name.split(".")[0]))
            #     # command = command.format(file_name,file_name.split(".")[0]+"_cpp")
            #     command = prefix1 + file_name + tail1 + file_name.split(".")[0]+"_cpp"
            #     print(command)
            #     cmd = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE,
            #                                                         stderr=subprocess.STDOUT)
            #     result = cmd.stdout.read()
            #     print(result)

            # if ".c" in file_name:
            #     print("------------------------------------------")
            #     print("now compile "+file_name)
            #     # print(type(file_name))
            #     # print(type(file_name.split(".")[0]))
            #     # command2 = command2.format(file_name,file_name.split(".")[0]+"_c")
            #     command = prefix2 + file_name +tail2 + file_name.split(".")[0]+"_c"
            #     print(command)
            #     cmd = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE,
            #                                                         stderr=subprocess.STDOUT)
            #     result = cmd.stdout.read()
            #     print(result)
