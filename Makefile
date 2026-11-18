# 컴파일러 및 컴파일 옵션 설정
CC = gcc
# -I./include: 헤더 파일 위치 지정
# -g: GDB 디버깅용 심볼 포함
CFLAGS = -Wall -Wextra -g -I./include

# 빌드 대상 타겟(실행 파일) 이름
TARGET = bptree_test

# 소스 파일 및 오브젝트 파일 지정
SRCS = src/main.c src/bptree.c
OBJS = $(SRCS:.c=.o)

# 기본 빌드 규칙 (make 입력 시 실행)
all: $(TARGET)

# 실행 파일 링킹
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 개별 소스 파일을 오브젝트 파일로 컴파일
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 빌드 산출물 삭제 (make clean)
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean