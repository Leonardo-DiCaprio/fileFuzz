#!/bin/bash
# Need to `sudo apt-get install inotify-tools` first.
# Need to mkdir fuzz_output/queue first

# In Ubuntu, function definition does not need keyword "function"
# function usage() {
usage() { 
    echo "Usage: monitor_outputs -p PROG -t TEMPLATE TARGET_DIR"
    echo " Watch and response to file creation in the TARGET_DIR "
    echo "   1. taint analysis with PROG"
    echo "   2. parse the file with TEMPLATE"
    echo " The options are:"
    echo "  -p PROG,       Specify the program for taint analysis"
    echo "  -t TEMPLATE,   Specify the template for format parsing"
    echo "  -k KEYWORD,    Specify the keyword file for the target format"
    echo "  -o OTHERS     Specify the other parameters"
    echo "  -h,            Display this information"
}

while getopts ":p:t:k:a:b:o:h" arg 
do
    case $arg in 
        p) 
            prog=${OPTARG}
            ;;
        t)
            template=${OPTARG}
            ;;
        k)
            kw_file=${OPTARG}
            ;;
        a)
            aparameters=${OPTARG}
            ;;
        b)
            bparameters=${OPTARG}
            ;;
        o)
            others=${OPTARG}   
            ;;     
        h)
            usage
            # echo "Usage: $0 -t FILETYPE <CORPUS_DIR>"  
            exit 1
            ;;
        ?)
            echo "Error: Invalid option: -$OPTARG"
            echo "Usage: $0 -p <PROG> -t <TEMPLATE> <TARGET_DIR>"
            exit 2
    esac
done

# echo "************************"
# echo $prog,$aparameters
# echo "************************"

shift $((OPTIND-1))
dname=${1%*/}    # remove the tail '/'
target_dir=$dname"/queue"
ins2keyoff_file=$target_dir"_ins2keyoff.json"

template_filename=${template##*/}     # PNG.bt
ftype=${template_filename%.*}         # PNG
ftype=`echo $ftype | tr 'A-Z' 'a-z'`  # png

# 检查 target_dir 是否存在，是否半路被删除
# if [ ! -d $target_dir ]
# then
#     echo "[-] E: $target_dir does not exist!"
#     exit -1
# fi

# you might want to change the directory below into the targeted directory
# DIR="/home/jacob/Bureaublad/test123"

# echo "[+] Generating the initial keyword mapping..."
# python3 gen_keyword_map.py --keyword_file keywords.txt --corpus_dir $target_dir

i2k_file=${target_dir%*/}"_ins2keyoff.json"
# echo "[+] Mapping file $i2k_file is stored."

echo "[+] Watching $target_dir for new files..."
inotifywait -m -r -e move -e create --format '%w%f' "$1" | while read f

do
  ext=${f:(-3):3}
  f_dir=${f%/*}

  if [ $f_dir != $target_dir ]
  then
    continue
  fi

  echo ">>>>> $f detected."
  # echo "$f_dir, $ext, $ftype"

  if [ $ext == "cov" -o $ext == $ftype ]
  then
    if [ $ext == "cov" ]
    then
      echo "  [+] $f has new code coverage, need a taint analysis."
    else
      echo "  [+] $f is a seed input, need a taint analysis."
    fi
    # echo "--------------------------"
    # echo $others,$aparameter,$bparameters,$prog
    # echo "--------------------------"
    python3 utils/run-dtracker-64bits.py --dtracker_root libdft64 --target_prog $prog $aparameters --target_data $f $bparameters $others --fuzz_out $dname

    # echo "  [+] Done."
    echo "  [+] Parsing $f's structures..."
    /usr/bin/python2 utils/pfp_show_file_struct.py --file $f -t $template # works under Python2 or Python3
    # echo "  [+] Done parsing $f."
    
    echo "  [+] Adding new key mappings to the $i2k_file."
    echo $kw_file,$f
    python3 utils/gen_keyword_map.py --keyword_file $kw_file --file $f

    if [ $ext == "cov" ]
    then
      echo "  [+] Predicting input offsets affecting each CMP instruction."
      #echo $ins2keyoff_file,$f
      python3 utils/predict_struct_pos.py --keyword_file $kw_file --ins2keyoff_map $ins2keyoff_file --target_file $f
    fi


    echo -e "\n[+] Watching $target_dir again..."
  else
    echo " [+] Nothing to do."
  fi

done
