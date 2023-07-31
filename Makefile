#CC = gcc								#编译工具链 x86
CC = arm-linux-gnueabihf-gcc			 #交叉编译
TRG = bin/main 							#目标文件  -->  可执行程序
SRC = $(wildcard ./src/*.c)				#src目录下的所有源文件 (*.c)
CFLAGS = -I ./include -L ./lib 				#编译过程中命令行参数：指定头文件路径 ./include,库文件 ./lib
#LIB = ./lib/*.a						      #链接静态库
OBJ = $(patsubst %.c, %.o, $(SRC)) 	      #编译过程中的文件 (*.o)
THREAD = -lpthread				    	     #线程

#把.o文件生成可执行文件
$(TRG):$(OBJ)
	$(CC) $^ -o $@ $(CFLAGS) -lpthread -std=c11 -D_GNU_SOURCE -g

#把所有的.c文件编译成.o文件
%.o:%.c
	$(CC) -c $^ -o $@ $(CFLAGS) -lpthread -std=c11 -D_GNU_SOURCE -g

#删除编译过程中产生的文件
clean:
	$(RM) $(OBJ)
	$(RM) $(TRG)
