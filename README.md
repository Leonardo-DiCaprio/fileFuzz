# fileFuzz
file format fuzzer based on AFL


- 主要修改的代码在cmp->afl中，主要修改了afl-fuzz以及readjson两个文件。
- 运行说明在cmp->readme中
- 工程比较大是因为添加了fuzz环境进去，target_progs文件夹中是编译好的被测程序以及测试结果，如果只想clone代码不想clone程序，可以clone dev分支,二者代码相同。
