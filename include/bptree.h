#ifndef BPTREE_H
#define BPTREE_H

#include <stdbool.h>

#define M                       4
#define MAX_KEY_SIZE            (M - 1)   // 논리적 최대 키: 3개
#define MAX_CHILDREN M                    // 논리적 최대 자식: 4개
#define MIN_KEY_SIZE            (MAX_KEY_SIZE / 2) // 최소 키 개수: 1개

typedef struct _Node 
{
    int keys[MAX_KEY_SIZE + 1];                 // 오버플로우 임시 수용(+1): 4칸
    struct _Node* children[MAX_CHILDREN + 1];   // 오버플로우 임시 수용(+1): 5칸
    struct _Node* next;                         // 리프 노드 순차 연결용
    struct _Node* parent;                         
    int keys_count;
    bool is_leaf;
} Node;

/*
    함수 선언
*/ 



// 새로운 노드를 생성하는 함수
Node* create_node(bool is_leaf);


// B+ Tree에 키를 삽입하는 함수
int bptree_insert(Node** root, int key);

// B+ Tree를 출력하는 함수
void bptree_print(Node* root, int level);

// B+ Tree의 리프 노드들을 순차적으로 출력하는 함수
void bptree_leaf_print(Node* root);

// B+ Tree의 메모리를 해제하는 함수
void bptree_destroy(Node* root);

#endif // BPTREE_H