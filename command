sudo systemctl start docker.service
sodo docker run -it --rm -v /home/syhxzzz/Documents/Code/pku-minic/:/root/compiler maxxing/compiler-dev bash
在docker内部的命令
build/compiler -koopa debug/hello.c -o hello.koopa

docker ps -a
打印出所有的docker容器
docker rm XXXXXXXX
删除容器

056afdc36b6e
下载了gdb的容器名

sudo docker exec -it 056 bash运行之前的容器
