#include <stdio.h>
#include <stdlib.h>
#include "bptree.h"

int main() 
{
    Node* root = NULL;

    // B+ Tree에 키 삽입
    int keys_to_insert[] = {10, 20, 5, 6, 12, 30, 7, 17};
    int num_keys = sizeof(keys_to_insert) / sizeof(keys_to_insert[0]);

    for (int i = 0; i < num_keys; i++) 
    {
        bptree_insert(&root, keys_to_insert[i]);
    }

    // B+ Tree 출력
    printf("B+ Tree Structure:\n");
    bptree_print(root, 0);

    // B+ Tree 메모리 해제
    bptree_destroy(root);

    return 0;
}